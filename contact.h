#ifndef CONTACT_H
#define CONTACT_H

#include <QString>
#include <QVector>
#include <QDate>

class Contact {
public:
    Contact();
    Contact(const QString& first, const QString& last,
            const QString& email, const QString& phone);

    QString getFirstName() const { return firstName; }
    QString getLastName() const { return lastName; }
    QString getMiddleName() const { return middleName; }
    QString getEmail() const { return email; }
    QDate getBirthDate() const { return birthDate; }
    QString getBirthDateString() const;
    QString getAddress() const { return address; }
    QVector<QString> getPhones() const { return phones; }

    void setFirstName(const QString& name) { firstName = name; }
    void setLastName(const QString& name) { lastName = name; }
    void setMiddleName(const QString& name) { middleName = name; }
    void setEmail(const QString& mail) { email = mail; }
    void setBirthDate(const QDate& date) { birthDate = date; }
    void setBirthDate(const QString& dateStr);
    void setAddress(const QString& addr) { address = addr; }
    void addPhone(const QString& phone) { phones.append(phone); }
    void removePhone(int index) {
        if (index >= 0 && index < phones.size())
            phones.remove(index);
    }
    void clearPhones() { phones.clear(); }

private:
    QString firstName;
    QString lastName;
    QString middleName;
    QString email;
    QDate birthDate;
    QString address;
    QVector<QString> phones;
};

#endif
