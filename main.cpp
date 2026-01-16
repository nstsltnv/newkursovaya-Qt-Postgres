#include <QApplication>
#include <QSqlDatabase>
#include <QDebug>
#include <QMessageBox>
#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    // Проверяем драйверы БД
    qDebug() << "=== ПРОВЕРКА ДРАЙВЕРОВ БАЗЫ ДАННЫХ ===";
    QStringList drivers = QSqlDatabase::drivers();

    qDebug() << "Доступные драйверы:";
    for (const QString &driver : drivers) {
        qDebug() << "  - " << driver;
    }

    // Проверяем PostgreSQL драйвер
    bool hasPostgreSQL = drivers.contains("QPSQL") || drivers.contains("QPSQL7");

    if (hasPostgreSQL) {
        qDebug() << "✅ PostgreSQL драйвер найден!";
    } else {
        qDebug() << "⚠️ PostgreSQL драйвер НЕ найден!";
        QMessageBox::warning(nullptr, "Внимание",
            "Драйвер PostgreSQL не найден.\n"
            "Убедитесь, что в папке с программой есть:\n"
            "1. qsqlpsql.dll (драйвер Qt)\n"
            "2. libpq.dll (PostgreSQL 18)\n"
            "3. libssl-3-x64.dll\n"
            "4. libcrypto-3-x64.dll\n\n"
            "Программа продолжит работу с предупреждением.");
    }

    MainWindow window;
    window.show();

    return app.exec();
}
