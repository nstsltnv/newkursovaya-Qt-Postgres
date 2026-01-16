#ifndef VALIDATOR_H
#define VALIDATOR_H

#include <string>
using namespace std;

class Control {
public:
    static bool controlName(const string& name);
    static bool controlEmail(const string& email);
    static bool controlPhone(const string& phone);
    static bool controlBirthDate(const string& date);
    static string normalizeSpaces(const string& str);
    static string normalizePhone(const string& phone);
};

#endif