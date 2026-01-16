#include "control.h"
#include <iostream>
#include <algorithm>
using namespace std;

bool Control::controlName(const string& name) {
    if (name.empty()) return false;

    string trimmed = name;
    size_t start = trimmed.find_first_not_of(" ");
    size_t end = trimmed.find_last_not_of(" ");
    if (start != string::npos and end != string::npos) {
        trimmed = trimmed.substr(start, end - start + 1);
    }

    if (trimmed.empty()) return false;

    unsigned char first = trimmed[0];
    if (!((first >= 'A' and first <= 'Z') or
        (first >= 'a' and first <= 'z') or
        (first >= 192 and first <= 255))) {
        return false;
    }

    unsigned char last = trimmed[trimmed.length() - 1];
    if (last == '-') return false;

    for (char c : trimmed) {
        unsigned char uc = c;
        if (!((uc >= 'A' and uc <= 'Z') or
            (uc >= 'a' and uc <= 'z') or
            (uc >= '0' and uc <= '9') or
            (uc >= 192 and uc <= 255) or
            uc == '-' or uc == ' ')) {
            return false;
        }
    }

    if (trimmed.find("--") != string::npos) return false;

    return true;
}

bool Control::controlEmail(const string& email) {
    string normalized = normalizeSpaces(email);

    if (normalized.empty()) return false;

    int atPos = normalized.find('@');
    if (atPos == string::npos or atPos == 0 or atPos == normalized.length() - 1) {
        return false;
    }

    string username = normalized.substr(0, atPos);
    string domain = normalized.substr(atPos + 1);

    for (char c : username) {
        if (!isalnum(c) and c != '.' and c != '_' and c != '%' and c != '+' and c != '-') {
            return false;
        }
    }

    if (domain.find('.') == string::npos) {
        return false;
    }

    for (char c : domain) {
        if (!isalnum(c) and c != '.' and c != '-') {
            return false;
        }
    }

    return true;
}

bool Control::controlPhone(const string& phone) {
    string normalized = normalizePhone(phone);

    if (normalized.empty()) return false;

    if (normalized[0] != '+' and !isdigit(normalized[0])) {
        return false;
    }

    for (size_t i = 1; i < normalized.length(); i++) {
        if (!isdigit(normalized[i])) {
            return false;
        }
    }

    if (normalized[0] == '+') {
        if (normalized.length() != 12) return false;
    }
    else {
        if (normalized.length() != 11) return false;
    }

    return true;
}

bool Control::controlBirthDate(const string& date) {
    if (date.empty()) return true;

    if (date.length() != 10) return false;
    if (date[2] != '.' or date[5] != '.') return false;

    for (int i = 0; i < 10; i++) {
        if (i != 2 and i != 5 and !isdigit(date[i])) {
            return false;
        }
    }

    return true;
}

string Control::normalizeSpaces(const string& str) {
    string result = str;

    size_t start = result.find_first_not_of(" \t\n\r");
    size_t end = result.find_last_not_of(" \t\n\r");

    if (start == string::npos) return "";

    result = result.substr(start, end - start + 1);

    size_t atPos = result.find('@');
    if (atPos != string::npos) {
        while (atPos > 0 and isspace(result[atPos - 1])) {
            result.erase(atPos - 1, 1);
            atPos--;
        }
        while (atPos + 1 < result.length() and isspace(result[atPos + 1])) {
            result.erase(atPos + 1, 1);
        }
    }

    return result;
}

string Control::normalizePhone(const string& phone) {
    string result;

    for (size_t i = 0; i < phone.length(); i++) {
        char c = phone[i];
        if (isdigit(c) or (i == 0 and c == '+')) {
            result += c;
        }
    }
    if (result.length() == 11 and result[0] == '8') {
        result = "+7" + result.substr(1);
    }

    return result;
}