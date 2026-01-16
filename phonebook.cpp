#include "phonebook.h"
#include "control.h"
#include <fstream>
#include <iostream>
#include <sstream>
#include <algorithm>
using namespace std;

PhoneBook::PhoneBook() {}
PhoneBook::~PhoneBook() {}

bool PhoneBook::addContact(const Contact& contact) {
    if (contact.getFirstName().empty() or
        contact.getLastName().empty() or
        contact.getEmail().empty() or
        contact.getPhones().empty()) {
        return false;
    }

    contacts.push_back(contact);
    return true;
}

bool PhoneBook::removeContact(int index) {
    if (index < 0 or index >= contacts.size()) {
        return false;
    }
    contacts.erase(contacts.begin() + index);
    return true;
}

bool PhoneBook::editContact(int index, const Contact& newContact) {
    if (index < 0 or index >= contacts.size()) {
        return false;
    }
    contacts[index] = newContact;
    return true;
}

vector<Contact> PhoneBook::searchContacts(const string& query) const {
    vector<Contact> result;
    string lowerQuery = query;
    transform(lowerQuery.begin(), lowerQuery.end(), lowerQuery.begin(), ::tolower);

    for (const auto& contact : contacts) {
        string first = contact.getFirstName();
        string last = contact.getLastName();
        string email = contact.getEmail();

        transform(first.begin(), first.end(), first.begin(), ::tolower);
        transform(last.begin(), last.end(), last.begin(), ::tolower);
        transform(email.begin(), email.end(), email.begin(), ::tolower);

        if (first.find(lowerQuery) != string::npos or
            last.find(lowerQuery) != string::npos or
            email.find(lowerQuery) != string::npos) {
            result.push_back(contact);
        }
    }
    return result;
}

void PhoneBook::sortByField(const string& field) {
    if (field == "firstName") {
        sort(contacts.begin(), contacts.end(),
            [](const Contact& a, const Contact& b) {
                return a.getFirstName() < b.getFirstName();
            });
    }
    else if (field == "lastName") {
        sort(contacts.begin(), contacts.end(),
            [](const Contact& a, const Contact& b) {
                return a.getLastName() < b.getLastName();
            });
    }
    else if (field == "email") {
        sort(contacts.begin(), contacts.end(),
            [](const Contact& a, const Contact& b) {
                return a.getEmail() < b.getEmail();
            });
    }
}

void PhoneBook::saveToFile(const string& filename) const {
    ofstream file(filename);
    if (!file.is_open()) {
        cout << "Ошибка открытия файла для записи!" << endl;
        return;
    }

    for (const auto& contact : contacts) {
        file << contact.getFirstName() << ";"
            << contact.getLastName() << ";"
            << contact.getMiddleName() << ";"
            << contact.getEmail() << ";"
            << contact.getBirthDate() << ";"
            << contact.getAddress() << ";";

        auto phones = contact.getPhones();
        for (size_t i = 0; i < phones.size(); ++i) {
            file << phones[i];
            if (i < phones.size() - 1) file << ",";
        }
        file << "\n";
    }
    file.close();
    cout << "Данные сохранены в файл: " << filename << endl;
}

void PhoneBook::loadFromFile(const string& filename) {
    ifstream file(filename);
    if (!file.is_open()) {
        cout << "Ошибка открытия файла для чтения!" << endl;
        return;
    }

    contacts.clear();
    string line;

    while (getline(file, line)) {
        stringstream ss(line);
        string token;
        vector<string> tokens;

        while (getline(ss, token, ';')) {
            tokens.push_back(token);
        }

        if (tokens.size() >= 6) {
            Contact contact;
            contact.setFirstName(tokens[0]);
            contact.setLastName(tokens[1]);
            contact.setMiddleName(tokens[2]);
            contact.setEmail(tokens[3]);
            contact.setBirthDate(tokens[4]);
            contact.setAddress(tokens[5]);

            if (tokens.size() > 6) {
                stringstream phoneStream(tokens[6]);
                string phone;
                while (getline(phoneStream, phone, ',')) {
                    if (!phone.empty()) {
                        contact.addPhone(phone);
                    }
                }
            }

            contacts.push_back(contact);
        }
    }
    file.close();
    cout << "Данные загружены из файла: " << filename << endl;
}