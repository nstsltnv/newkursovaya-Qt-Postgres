#include "mainwindow.h"
#include "control.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDialog>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QDebug>
#include <QMenuBar>
#include <QMenu>
#include <QInputDialog>
#include <QGroupBox>
#include <QScrollArea>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent), currentMode(FileMode), dbManager(new DBManager(this))
{
    setupUI();
    setupMenu();
    setWindowTitle("Телефонный справочник [Файловый режим]");
    resize(1000, 600);

    connect(dbManager, &DBManager::databaseError,
            this, &MainWindow::onDatabaseError);
}

MainWindow::~MainWindow()
{
    delete dbManager;
}

void MainWindow::setupMenu()
{
    QMenuBar *menuBar = new QMenuBar(this);

    QMenu *storageMenu = menuBar->addMenu("Хранилище");
    storageMenu->addAction("Файловый режим", this, &MainWindow::onSwitchToFileMode);
    storageMenu->addAction("Режим базы данных", this, &MainWindow::onSwitchToDatabaseMode);
    storageMenu->addSeparator();
    storageMenu->addAction("Подключиться к PostgreSQL...", this, &MainWindow::onConnectToDatabase);

    setMenuBar(menuBar);
}

void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    statusLabel = new QLabel("Режим: Файловое хранилище");
    statusLabel->setStyleSheet("font-weight: bold; padding: 5px; background-color: #e0e0e0;");
    mainLayout->addWidget(statusLabel);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel("Поиск:"));
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Имя, фамилия, email, телефон, адрес...");
    searchLayout->addWidget(searchEdit);

    QPushButton *searchButton = new QPushButton("Найти");
    connect(searchButton, &QPushButton::clicked, this, &MainWindow::onSearch);
    searchLayout->addWidget(searchButton);

    QPushButton *clearButton = new QPushButton("Очистить");
    connect(clearButton, &QPushButton::clicked, this, [this]() {
        searchEdit->clear();
        refreshTable();
    });
    searchLayout->addWidget(clearButton);

    mainLayout->addLayout(searchLayout);

    table = new QTableWidget();
    table->setColumnCount(7);
    QStringList headers = {"Имя", "Фамилия", "Отчество", "Email", "Телефон", "Дата рождения", "Адрес"};
    table->setHorizontalHeaderLabels(headers);
    table->setSelectionBehavior(QAbstractItemView::SelectRows);
    table->setEditTriggers(QAbstractItemView::NoEditTriggers);
    table->setSelectionMode(QAbstractItemView::SingleSelection);
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSortingEnabled(false);

    table->setColumnWidth(0, 120);
    table->setColumnWidth(1, 120);
    table->setColumnWidth(2, 120);
    table->setColumnWidth(3, 180);
    table->setColumnWidth(4, 150);
    table->setColumnWidth(5, 120);

    mainLayout->addWidget(table);

    QHBoxLayout *buttonLayout = new QHBoxLayout();

    QPushButton *addButton = new QPushButton("Добавить");
    connect(addButton, &QPushButton::clicked, this, &MainWindow::onAddContact);
    buttonLayout->addWidget(addButton);

    QPushButton *editButton = new QPushButton("Редактировать");
    connect(editButton, &QPushButton::clicked, this, &MainWindow::onEditContact);
    buttonLayout->addWidget(editButton);

    QPushButton *deleteButton = new QPushButton("Удалить");
    connect(deleteButton, &QPushButton::clicked, this, &MainWindow::onDeleteContact);
    buttonLayout->addWidget(deleteButton);

    buttonLayout->addStretch();

    QPushButton *saveButton = new QPushButton("Сохранить в файл");
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSaveToFile);
    buttonLayout->addWidget(saveButton);

    QPushButton *loadButton = new QPushButton("Загрузить из файла");
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::onLoadFromFile);
    buttonLayout->addWidget(loadButton);

    mainLayout->addLayout(buttonLayout);

    QHBoxLayout *sortLayout = new QHBoxLayout();
    sortLayout->addWidget(new QLabel("Сортировка:"));

    QPushButton *sortFirstNameAsc = new QPushButton("Имя ↑");
    connect(sortFirstNameAsc, &QPushButton::clicked, this, &MainWindow::onSortFirstNameAsc);
    sortLayout->addWidget(sortFirstNameAsc);

    QPushButton *sortFirstNameDesc = new QPushButton("Имя ↓");
    connect(sortFirstNameDesc, &QPushButton::clicked, this, &MainWindow::onSortFirstNameDesc);
    sortLayout->addWidget(sortFirstNameDesc);

    QPushButton *sortLastNameAsc = new QPushButton("Фамилия ↑");
    connect(sortLastNameAsc, &QPushButton::clicked, this, &MainWindow::onSortLastNameAsc);
    sortLayout->addWidget(sortLastNameAsc);

    QPushButton *sortLastNameDesc = new QPushButton("Фамилия ↓");
    connect(sortLastNameDesc, &QPushButton::clicked, this, &MainWindow::onSortLastNameDesc);
    sortLayout->addWidget(sortLastNameDesc);

    QPushButton *sortEmailAsc = new QPushButton("Email ↑");
    connect(sortEmailAsc, &QPushButton::clicked, this, &MainWindow::onSortEmailAsc);
    sortLayout->addWidget(sortEmailAsc);

    QPushButton *sortEmailDesc = new QPushButton("Email ↓");
    connect(sortEmailDesc, &QPushButton::clicked, this, &MainWindow::onSortEmailDesc);
    sortLayout->addWidget(sortEmailDesc);

    QPushButton *sortBirthDateAsc = new QPushButton("Дата рождения ↑");
    connect(sortBirthDateAsc, &QPushButton::clicked, this, &MainWindow::onSortBirthDateAsc);
    sortLayout->addWidget(sortBirthDateAsc);

    QPushButton *sortBirthDateDesc = new QPushButton("Дата рождения ↓");
    connect(sortBirthDateDesc, &QPushButton::clicked, this, &MainWindow::onSortBirthDateDesc);
    sortLayout->addWidget(sortBirthDateDesc);

    sortLayout->addStretch();
    mainLayout->addLayout(sortLayout);

    countLabel = new QLabel("Контактов: 0");
    countLabel->setStyleSheet("font-weight: bold; padding: 5px;");
    mainLayout->addWidget(countLabel);

    setCentralWidget(centralWidget);
}

