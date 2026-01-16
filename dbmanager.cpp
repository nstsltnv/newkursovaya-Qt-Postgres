#include "dbmanager.h"
#include <QSqlError>
#include <QDebug>
#include <QMessageBox>
#include <QCoreApplication>

DBManager::DBManager(QObject *parent)
    : QObject(parent)
{
    // Используем PostgreSQL драйвер
    m_db = QSqlDatabase::addDatabase("QPSQL", "phonebook_connection");

    qDebug() << "Создан менеджер базы данных PostgreSQL";

    // Проверяем доступность драйвера
    if (!m_db.isValid()) {
        qDebug() << "⚠️ Драйвер PostgreSQL не валиден!";
        qDebug() << "Ошибка:" << m_db.lastError().text();
    }
}

DBManager::~DBManager()
{
    if (m_db.isOpen()) {
        m_db.close();
        qDebug() << "Соединение с базой данных закрыто";
    }
}

bool DBManager::connectToDatabase(const QString &host, int port,
                                 const QString &dbName, const QString &user,
                                 const QString &password)
{
    qDebug() << "\n=== ПОПЫТКА ПОДКЛЮЧЕНИЯ К POSTGRESQL ===";
    qDebug() << "Хост:" << host;
    qDebug() << "Порт:" << port;
    qDebug() << "База данных:" << dbName;
    qDebug() << "Пользователь:" << user;

    // Проверяем валидность драйвера
    if (!m_db.isValid()) {
        QString error = "Драйвер PostgreSQL не доступен. Убедитесь, что:\n"
                       "1. qsqlpsql.dll находится в папке с программой\n"
                       "2. libpq.dll (PostgreSQL 18) тоже там\n"
                       "3. libssl-3-x64.dll и libcrypto-3-x64.dll присутствуют";
        qDebug() << "❌" << error;
        emit databaseError(error);
        return false;
    }

    // Настраиваем подключение
    m_db.setHostName(host);
    m_db.setPort(port);
    m_db.setDatabaseName(dbName);
    m_db.setUserName(user);
    m_db.setPassword(password);

    // Настройки для лучшей совместимости
    m_db.setConnectOptions("connect_timeout=10");

    // Пытаемся подключиться
    qDebug() << "Пытаюсь подключиться к PostgreSQL...";

    if (!m_db.open()) {
        QString error = m_db.lastError().text();
        qDebug() << "❌ Ошибка подключения:" << error;

        // Детальная диагностика
        QString detailedError = "Ошибка подключения к PostgreSQL:\n" + error;

        if (error.contains("libssl", Qt::CaseInsensitive)) {
            detailedError += "\n\nПроблема с SSL библиотеками!\n"
                           "PostgreSQL 18 требует libssl-3-x64.dll";
        }

        if (error.contains("libcrypto", Qt::CaseInsensitive)) {
            detailedError += "\n\nПроблема с Crypto библиотеками!\n"
                           "PostgreSQL 18 требует libcrypto-3-x64.dll";
        }

        if (error.contains("password", Qt::CaseInsensitive)) {
            detailedError += "\n\nПроверьте пароль пользователя PostgreSQL!";
        }

        if (error.contains("does not exist", Qt::CaseInsensitive)) {
            detailedError += "\n\nБаза данных не существует!\n"
                           "Создайте базу в pgAdmin: CREATE DATABASE " + dbName + ";";
        }

        emit databaseError(detailedError);
        return false;
    }

    qDebug() << "✅ Успешное подключение к PostgreSQL!";

    // Проверяем/создаем таблицы
    if (!createTables()) {
        qDebug() << "❌ Не удалось создать таблицы";
        return false;
    }

    emit connectedSuccessfully();
    return true;
}

