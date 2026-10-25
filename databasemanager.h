#ifndef DATABASEMANAGER_H
#define DATABASEMANAGER_H

#include <QSqlDatabase>
#include <QString>

class DatabaseManager
{
public:
    static bool openConnection(QString *errorMessage = nullptr);
    static QSqlDatabase database();
};

#endif