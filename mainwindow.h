#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QDateEdit>
#include <QPushButton>
#include <QLabel>
#include "contact.h"
#include "phonebook.h"
#include "dbmanager.h"

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

private slots:
    void onAddContact();
    void onDeleteContact();
    void onEditContact();
    void onSearch();
    void onSaveToFile();
    void onLoadFromFile();
    void onConnectToDatabase();
    void onSwitchToFileMode();
    void onSwitchToDatabaseMode();
    void onDatabaseError(const QString &error);

    // Слоты для сортировки
    void onSortFirstNameAsc();
    void onSortFirstNameDesc();
    void onSortLastNameAsc();
    void onSortLastNameDesc();
    void onSortEmailAsc();
    void onSortEmailDesc();
    void onSortBirthDateAsc();
    void onSortBirthDateDesc();

private:
    void setupUI();
    void setupMenu();
    void refreshTable();
    void showError(const QString &message);
    void showSuccess(const QString &message);
    void updateStatus();
    bool showContactDialog(Contact &contact, const QString &title);
    void displayContacts(const QVector<Contact> &contacts);

    enum Mode { FileMode, DatabaseMode };

    Mode currentMode;
    PhoneBook phoneBook;
    DBManager *dbManager;

    QTableWidget *table;
    QLineEdit *searchEdit;
    QLabel *statusLabel;
    QLabel *countLabel;
};

#endif