bool DBManager::createTables()
{
    if (!m_db.isOpen()) {
        emit databaseError("База данных не открыта");
        return false;
    }

    QSqlQuery query(m_db);

    // Таблица contacts
    QString sqlContacts =
        "CREATE TABLE IF NOT EXISTS contacts ("
        "    id SERIAL PRIMARY KEY,"
        "    first_name VARCHAR(100) NOT NULL,"
        "    last_name VARCHAR(100) NOT NULL,"
        "    middle_name VARCHAR(100),"
        "    email VARCHAR(100) NOT NULL UNIQUE,"
        "    birth_date DATE,"
        "    address TEXT,"
        "    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")";

    if (!query.exec(sqlContacts)) {
        QString error = query.lastError().text();
        // Если таблица уже существует - это нормально
        if (!error.contains("already exists", Qt::CaseInsensitive)) {
            qDebug() << "❌ Ошибка создания таблицы contacts:" << error;
            emit databaseError("Ошибка создания таблицы contacts: " + error);
            return false;
        }
        qDebug() << "Таблица contacts уже существует";
    } else {
        qDebug() << "✅ Таблица contacts создана";
    }

    // Таблица phones
    QString sqlPhones =
        "CREATE TABLE IF NOT EXISTS phones ("
        "    id SERIAL PRIMARY KEY,"
        "    contact_id INTEGER NOT NULL REFERENCES contacts(id) ON DELETE CASCADE,"
        "    phone_number VARCHAR(20) NOT NULL,"
        "    created_at TIMESTAMP DEFAULT CURRENT_TIMESTAMP"
        ")";

    if (!query.exec(sqlPhones)) {
        QString error = query.lastError().text();
        if (!error.contains("already exists", Qt::CaseInsensitive)) {
            qDebug() << "❌ Ошибка создания таблицы phones:" << error;
            emit databaseError("Ошибка создания таблицы phones: " + error);
            return false;
        }
        qDebug() << "Таблица phones уже существует";
    } else {
        qDebug() << "✅ Таблица phones создана";
    }

    // Создаем индексы для ускорения поиска
    query.exec("CREATE INDEX IF NOT EXISTS idx_contacts_name ON contacts (last_name, first_name)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_contacts_email ON contacts (email)");
    query.exec("CREATE INDEX IF NOT EXISTS idx_phones_contact ON phones (contact_id)");

    qDebug() << "✅ Все таблицы проверены/созданы";
    return true;
}

bool DBManager::addContact(const Contact &contact)
{
    if (!m_db.isOpen()) {
        emit databaseError("База данных не открыта");
        return false;
    }

    QSqlQuery query(m_db);
    m_db.transaction();

    // Вставляем контакт и получаем ID
    query.prepare(
        "INSERT INTO contacts (first_name, last_name, middle_name, email, birth_date, address) "
        "VALUES (:first_name, :last_name, :middle_name, :email, :birth_date, :address) "
        "RETURNING id"
    );

    query.bindValue(":first_name", contact.getFirstName());
    query.bindValue(":last_name", contact.getLastName());
    query.bindValue(":middle_name", contact.getMiddleName());
    query.bindValue(":email", contact.getEmail());

    if (contact.getBirthDate().isValid()) {
        query.bindValue(":birth_date", contact.getBirthDate().toString("yyyy-MM-dd"));
    } else {
        query.bindValue(":birth_date", QVariant(QVariant::Date));
    }

    query.bindValue(":address", contact.getAddress());

    if (!query.exec()) {
        m_db.rollback();
        QString error = query.lastError().text();
        qDebug() << "❌ Ошибка добавления контакта:" << error;

        // Проверяем уникальность email
        if (error.contains("unique", Qt::CaseInsensitive)) {
            emit databaseError("Контакт с таким email уже существует!");
        } else {
            emit databaseError("Ошибка добавления контакта: " + error);
        }
        return false;
    }

    // Получаем ID нового контакта
    int contactId = -1;
    if (query.next()) {
        contactId = query.value(0).toInt();
    }

    if (contactId <= 0) {
        m_db.rollback();
        emit databaseError("Не удалось получить ID нового контакта");
        return false;
    }

    // Добавляем телефоны
    query.prepare("INSERT INTO phones (contact_id, phone_number) VALUES (:contact_id, :phone_number)");

    QVector<QString> phones = contact.getPhones();
    for (const QString &phone : phones) {
        query.bindValue(":contact_id", contactId);
        query.bindValue(":phone_number", phone);

        if (!query.exec()) {
            m_db.rollback();
            QString error = query.lastError().text();
            qDebug() << "❌ Ошибка добавления телефона:" << error;
            emit databaseError("Ошибка добавления телефона: " + error);
            return false;
        }
    }

    m_db.commit();
    qDebug() << "✅ Контакт добавлен, ID:" << contactId;
    return true;
}