void MainWindow::displayContacts(const QVector<Contact> &contacts)
{
    table->setRowCount(contacts.size());

    for (int i = 0; i < contacts.size(); i++) {
        const Contact &contact = contacts[i];
        table->setItem(i, 0, new QTableWidgetItem(contact.getFirstName()));
        table->setItem(i, 1, new QTableWidgetItem(contact.getLastName()));
        table->setItem(i, 2, new QTableWidgetItem(contact.getMiddleName()));
        table->setItem(i, 3, new QTableWidgetItem(contact.getEmail()));

        QString phones;
        QVector<QString> phoneList = contact.getPhones();
        for (int j = 0; j < phoneList.size(); j++) {
            if (!phones.isEmpty()) phones += "\n";
            phones += phoneList[j];
        }
        QTableWidgetItem *phoneItem = new QTableWidgetItem(phones);
        phoneItem->setToolTip(phones);
        table->setItem(i, 4, phoneItem);

        table->setItem(i, 5, new QTableWidgetItem(contact.getBirthDateString()));
        table->setItem(i, 6, new QTableWidgetItem(contact.getAddress()));
    }

    countLabel->setText(QString("Контактов: %1").arg(contacts.size()));
}

void MainWindow::refreshTable()
{
    if (currentMode == FileMode) {
        QVector<Contact> contacts = phoneBook.getAllContacts();
        displayContacts(contacts);
    } else {
        if (dbManager->isConnected()) {
            QVector<Contact> contacts = dbManager->getAllContacts();
            displayContacts(contacts);
        } else {
            countLabel->setText("Не подключено к базе данных");
            table->setRowCount(0);
        }
    }
}

