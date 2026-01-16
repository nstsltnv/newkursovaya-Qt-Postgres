#ifndef DBMANAGER_H
#define DBMANAGER_H

#include <QObject>
#include <QSqlDatabase>
#include <QSqlQuery>
#include <QVector>
#include "contact.h"

class DBManager : public QObject
{
    Q_OBJECT

public:
    explicit DBManager(QObject *parent = nullptr);
    ~DBManager();

    // Подключение к PostgreSQL
    bool connectToDatabase(const QString &host = "localhost",
                          int port = 5432,
                          const QString &dbName = "phonebook_db",
                          const QString &user = "postgres",
                          const QString &password = "");

    bool isConnected() const { return m_db.isOpen(); }

    // CRUD операции
    bool addContact(const Contact &contact);
    bool updateContact(int id, const Contact &contact);
    bool deleteContact(int id);
    Contact getContactById(int id);

    // Получение данных
    QVector<Contact> getAllContacts();
    QVector<Contact> searchContacts(const QString &keyword);

    // Сортировка
    QVector<Contact> getContactsSorted(const QString &field, bool ascending = true);

    // Получение ID по номеру строки
    int getContactIdByRow(int row) const;

signals:
    void databaseError(const QString &error);
    void connectedSuccessfully();

private:
    bool createTables();
    bool checkPostgresConnection();

    QSqlDatabase m_db;
    QVector<int> m_contactIds;

    // Вспомогательные методы
    Contact createContactFromQuery(const QSqlQuery &query);
    void loadPhonesForContact(Contact &contact, int contactId);
};

#endif
