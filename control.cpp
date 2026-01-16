#include "control.h"
#include <QDebug>

const QRegularExpression Control::nameRegex(
    R"(^[A-Za-zА-Яа-яЁё][A-Za-zА-Яа-яЁё0-9]*(?:[-\s][A-Za-zА-Яа-яЁё0-9]+)*$)"
);

const QRegularExpression Control::emailRegex(
    R"(^[a-zA-Z0-9._%+-]+@[a-zA-Z0-9.-]+\.[a-zA-Z]{2,}$)"
);

const QRegularExpression Control::phoneRegex(
    R"(^(\+7|8)[\s\-]?\(?\d{3}\)?[\s\-]?\d{3}[\s\-]?\d{2}[\s\-]?\d{2}$)"
);

bool Control::controlName(const QString& name) {
    QString trimmed = normalizeSpaces(name);
    if (trimmed.isEmpty()) return false;

    QRegularExpressionMatch match = nameRegex.match(trimmed);
    if (!match.hasMatch()) {
        return false;
    }

    if (trimmed.startsWith('-') || trimmed.endsWith('-')) {
        return false;
    }

    if (trimmed.contains("--")) {
        return false;
    }

    return true;
}

bool Control::controlEmail(const QString& email) {
    QString normalized = normalizeSpaces(email);
    if (normalized.isEmpty()) return false;

    normalized = normalized.replace(" @", "@").replace("@ ", "@");

    QRegularExpressionMatch match = emailRegex.match(normalized);
    return match.hasMatch();
}

bool Control::controlPhone(const QString& phone) {
    QString normalized = normalizePhone(phone);
    QRegularExpressionMatch match = phoneRegex.match(normalized);

    if (!match.hasMatch()) {
        return false;
    }

    normalized = normalized.replace(QRegularExpression(R"([\s\(\)\-])"), "");

    if (normalized.startsWith("+7")) {
        return normalized.length() == 12;
    } else if (normalized.startsWith("8")) {
        return normalized.length() == 11;
    }

    return false;
}

bool Control::controlBirthDate(const QDate& date) {
    if (!date.isValid()) return true;

    QDate current = QDate::currentDate();
    if (date > current) return false;

    return true;
}

bool Control::controlBirthDateString(const QString& date) {
    if (date.isEmpty()) return true;

    QDate d = QDate::fromString(date, "dd.MM.yyyy");
    return controlBirthDate(d);
}

QString Control::normalizeSpaces(const QString& str) {
    return str.trimmed();
}

QString Control::normalizePhone(const QString& phone) {
    QString result = phone.trimmed();
    return result;
}