bool DBManager::updateContact(int id, const Contact &contact)
{
    if (!m_db.isOpen() || id <= 0) {
        emit databaseError("База не открыта или неверный ID");
        return false;
    }

    QSqlQuery query(m_db);
    m_db.transaction();

    // Обновляем контакт
    query.prepare(
        "UPDATE contacts SET "
        "first_name = :first_name, "
        "last_name = :last_name, "
        "middle_name = :middle_name, "
        "email = :email, "
        "birth_date = :birth_date, "
        "address = :address "
        "WHERE id = :id"
    );

    query.bindValue(":id", id);
    query.bindValue(":first_name", contact.getFirstName());
    query.bindValue(":last_name", contact.getLastName());
    query.bindValue(":middle_name", contact.getMiddleName());
    query.bindValue(":email", contact.getEmail());

    if (contact.getBirthDate().isValid()) {
        query.bindValue(":birth_date", contact.getBirthDate().toString("yyyy-MM-dd"));
    } else {
        query.bindValue(":birth_date", QVariant(QVariant::Date));
    }

    query.bindValue(":address", contact.getAddress());

    if (!query.exec()) {
        m_db.rollback();
        QString error = query.lastError().text();
        qDebug() << "❌ Ошибка обновления контакта:" << error;
        emit databaseError("Ошибка обновления контакта: " + error);
        return false;
    }

    // Удаляем старые телефоны
    query.prepare("DELETE FROM phones WHERE contact_id = :contact_id");
    query.bindValue(":contact_id", id);

    if (!query.exec()) {
        m_db.rollback();
        QString error = query.lastError().text();
        qDebug() << "❌ Ошибка удаления старых телефонов:" << error;
        emit databaseError("Ошибка удаления старых телефонов: " + error);
        return false;
    }

    // Добавляем новые телефоны
    query.prepare("INSERT INTO phones (contact_id, phone_number) VALUES (:contact_id, :phone_number)");

    QVector<QString> phones = contact.getPhones();
    for (const QString &phone : phones) {
        query.bindValue(":contact_id", id);
        query.bindValue(":phone_number", phone);

        if (!query.exec()) {
            m_db.rollback();
            QString error = query.lastError().text();
            qDebug() << "❌ Ошибка добавления телефона:" << error;
            emit databaseError("Ошибка добавления телефона: " + error);
            return false;
        }
    }

    m_db.commit();
    qDebug() << "✅ Контакт обновлен, ID:" << id;
    return true;
}

bool DBManager::deleteContact(int id)
{
    if (!m_db.isOpen() || id <= 0) {
        emit databaseError("База не открыта или неверный ID");
        return false;
    }

    QSqlQuery query(m_db);
    query.prepare("DELETE FROM contacts WHERE id = :id");
    query.bindValue(":id", id);

    if (!query.exec()) {
        QString error = query.lastError().text();
        qDebug() << "❌ Ошибка удаления контакта:" << error;
        emit databaseError("Ошибка удаления контакта: " + error);
        return false;
    }

    qDebug() << "✅ Контакт удален, ID:" << id;
    return true;
}

Contact DBManager::getContactById(int id)
{
    Contact contact;

    if (!m_db.isOpen() || id <= 0) {
        return contact;
    }

    QSqlQuery query(m_db);
    query.prepare(
        "SELECT id, first_name, last_name, middle_name, email, birth_date, address "
        "FROM contacts WHERE id = :id"
    );
    query.bindValue(":id", id);

    if (query.exec() && query.next()) {
        contact = createContactFromQuery(query);
        loadPhonesForContact(contact, id);
    }

    return contact;
}

