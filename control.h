#ifndef CONTROL_H
#define CONTROL_H

#include <QString>
#include <QDate>
#include <QRegularExpression>

class Control {
public:
    static bool controlName(const QString& name);
    static bool controlEmail(const QString& email);
    static bool controlPhone(const QString& phone);
    static bool controlBirthDate(const QDate& date);
    static bool controlBirthDateString(const QString& date);

    static QString normalizeSpaces(const QString& str);
    static QString normalizePhone(const QString& phone);

private:
    static const QRegularExpression nameRegex;
    static const QRegularExpression emailRegex;
    static const QRegularExpression phoneRegex;
};

#endif
