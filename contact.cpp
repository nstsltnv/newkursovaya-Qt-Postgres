#include "contact.h"
#include <QDate>

Contact::Contact(): firstName(""), lastName(""), middleName(""),
    email(""), birthDate(QDate()), address("") {}

Contact::Contact(const QString& first, const QString& last,
    const QString& mail, const QString& phone):
    firstName(first), lastName(last), email(mail), birthDate(QDate()) {
    phones.append(phone);
}

QString Contact::getBirthDateString() const {
    if (birthDate.isValid()) {
        return birthDate.toString("dd.MM.yyyy");
    }
    return "";
}

void Contact::setBirthDate(const QString& dateStr) {
    birthDate = QDate::fromString(dateStr, "dd.MM.yyyy");
}