QVector<Contact> DBManager::getAllContacts()
{
    QVector<Contact> contacts;

    if (!m_db.isOpen()) {
        return contacts;
    }

    QSqlQuery query(m_db);
    query.prepare(
        "SELECT id, first_name, last_name, middle_name, email, birth_date, address "
        "FROM contacts ORDER BY last_name, first_name"
    );

    m_contactIds.clear();

    if (query.exec()) {
        while (query.next()) {
            Contact contact = createContactFromQuery(query);
            int id = query.value("id").toInt();
            loadPhonesForContact(contact, id);

            contacts.append(contact);
            m_contactIds.append(id);
        }
    }

    return contacts;
}

QVector<Contact> DBManager::searchContacts(const QString &keyword)
{
    QVector<Contact> contacts;

    if (!m_db.isOpen() || keyword.isEmpty()) {
        return contacts;
    }

    QString searchPattern = "%" + keyword + "%";

    QSqlQuery query(m_db);
    query.prepare(
        "SELECT DISTINCT c.id, c.first_name, c.last_name, c.middle_name, "
        "c.email, c.birth_date, c.address "
        "FROM contacts c "
        "LEFT JOIN phones p ON c.id = p.contact_id "
        "WHERE c.first_name ILIKE :pattern "
        "OR c.last_name ILIKE :pattern "
        "OR c.middle_name ILIKE :pattern "
        "OR c.email ILIKE :pattern "
        "OR c.address ILIKE :pattern "
        "OR p.phone_number ILIKE :pattern "
        "ORDER BY c.last_name, c.first_name"
    );

    query.bindValue(":pattern", searchPattern);

    m_contactIds.clear();

    if (query.exec()) {
        while (query.next()) {
            Contact contact = createContactFromQuery(query);
            int id = query.value("id").toInt();
            loadPhonesForContact(contact, id);

            contacts.append(contact);
            m_contactIds.append(id);
        }
    }

    return contacts;
}

QVector<Contact> DBManager::getContactsSorted(const QString &field, bool ascending)
{
    QVector<Contact> contacts;

    if (!m_db.isOpen()) {
        return contacts;
    }

    QString orderField;
    if (field == "firstName") orderField = "first_name";
    else if (field == "lastName") orderField = "last_name";
    else if (field == "email") orderField = "email";
    else if (field == "birthDate") orderField = "birth_date";
    else orderField = "last_name";

    QString orderDirection = ascending ? "ASC" : "DESC";

    QString queryStr = QString(
        "SELECT id, first_name, last_name, middle_name, email, birth_date, address "
        "FROM contacts "
        "ORDER BY %1 %2, last_name, first_name"
    ).arg(orderField).arg(orderDirection);

    QSqlQuery query(m_db);
    query.prepare(queryStr);

    m_contactIds.clear();

    if (query.exec()) {
        while (query.next()) {
            Contact contact = createContactFromQuery(query);
            int id = query.value("id").toInt();
            loadPhonesForContact(contact, id);

            contacts.append(contact);
            m_contactIds.append(id);
        }
    }

    return contacts;
}

int DBManager::getContactIdByRow(int row) const
{
    if (row >= 0 && row < m_contactIds.size()) {
        return m_contactIds[row];
    }
    return -1;
}

// Вспомогательные методы
Contact DBManager::createContactFromQuery(const QSqlQuery &query)
{
    Contact contact;

    contact.setFirstName(query.value("first_name").toString());
    contact.setLastName(query.value("last_name").toString());
    contact.setMiddleName(query.value("middle_name").toString());
    contact.setEmail(query.value("email").toString());

    QString birthDateStr = query.value("birth_date").toString();
    if (!birthDateStr.isEmpty()) {
        contact.setBirthDate(QDate::fromString(birthDateStr, "yyyy-MM-dd"));
    }

    contact.setAddress(query.value("address").toString());

    return contact;
}

void DBManager::loadPhonesForContact(Contact &contact, int contactId)
{
    if (!m_db.isOpen() || contactId <= 0) {
        return;
    }

    QSqlQuery phoneQuery(m_db);
    phoneQuery.prepare("SELECT phone_number FROM phones WHERE contact_id = :contact_id ORDER BY id");
    phoneQuery.bindValue(":contact_id", contactId);

    if (phoneQuery.exec()) {
        while (phoneQuery.next()) {
            contact.addPhone(phoneQuery.value("phone_number").toString());
        }
    }
}
