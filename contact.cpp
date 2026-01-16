#include "contact.h"

Contact::Contact(): firstName(""), lastName(""), middleName(""),
    email(""), birthDate(""), address("") {}

Contact::Contact(const string& first, const string& last,
    const string& mail, const string& phone): firstName(first), lastName(last), email(mail) {
    phones.push_back(phone);
}