void MainWindow::onAddContact()
{
    Contact contact;
    if (showContactDialog(contact, "Добавление контакта")) {
        bool success = false;

        if (currentMode == FileMode) {
            success = phoneBook.addContact(contact);
            if (success) {
                refreshTable();
                showSuccess("Контакт добавлен");
            } else {
                showError("Не удалось добавить контакт");
            }
        } else {
            success = dbManager->addContact(contact);
            if (success) {
                refreshTable();
                showSuccess("Контакт добавлен в базу данных");
            } else {
                showError("Не удалось добавить контакт в базу данных");
            }
        }
    }
}

void MainWindow::onDeleteContact()
{
    int row = table->currentRow();
    if (row < 0) {
        showError("Выберите контакт для удаления");
        return;
    }

    QMessageBox::StandardButton reply;
    reply = QMessageBox::question(this, "Подтверждение",
                                 "Удалить выбранный контакт?",
                                 QMessageBox::Yes | QMessageBox::No);
    if (reply == QMessageBox::Yes) {
        bool success = false;

        if (currentMode == FileMode) {
            success = phoneBook.removeContact(row);
        } else {
            int contactId = dbManager->getContactIdByRow(row);
            if (contactId > 0) {
                success = dbManager->deleteContact(contactId);
            } else {
                showError("Не удалось получить ID контакта");
                return;
            }
        }

        if (success) {
            refreshTable();
            showSuccess("Контакт удален");
        } else {
            showError("Не удалось удалить контакт");
        }
    }
}

void MainWindow::onEditContact()
{
    int row = table->currentRow();
    if (row < 0) {
        showError("Выберите контакт для редактирования");
        return;
    }

    Contact contact;

    if (currentMode == FileMode) {
        QVector<Contact> contacts = phoneBook.getAllContacts();
        if (row >= contacts.size()) {
            showError("Неверный индекс контакта");
            return;
        }
        contact = contacts[row];
    } else {
        int contactId = dbManager->getContactIdByRow(row);
        if (contactId <= 0) {
            showError("Не удалось получить контакт");
            return;
        }
        contact = dbManager->getContactById(contactId);
    }

    if (showContactDialog(contact, "Редактирование контакта")) {
        bool success = false;

        if (currentMode == FileMode) {
            success = phoneBook.editContact(row, contact);
            if (success) {
                refreshTable();
                showSuccess("Контакт обновлен");
            } else {
                showError("Не удалось обновить контакт");
            }
        } else {
            int contactId = dbManager->getContactIdByRow(row);
            if (contactId > 0) {
                success = dbManager->updateContact(contactId, contact);
                if (success) {
                    refreshTable();
                    showSuccess("Контакт обновлен в базе данных");
                } else {
                    showError("Не удалось обновить контакт в базе данных");
                }
            } else {
                showError("Не удалось получить ID контакта");
            }
        }
    }
}

