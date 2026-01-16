#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QTableWidget>
#include <QLineEdit>
#include <QDateEdit>
#include "contact.h"
#include "phonebook.h"

class MainWindow : public QMainWindow {
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);

private slots:
    void onAddContact();
    void onDeleteContact();
    void onEditContact();
    void onSearch();
    void onSave();
    void onLoad();
    void onSort(int column);

private:
    void setupUI();
    void refreshTable();
    void showError(const QString& message);
    void showSuccess(const QString& message);

    PhoneBook phoneBook;
    QTableWidget *table;
    QLineEdit *searchEdit;
};

#endif
