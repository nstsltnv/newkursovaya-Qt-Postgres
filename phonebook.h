#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#include "contact.h"
#include <QVector>
#include <QString>

class PhoneBook {
public:
    PhoneBook();
    ~PhoneBook();

    bool addContact(const Contact& contact);
    bool removeContact(int index);
    bool editContact(int index, const Contact& newContact);
    QVector<Contact> searchContacts(const QString& query) const;
    void sortByField(const QString& field);

    void saveToFile(const QString& filename) const;
    void loadFromFile(const QString& filename);

    QVector<Contact> getAllContacts() const { return contacts; }

private:
    QVector<Contact> contacts;
};

#endif