bool MainWindow::showContactDialog(Contact &contact, const QString &title)
{
    QDialog dialog(this);
    dialog.setWindowTitle(title);
    dialog.setModal(true);
    dialog.resize(600, 500);

    QVBoxLayout *mainLayout = new QVBoxLayout(&dialog);

    // Основные поля
    QFormLayout *formLayout = new QFormLayout();

    QLineEdit *firstNameEdit = new QLineEdit(contact.getFirstName());
    firstNameEdit->setPlaceholderText("Иван");
    formLayout->addRow("Имя*:", firstNameEdit);

    QLineEdit *lastNameEdit = new QLineEdit(contact.getLastName());
    lastNameEdit->setPlaceholderText("Иванов");
    formLayout->addRow("Фамилия*:", lastNameEdit);

    QLineEdit *middleNameEdit = new QLineEdit(contact.getMiddleName());
    middleNameEdit->setPlaceholderText("Иванович");
    formLayout->addRow("Отчество:", middleNameEdit);

    QLineEdit *emailEdit = new QLineEdit(contact.getEmail());
    emailEdit->setPlaceholderText("example@mail.com");
    formLayout->addRow("Email*:", emailEdit);

    QLineEdit *birthDateEdit = new QLineEdit(contact.getBirthDateString());
    birthDateEdit->setPlaceholderText("дд.мм.гггг");
    formLayout->addRow("Дата рождения:", birthDateEdit);

    QLineEdit *addressEdit = new QLineEdit(contact.getAddress());
    addressEdit->setPlaceholderText("г. Москва, ул. Пушкина, д. 1");
    formLayout->addRow("Адрес:", addressEdit);

    mainLayout->addLayout(formLayout);

    // Телефоны
    QGroupBox *phoneGroup = new QGroupBox("Телефоны*");
    QVBoxLayout *phoneLayout = new QVBoxLayout(phoneGroup);

    QVector<QString> phones = contact.getPhones();
    QList<QLineEdit*> phoneEdits;

    for (const QString &phone : phones) {
        QLineEdit *phoneEdit = new QLineEdit(phone);
        phoneEdit->setPlaceholderText("+7 (999) 123-45-67");
        phoneLayout->addWidget(phoneEdit);
        phoneEdits.append(phoneEdit);
    }

    // Если телефонов нет, добавляем одно поле
    if (phoneEdits.isEmpty()) {
        QLineEdit *phoneEdit = new QLineEdit();
        phoneEdit->setPlaceholderText("+7 (999) 123-45-67");
        phoneLayout->addWidget(phoneEdit);
        phoneEdits.append(phoneEdit);
    }

    QHBoxLayout *phoneButtonLayout = new QHBoxLayout();
    QPushButton *addPhoneButton = new QPushButton("+ Добавить телефон");
    QPushButton *removePhoneButton = new QPushButton("- Удалить телефон");
    phoneButtonLayout->addWidget(addPhoneButton);
    phoneButtonLayout->addWidget(removePhoneButton);
    phoneButtonLayout->addStretch();
    phoneLayout->addLayout(phoneButtonLayout);

    mainLayout->addWidget(phoneGroup);

    // Кнопки диалога
    QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel);
    mainLayout->addWidget(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    connect(addPhoneButton, &QPushButton::clicked, &dialog, [phoneLayout, &phoneEdits]() {
        QLineEdit *phoneEdit = new QLineEdit();
        phoneEdit->setPlaceholderText("+7 (999) 123-45-67");
        phoneLayout->insertWidget(phoneLayout->count() - 1, phoneEdit);
        phoneEdits.append(phoneEdit);
    });

    connect(removePhoneButton, &QPushButton::clicked, &dialog, [phoneLayout, &phoneEdits]() {
        if (phoneEdits.size() > 1) {
            QLineEdit *lastEdit = phoneEdits.takeLast();
            phoneLayout->removeWidget(lastEdit);
            delete lastEdit;
        }
    });

    if (dialog.exec() == QDialog::Accepted) {
        // Валидация
        QString firstName = firstNameEdit->text().trimmed();
        QString lastName = lastNameEdit->text().trimmed();
        QString email = emailEdit->text().trimmed();

        if (firstName.isEmpty()) {
            showError("Имя обязательно для заполнения");
            return false;
        }

        if (lastName.isEmpty()) {
            showError("Фамилия обязательна для заполнения");
            return false;
        }

        if (!Control::controlEmail(email)) {
            showError("Неверный формат email");
            return false;
        }

        // Собираем телефоны
        QStringList phoneList;
        bool hasValidPhone = false;
        for (QLineEdit *phoneEdit : phoneEdits) {
            QString phone = phoneEdit->text().trimmed();
            if (!phone.isEmpty()) {
                if (Control::controlPhone(phone)) {
                    phoneList.append(phone);
                    hasValidPhone = true;
                } else {
                    showError("Неверный формат телефона: " + phone);
                    return false;
                }
            }
        }

        if (!hasValidPhone) {
            showError("Добавьте хотя бы один телефон");
            return false;
        }

        // Обновляем контакт
        contact.setFirstName(firstName);
        contact.setLastName(lastName);
        contact.setMiddleName(middleNameEdit->text().trimmed());
        contact.setEmail(email);

        QString birthDate = birthDateEdit->text().trimmed();
        if (!birthDate.isEmpty()) {
            if (Control::controlBirthDateString(birthDate)) {
                contact.setBirthDate(birthDate);
            } else {
                showError("Неверная дата рождения");
                return false;
            }
        }

        contact.setAddress(addressEdit->text().trimmed());

        // Очищаем старые телефоны и добавляем новые
        contact.clearPhones();
        for (const QString &phone : phoneList) {
            contact.addPhone(phone);
        }

        return true;
    }

    return false;
}

