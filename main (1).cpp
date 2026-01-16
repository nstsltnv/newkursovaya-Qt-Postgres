#include <iostream>
#include <string>
#include "phonebook.h"
#include "contact.h"
#include "control.h"
#include <windows.h>
using namespace std;

void setRussianConsole() {
    SetConsoleOutputCP(1251);
    SetConsoleCP(1251);
}
void printMenu() {
    cout << "\n=== ТЕЛЕФОННЫЙ СПРАВОЧНИК ===\n";
    cout << "1. Добавить контакт\n";
    cout << "2. Показать все контакты\n";
    cout << "3. Найти контакт\n";
    cout << "4. Удалить контакт\n";
    cout << "5. Сохранить в файл\n";
    cout << "6. Загрузить из файла\n";
    cout << "7. Сортировать контакты\n";
    cout << "0. Выход\n";
    cout << "Выберите действие: ";
}

void printContact(const Contact& contact, int index) {
    cout << index + 1 << ". " << contact.getLastName() << " "
        << contact.getFirstName() << " " << contact.getMiddleName() << "\n";
    cout << "   Email: " << contact.getEmail() << "\n";
    cout << "   Дата рождения: " << contact.getBirthDate() << "\n";
    cout << "   Адрес: " << contact.getAddress() << "\n";
    cout << "   Телефоны: ";
    auto phones = contact.getPhones();
    for (size_t i = 0; i < phones.size(); ++i) {
        cout << phones[i];
        if (i < phones.size() - 1) cout << ", ";
    }
    cout << "\n" << endl;
}

int main() {
    setRussianConsole();
    PhoneBook phoneBook;
    int choice;

    do {
        printMenu();
        cin >> choice;
        cin.ignore();

        switch (choice) {
        case 1: {
            string first, last, email, phone;

            cout << "Имя: ";
            getline(cin, first);
            first = Control::normalizeSpaces(first);

            cout << "Фамилия: ";
            getline(cin, last);
            last = Control::normalizeSpaces(last);

            cout << "Email: ";
            getline(cin, email);
            email = Control::normalizeSpaces(email);

            cout << "Телефон: ";
            getline(cin, phone);

            if (!Control::controlName(first) or
                !Control::controlName(last) or
                !Control::controlEmail(email) or
                !Control::controlPhone(phone)) {
                cout << "Ошибка: неверные данные!\n";
                break;
            }

            Contact contact(first, last, email, phone);

            string middle, birth, address;

            cout << "Отчество (необязательно): ";
            getline(cin, middle);
            middle = Control::normalizeSpaces(middle);
            if (!middle.empty() and Control::controlName(middle)) {
                contact.setMiddleName(middle);
            }

            cout << "Дата рождения (ДД.ММ.ГГГГ, необязательно): ";
            getline(cin, birth);
            birth = Control::normalizeSpaces(birth);
            if (!birth.empty() and Control::controlBirthDate(birth)) {
                contact.setBirthDate(birth);
            }

            cout << "Адрес (необязательно): ";
            getline(cin, address);
            contact.setAddress(address);

            if (phoneBook.addContact(contact)) {
                cout << "Контакт добавлен!\n";
            }
            else {
                cout << "Ошибка добавления контакта!\n";
            }
            break;
        }

        case 2: {
            auto contacts = phoneBook.getAllContacts();
            if (contacts.empty()) {
                cout << "Телефонная книга пуста!\n";
            }
            else {
                cout << "\n=== ВСЕ КОНТАКТЫ ===\n";
                for (size_t i = 0; i < contacts.size(); ++i) {
                    printContact(contacts[i], i);
                }
            }
            break;
        }

        case 3: {
            string query;
            cout << "Введите поисковый запрос: ";
            getline(cin, query);

            auto results = phoneBook.searchContacts(query);
            if (results.empty()) {
                cout << "Контакты не найдены!\n";
            }
            else {
                cout << "\n=== РЕЗУЛЬТАТЫ ПОИСКА ===\n";
                for (size_t i = 0; i < results.size(); ++i) {
                    printContact(results[i], i);
                }
            }
            break;
        }

        case 4: {
            auto contacts = phoneBook.getAllContacts();
            if (contacts.empty()) {
                cout << "Телефонная книга пуста!\n";
                break;
            }

            cout << "\n=== ВЫБЕРИТЕ КОНТАКТ ДЛЯ УДАЛЕНИЯ ===\n";
            for (size_t i = 0; i < contacts.size(); ++i) {
                cout << i + 1 << ". " << contacts[i].getLastName()
                    << " " << contacts[i].getFirstName() << "\n";
            }

            int index;
            cout << "Номер контакта: ";
            cin >> index;

            if (phoneBook.removeContact(index - 1)) {
                cout << "Контакт удален!\n";
            }
            else {
                cout << "Ошибка удаления контакта!\n";
            }
            break;
        }

        case 5: {
            string filename;
            cout << "Имя файла для сохранения: ";
            getline(cin, filename);
            phoneBook.saveToFile(filename);
            break;
        }

        case 6: {
            string filename;
            cout << "Имя файла для загрузки: ";
            getline(cin, filename);
            phoneBook.loadFromFile(filename);
            break;
        }

        case 7: {
            cout << "Сортировать по (1-Имя, 2-Фамилия, 3-Email): ";
            int sortChoice;
            cin >> sortChoice;

            switch (sortChoice) {
            case 1: phoneBook.sortByField("firstName"); break;
            case 2: phoneBook.sortByField("lastName"); break;
            case 3: phoneBook.sortByField("email"); break;
            default: cout << "Неверный выбор!\n";
            }
            cout << "Контакты отсортированы!\n";
            break;
        }

        case 0:
            cout << "Выход...\n";
            break;

        default:
            cout << "Неверный выбор!\n";
        }

    } while (choice != 0);

    return 0;
}