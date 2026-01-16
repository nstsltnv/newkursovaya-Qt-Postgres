#include "mainwindow.h"
#include "control.h"
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QFormLayout>
#include <QDialogButtonBox>
#include <QDialog>
#include <QPushButton>
#include <QLabel>
#include <QMessageBox>
#include <QFileDialog>
#include <QHeaderView>
#include <QDebug>
#include <QComboBox>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent) {
    setupUI();
    setWindowTitle("Телефонный справочник");
    resize(1000, 600);
}

void MainWindow::setupUI() {
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *searchLayout = new QHBoxLayout();
    searchLayout->addWidget(new QLabel("Поиск:"));
    searchEdit = new QLineEdit();
    searchEdit->setPlaceholderText("Имя, фамилия, email, адрес...");
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
    table->horizontalHeader()->setStretchLastSection(true);
    table->setSortingEnabled(true);

    connect(table->horizontalHeader(), &QHeaderView::sectionClicked,
            this, &MainWindow::onSort);

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

    QPushButton *saveButton = new QPushButton("Сохранить");
    connect(saveButton, &QPushButton::clicked, this, &MainWindow::onSave);
    buttonLayout->addWidget(saveButton);

    QPushButton *loadButton = new QPushButton("Загрузить");
    connect(loadButton, &QPushButton::clicked, this, &MainWindow::onLoad);
    buttonLayout->addWidget(loadButton);

    mainLayout->addLayout(buttonLayout);

    QLabel *statusLabel = new QLabel("Всего контактов: 0");
    mainLayout->addWidget(statusLabel);

    setCentralWidget(centralWidget);
}

void MainWindow::refreshTable() {
    QVector<Contact> contacts = phoneBook.getAllContacts();
    table->setRowCount(contacts.size());

    for (int i = 0; i < contacts.size(); i++) {
        const Contact& contact = contacts[i];
        table->setItem(i, 0, new QTableWidgetItem(contact.getFirstName()));
        table->setItem(i, 1, new QTableWidgetItem(contact.getLastName()));
        table->setItem(i, 2, new QTableWidgetItem(contact.getMiddleName()));
        table->setItem(i, 3, new QTableWidgetItem(contact.getEmail()));

        auto phones = contact.getPhones();
        QString phoneStr;
        for (int j = 0; j < phones.size(); j++) {
            phoneStr += phones[j];
            if (j < phones.size() - 1) phoneStr += "\n";
        }
        table->setItem(i, 4, new QTableWidgetItem(phoneStr));

        table->setItem(i, 5, new QTableWidgetItem(contact.getBirthDateString()));
        table->setItem(i, 6, new QTableWidgetItem(contact.getAddress()));
    }

    QLayout *layout = centralWidget()->layout();
    QLabel *label = qobject_cast<QLabel*>(layout->itemAt(layout->count() - 1)->widget());
    if (label) {
        label->setText(QString("Всего контактов: %1").arg(contacts.size()));
    }
}

