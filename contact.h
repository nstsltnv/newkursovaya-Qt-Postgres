#ifndef CONTACT_H
#define CONTACT_H

#include <string>
#include <vector>
using namespace std;

class Contact {
public:
    Contact();
    Contact(const string& first, const string& last,
        const string& email, const string& phone);

    string getFirstName() const { return firstName; }
    string getLastName() const { return lastName; }
    string getMiddleName() const { return middleName; }
    string getEmail() const { return email; }
    string getBirthDate() const { return birthDate; }
    string getAddress() const { return address; }
    vector<string> getPhones() const { return phones; }

    void setFirstName(const string& name) { firstName = name; }
    void setLastName(const string& name) { lastName = name; }
    void setMiddleName(const string& name) { middleName = name; }
    void setEmail(const string& mail) { email = mail; }
    void setBirthDate(const string& date) { birthDate = date; }
    void setAddress(const string& addr) { address = addr; }
    void addPhone(const string& phone) { phones.push_back(phone); }
    void removePhone(int index) {
        if (index >= 0 and index < phones.size())
            phones.erase(phones.begin() + index);
    }

private:
    string firstName;
    string lastName;
    string middleName;
    string email;
    string birthDate;
    string address;
    vector<string> phones;
};

#endif