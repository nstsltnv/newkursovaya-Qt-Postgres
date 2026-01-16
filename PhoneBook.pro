QT += core gui widgets sql

CONFIG += c++17

TARGET = PhoneBook
TEMPLATE = app

SOURCES += \
    main.cpp \
    mainwindow.cpp \
    contact.cpp \
    phonebook.cpp \
    control.cpp \
    dbmanager.cpp

HEADERS += \
    mainwindow.h \
    contact.h \
    phonebook.h \
    control.h \
    dbmanager.h

FORMS += \
    mainwindow.ui

DEFINES += QT_DEPRECATED_WARNINGS

CONFIG += debug

win32 {
    CONFIG += console
    # Без линковки к PostgreSQL - Qt загружает DLL динамически
}