void MainWindow::onAddContact() {
    QDialog dialog(this);
    dialog.setWindowTitle("Добавить контакт");
    QFormLayout *form = new QFormLayout(&dialog);

    QLineEdit *firstNameEdit = new QLineEdit();
    QLineEdit *lastNameEdit = new QLineEdit();
    QLineEdit *middleNameEdit = new QLineEdit();
    QLineEdit *emailEdit = new QLineEdit();
    QLineEdit *phoneEdit = new QLineEdit();
    QDateEdit *birthDateEdit = new QDateEdit();
    QLineEdit *addressEdit = new QLineEdit();

    birthDateEdit->setCalendarPopup(true);
    birthDateEdit->setDisplayFormat("dd.MM.yyyy");
    birthDateEdit->setDate(QDate::currentDate());
    birthDateEdit->setMaximumDate(QDate::currentDate());

    QLineEdit *additionalPhonesEdit = new QLineEdit();
    additionalPhonesEdit->setPlaceholderText("Доп. телефоны через запятую");

    form->addRow("Имя *:", firstNameEdit);
    form->addRow("Фамилия *:", lastNameEdit);
    form->addRow("Отчество:", middleNameEdit);
    form->addRow("Email *:", emailEdit);
    form->addRow("Телефон *:", phoneEdit);
    form->addRow("Доп. телефоны:", additionalPhonesEdit);
    form->addRow("Дата рождения:", birthDateEdit);
    form->addRow("Адрес:", addressEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, &dialog);
    form->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString firstName = Control::normalizeSpaces(firstNameEdit->text());
        QString lastName = Control::normalizeSpaces(lastNameEdit->text());
        QString email = Control::normalizeSpaces(emailEdit->text());
        QString phone = phoneEdit->text();

        if (!Control::controlName(firstName)) {
            showError("Имя должно начинаться с буквы и содержать только буквы, цифры, дефисы и пробелы");
            return;
        }
        if (!Control::controlName(lastName)) {
            showError("Фамилия должна начинаться с буквы и содержать только буквы, цифры, дефисы и пробелы");
            return;
        }
        if (!Control::controlEmail(email)) {
            showError("Неверный формат email");
            return;
        }
        if (!Control::controlPhone(phone)) {
            showError("Неверный формат телефона. Примеры: +78121234567, 8(812)123-45-67");
            return;
        }

        Contact contact(firstName, lastName, email, Control::normalizePhone(phone));

        QString middleName = Control::normalizeSpaces(middleNameEdit->text());
        if (!middleName.isEmpty()) {
            if (!Control::controlName(middleName)) {
                showError("Отчество должно начинаться с буквы");
                return;
            }
            contact.setMiddleName(middleName);
        }

        QDate birthDate = birthDateEdit->date();
        if (birthDate.isValid() && birthDate != QDate::currentDate()) {
            if (!Control::controlBirthDate(birthDate)) {
                showError("Дата рождения не может быть в будущем");
                return;
            }
            contact.setBirthDate(birthDate);
        }

        QString address = addressEdit->text().trimmed();
        if (!address.isEmpty()) {
            contact.setAddress(address);
        }

        QString additionalPhones = additionalPhonesEdit->text();
        if (!additionalPhones.isEmpty()) {
            QStringList phones = additionalPhones.split(",", Qt::SkipEmptyParts);
            for (QString& phone : phones) {
                phone = phone.trimmed();
                if (Control::controlPhone(phone)) {
                    contact.addPhone(Control::normalizePhone(phone));
                } else {
                    showError(QString("Неверный формат телефона: %1").arg(phone));
                    return;
                }
            }
        }

        if (phoneBook.addContact(contact)) {
            refreshTable();
            showSuccess("Контакт добавлен!");
        }
    }
}

void MainWindow::onDeleteContact() {
    int row = table->currentRow();
    if (row >= 0) {
        QMessageBox::StandardButton reply;
        reply = QMessageBox::question(this, "Подтверждение",
                                     "Удалить выбранный контакт?",
                                     QMessageBox::Yes | QMessageBox::No);
        if (reply == QMessageBox::Yes) {
            if (phoneBook.removeContact(row)) {
                refreshTable();
                showSuccess("Контакт удален");
            }
        }
    } else {
        showError("Выберите контакт для удаления");
    }
}

