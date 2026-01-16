#ifndef PHONEBOOK_H
#define PHONEBOOK_H

#include "contact.h"
#include <vector>
#include <string>
using namespace std;

class PhoneBook {
public:
    PhoneBook();
    ~PhoneBook();

    bool addContact(const Contact& contact);
    bool removeContact(int index);
    bool editContact(int index, const Contact& newContact);
    vector<Contact> searchContacts(const string& query) const;
    void sortByField(const string& field);
    void saveToFile(const string& filename) const;
    void loadFromFile(const string& filename);
    vector<Contact> getAllContacts() const { return contacts; }

private:
    vector<Contact> contacts;
};

#endif