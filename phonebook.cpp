#include "phonebook.h"
#include "control.h"
#include <QFile>
#include <QTextStream>
#include <QDebug>
#include <algorithm>

PhoneBook::PhoneBook() {}
PhoneBook::~PhoneBook() {}

bool PhoneBook::addContact(const Contact& contact) {
    if (contact.getFirstName().isEmpty() ||
        contact.getLastName().isEmpty() ||
        contact.getEmail().isEmpty() ||
        contact.getPhones().isEmpty()) {
        return false;
    }

    contacts.append(contact);
    return true;
}

bool PhoneBook::removeContact(int index) {
    if (index < 0 || index >= contacts.size()) {
        return false;
    }
    contacts.remove(index);
    return true;
}

bool PhoneBook::editContact(int index, const Contact& newContact) {
    if (index < 0 || index >= contacts.size()) {
        return false;
    }
    contacts[index] = newContact;
    return true;
}

QVector<Contact> PhoneBook::searchContacts(const QString& query) const {
    QVector<Contact> result;
    QString lowerQuery = query.toLower();

    for (const auto& contact : contacts) {
        QString first = contact.getFirstName().toLower();
        QString last = contact.getLastName().toLower();
        QString email = contact.getEmail().toLower();
        QString middle = contact.getMiddleName().toLower();
        QString address = contact.getAddress().toLower();

        if (first.contains(lowerQuery) ||
            last.contains(lowerQuery) ||
            email.contains(lowerQuery) ||
            middle.contains(lowerQuery) ||
            address.contains(lowerQuery)) {
            result.append(contact);
        }
    }
    return result;
}

void PhoneBook::sortByField(const QString& field) {
    if (field == "firstName") {
        std::sort(contacts.begin(), contacts.end(),
            [](const Contact& a, const Contact& b) {
                return a.getFirstName() < b.getFirstName();
            });
    } else if (field == "lastName") {
        std::sort(contacts.begin(), contacts.end(),
            [](const Contact& a, const Contact& b) {
                return a.getLastName() < b.getLastName();
            });
    } else if (field == "email") {
        std::sort(contacts.begin(), contacts.end(),
            [](const Contact& a, const Contact& b) {
                return a.getEmail() < b.getEmail();
            });
    } else if (field == "birthDate") {
        std::sort(contacts.begin(), contacts.end(),
            [](const Contact& a, const Contact& b) {
                return a.getBirthDate() < b.getBirthDate();
            });
    }
}

void PhoneBook::saveToFile(const QString& filename) const {
    QFile file(filename);

    if (!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
        qDebug() << "Ошибка открытия файла для записи:" << file.errorString();
        return;
    }

    QTextStream out(&file);
    out.setCodec("UTF-8");

    for (const auto& contact : contacts) {
        out << contact.getFirstName() << ";"
            << contact.getLastName() << ";"
            << contact.getMiddleName() << ";"
            << contact.getEmail() << ";"
            << contact.getBirthDateString() << ";"
            << contact.getAddress() << ";";

        auto phones = contact.getPhones();
        for (int i = 0; i < phones.size(); ++i) {
            out << phones[i];
            if (i < phones.size() - 1) out << ",";
        }
        out << "\n";
    }

    file.close();
    qDebug() << "Данные сохранены в файл:" << filename;
}

void PhoneBook::loadFromFile(const QString& filename) {
    QFile file(filename);

    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qDebug() << "Ошибка открытия файла для чтения:" << file.errorString();
        return;
    }

    contacts.clear();
    QTextStream in(&file);
    in.setCodec("UTF-8");

    while (!in.atEnd()) {
        QString line = in.readLine();
        if (line.isEmpty()) continue;

        QStringList tokens = line.split(";");

        if (tokens.size() >= 6) {
            Contact contact;
            contact.setFirstName(tokens[0]);
            contact.setLastName(tokens[1]);
            contact.setMiddleName(tokens[2]);
            contact.setEmail(tokens[3]);

            if (!tokens[4].isEmpty()) {
                contact.setBirthDate(tokens[4]);
            }

            contact.setAddress(tokens[5]);

            if (tokens.size() > 6 && !tokens[6].isEmpty()) {
                QStringList phones = tokens[6].split(",");
                for (const QString& phone : phones) {
                    if (!phone.isEmpty()) {
                        contact.addPhone(phone);
                    }
                }
            }

            contacts.append(contact);
        }
    }

    file.close();
    qDebug() << "Данные загружены из файла:" << filename;
}