void MainWindow::onSearch()
{
    QString query = searchEdit->text().trimmed();
    if (query.isEmpty()) {
        refreshTable();
        return;
    }

    QVector<Contact> results;
    if (currentMode == FileMode) {
        results = phoneBook.searchContacts(query);
    } else {
        results = dbManager->searchContacts(query);
    }

    displayContacts(results);
}

void MainWindow::onSaveToFile()
{
    if (currentMode != FileMode) {
        showError("Сохранение в файл доступно только в файловом режиме");
        return;
    }

    QString filename = QFileDialog::getSaveFileName(this, "Сохранить файл",
                                                   "contacts.txt",
                                                   "Текстовые файлы (*.txt)");
    if (!filename.isEmpty()) {
        phoneBook.saveToFile(filename);
        showSuccess("Данные сохранены в файл: " + filename);
    }
}

void MainWindow::onLoadFromFile()
{
    if (currentMode != FileMode) {
        showError("Загрузка из файла доступна только в файловом режиме");
        return;
    }

    QString filename = QFileDialog::getOpenFileName(this, "Загрузить файл",
                                                   "",
                                                   "Текстовые файлы (*.txt)");
    if (!filename.isEmpty()) {
        phoneBook.loadFromFile(filename);
        refreshTable();
        showSuccess("Данные загружены из файла: " + filename);
    }
}

void MainWindow::onConnectToDatabase()
{
    bool ok;

    QString host = QInputDialog::getText(this, "Подключение к PostgreSQL",
                                        "Хост (обычно localhost):", QLineEdit::Normal,
                                        "localhost", &ok);
    if (!ok) return;

    int port = QInputDialog::getInt(this, "Подключение к PostgreSQL",
                                   "Порт (обычно 5432):", 5432, 1, 65535, 1, &ok);
    if (!ok) return;

    QString dbName = QInputDialog::getText(this, "Подключение к PostgreSQL",
                                          "Имя базы данных:", QLineEdit::Normal,
                                          "phonebook_db", &ok);
    if (!ok) return;

    QString user = QInputDialog::getText(this, "Подключение к PostgreSQL",
                                        "Пользователь:", QLineEdit::Normal,
                                        "postgres", &ok);
    if (!ok) return;

    QString password = QInputDialog::getText(this, "Подключение к PostgreSQL",
                                           "Пароль:", QLineEdit::Password,
                                           "", &ok);
    if (!ok) return;

    if (dbManager->connectToDatabase(host, port, dbName, user, password)) {
        currentMode = DatabaseMode;
        updateStatus();
        refreshTable();
        showSuccess("Успешное подключение к PostgreSQL!");
    } else {
        showError("Не удалось подключиться к PostgreSQL");
    }
}