void MainWindow::onEditContact() {
    int row = table->currentRow();
    if (row < 0) {
        showError("Выберите контакт для редактирования");
        return;
    }

    QVector<Contact> contacts = phoneBook.getAllContacts();
    if (row >= contacts.size()) return;

    Contact contact = contacts[row];

    QDialog dialog(this);
    dialog.setWindowTitle("Редактировать контакт");
    QFormLayout *form = new QFormLayout(&dialog);

    QLineEdit *firstNameEdit = new QLineEdit(contact.getFirstName());
    QLineEdit *lastNameEdit = new QLineEdit(contact.getLastName());
    QLineEdit *middleNameEdit = new QLineEdit(contact.getMiddleName());
    QLineEdit *emailEdit = new QLineEdit(contact.getEmail());
    QLineEdit *phoneEdit = new QLineEdit();
    QDateEdit *birthDateEdit = new QDateEdit();
    QLineEdit *addressEdit = new QLineEdit(contact.getAddress());

    birthDateEdit->setCalendarPopup(true);
    birthDateEdit->setDisplayFormat("dd.MM.yyyy");
    birthDateEdit->setDate(contact.getBirthDate());
    birthDateEdit->setMaximumDate(QDate::currentDate());

    auto phones = contact.getPhones();
    if (!phones.isEmpty()) {
        phoneEdit->setText(phones[0]);
    }

    QLineEdit *additionalPhonesEdit = new QLineEdit();
    QString additionalPhones;
    for (int i = 1; i < phones.size(); i++) {
        if (i > 1) additionalPhones += ", ";
        additionalPhones += phones[i];
    }
    additionalPhonesEdit->setText(additionalPhones);

    form->addRow("Имя *:", firstNameEdit);
    form->addRow("Фамилия *:", lastNameEdit);
    form->addRow("Отчество:", middleNameEdit);
    form->addRow("Email *:", emailEdit);
    form->addRow("Телефон *:", phoneEdit);
    form->addRow("Доп. телефоны:", additionalPhonesEdit);
    form->addRow("Дата рождения:", birthDateEdit);
    form->addRow("Адрес:", addressEdit);

    QDialogButtonBox *buttonBox = new QDialogButtonBox(
        QDialogButtonBox::Ok | QDialogButtonBox::Cancel,
        Qt::Horizontal, &dialog);
    form->addRow(buttonBox);

    connect(buttonBox, &QDialogButtonBox::accepted, &dialog, &QDialog::accept);
    connect(buttonBox, &QDialogButtonBox::rejected, &dialog, &QDialog::reject);

    if (dialog.exec() == QDialog::Accepted) {
        QString firstName = Control::normalizeSpaces(firstNameEdit->text());
        QString lastName = Control::normalizeSpaces(lastNameEdit->text());
        QString email = Control::normalizeSpaces(emailEdit->text());
        QString phone = phoneEdit->text();

        if (!Control::controlName(firstName) ||
            !Control::controlName(lastName) ||
            !Control::controlEmail(email) ||
            !Control::controlPhone(phone)) {
            showError("Неверные данные");
            return;
        }

        Contact newContact(firstName, lastName, email, Control::normalizePhone(phone));
        newContact.setMiddleName(Control::normalizeSpaces(middleNameEdit->text()));
        newContact.setBirthDate(birthDateEdit->date());
        newContact.setAddress(addressEdit->text().trimmed());

        QString additionalPhones = additionalPhonesEdit->text();
        if (!additionalPhones.isEmpty()) {
            QStringList phones = additionalPhones.split(",", Qt::SkipEmptyParts);
            for (QString& phone : phones) {
                phone = phone.trimmed();
                if (Control::controlPhone(phone)) {
                    newContact.addPhone(Control::normalizePhone(phone));
                }
            }
        }

        if (phoneBook.editContact(row, newContact)) {
            refreshTable();
            showSuccess("Контакт обновлен");
        }
    }
}

void MainWindow::onSearch() {
    QString query = searchEdit->text().trimmed();
    if (query.isEmpty()) {
        refreshTable();
        return;
    }

    QVector<Contact> results = phoneBook.searchContacts(query);
    table->setRowCount(results.size());

    for (int i = 0; i < results.size(); i++) {
        const Contact& contact = results[i];
        table->setItem(i, 0, new QTableWidgetItem(contact.getFirstName()));
        table->setItem(i, 1, new QTableWidgetItem(contact.getLastName()));
        table->setItem(i, 2, new QTableWidgetItem(contact.getMiddleName()));
        table->setItem(i, 3, new QTableWidgetItem(contact.getEmail()));

        auto phones = contact.getPhones();
        QString phoneStr;
        for (int j = 0; j < phones.size(); j++) {
            phoneStr += phones[j];
            if (j < phones.size() - 1) phoneStr += "\n";
        }
        table->setItem(i, 4, new QTableWidgetItem(phoneStr));

        table->setItem(i, 5, new QTableWidgetItem(contact.getBirthDateString()));
        table->setItem(i, 6, new QTableWidgetItem(contact.getAddress()));
    }
}

void MainWindow::onSave() {
    QString filename = QFileDialog::getSaveFileName(this, "Сохранить файл",
                                                   "contacts.txt",
                                                   "Текстовые файлы (*.txt)");
    if (!filename.isEmpty()) {
        phoneBook.saveToFile(filename);
        showSuccess("Данные сохранены в файл: " + filename);
    }
}

void MainWindow::onLoad() {
    QString filename = QFileDialog::getOpenFileName(this, "Загрузить файл",
                                                   "",
                                                   "Текстовые файлы (*.txt)");
    if (!filename.isEmpty()) {
        phoneBook.loadFromFile(filename);
        refreshTable();
        showSuccess("Данные загружены из файла: " + filename);
    }
}

void MainWindow::onSort(int column) {
    QString field;
    switch (column) {
        case 0: field = "firstName"; break;
        case 1: field = "lastName"; break;
        case 3: field = "email"; break;
        case 5: field = "birthDate"; break;
        default: return;
    }

    phoneBook.sortByField(field);
    refreshTable();
}

void MainWindow::showError(const QString& message) {
    QMessageBox::critical(this, "Ошибка", message);
}

void MainWindow::showSuccess(const QString& message) {
    QMessageBox::information(this, "Успех", message);
}