void MainWindow::onSwitchToFileMode()
{
    currentMode = FileMode;
    updateStatus();
    refreshTable();
    showSuccess("Переключено на файловый режим");
}

void MainWindow::onSwitchToDatabaseMode()
{
    currentMode = DatabaseMode;
    updateStatus();
    refreshTable();

    if (!dbManager->isConnected()) {
        showError("Не подключено к базе данных. Используйте 'Подключиться к PostgreSQL...'");
    }
}

void MainWindow::onDatabaseError(const QString &error)
{
    showError("Ошибка базы данных:\n" + error);
}

// Слоты сортировки
void MainWindow::onSortFirstNameAsc()
{
    if (currentMode == FileMode) {
        phoneBook.sortByField("firstName");
        refreshTable();
    } else if (dbManager->isConnected()) {
        QVector<Contact> contacts = dbManager->getContactsSorted("firstName", true);
        displayContacts(contacts);
    }
}

void MainWindow::onSortFirstNameDesc()
{
    if (currentMode == FileMode) {
        phoneBook.sortByField("firstName");
        refreshTable();
    } else if (dbManager->isConnected()) {
        QVector<Contact> contacts = dbManager->getContactsSorted("firstName", false);
        displayContacts(contacts);
    }
}

void MainWindow::onSortLastNameAsc()
{
    if (currentMode == FileMode) {
        phoneBook.sortByField("lastName");
        refreshTable();
    } else if (dbManager->isConnected()) {
        QVector<Contact> contacts = dbManager->getContactsSorted("lastName", true);
        displayContacts(contacts);
    }
}

void MainWindow::onSortLastNameDesc()
{
    if (currentMode == FileMode) {
        phoneBook.sortByField("lastName");
        refreshTable();
    } else if (dbManager->isConnected()) {
        QVector<Contact> contacts = dbManager->getContactsSorted("lastName", false);
        displayContacts(contacts);
    }
}

void MainWindow::onSortEmailAsc()
{
    if (currentMode == FileMode) {
        phoneBook.sortByField("email");
        refreshTable();
    } else if (dbManager->isConnected()) {
        QVector<Contact> contacts = dbManager->getContactsSorted("email", true);
        displayContacts(contacts);
    }
}

void MainWindow::onSortEmailDesc()
{
    if (currentMode == FileMode) {
        phoneBook.sortByField("email");
        refreshTable();
    } else if (dbManager->isConnected()) {
        QVector<Contact> contacts = dbManager->getContactsSorted("email", false);
        displayContacts(contacts);
    }
}

void MainWindow::onSortBirthDateAsc()
{
    if (currentMode == FileMode) {
        phoneBook.sortByField("birthDate");
        refreshTable();
    } else if (dbManager->isConnected()) {
        QVector<Contact> contacts = dbManager->getContactsSorted("birthDate", true);
        displayContacts(contacts);
    }
}

void MainWindow::onSortBirthDateDesc()
{
    if (currentMode == FileMode) {
        phoneBook.sortByField("birthDate");
        refreshTable();
    } else if (dbManager->isConnected()) {
        QVector<Contact> contacts = dbManager->getContactsSorted("birthDate", false);
        displayContacts(contacts);
    }
}

void MainWindow::showError(const QString &message)
{
    QMessageBox::critical(this, "Ошибка", message);
}

void MainWindow::showSuccess(const QString &message)
{
    QMessageBox::information(this, "Успех", message);
}

void MainWindow::updateStatus()
{
    QString status;
    QString title = "Телефонный справочник";

    if (currentMode == FileMode) {
        status = "Режим: Файловое хранилище";
        title += " [Файловый режим]";
    } else {
        if (dbManager->isConnected()) {
            status = "Режим: PostgreSQL (подключено)";
            title += " [PostgreSQL]";
        } else {
            status = "Режим: PostgreSQL (не подключено)";
            title += " [PostgreSQL - не подключено]";
        }
    }

    statusLabel->setText(status);
    setWindowTitle(title);
}
