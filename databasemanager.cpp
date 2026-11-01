
#include "databasemanager.h"

#include <QSqlError>
#include <QtGlobal>

namespace {
constexpr const char *CONNECTION_NAME = "studynotes";
}

bool DatabaseManager::openConnection(QString *errorMessage)
{
    QSqlDatabase db;

    if (QSqlDatabase::contains(CONNECTION_NAME)) {
        db = QSqlDatabase::database(CONNECTION_NAME, false);
    } else {
        db = QSqlDatabase::addDatabase("QPSQL", CONNECTION_NAME);
    }

    db.setHostName(
        qEnvironmentVariable("STUDYNOTES_DB_HOST", "localhost")
    );

    db.setPort(
        qEnvironmentVariableIntValue("STUDYNOTES_DB_PORT")
    );

    if (db.port() == 0) {
        db.setPort(5432);
    }

    db.setDatabaseName(
        qEnvironmentVariable("STUDYNOTES_DB_NAME", "studynotes_db")
    );

    db.setUserName(
        qEnvironmentVariable("STUDYNOTES_DB_USER", "postgres")
    );

    db.setPassword(
        qEnvironmentVariable("STUDYNOTES_DB_PASSWORD")
    );

    if (!db.open()) {
        if (errorMessage) {
            *errorMessage = db.lastError().text();
        }
        return false;
    }

    if (errorMessage) {
        errorMessage->clear();
    }

    return true;
}

QSqlDatabase DatabaseManager::database()
{
    return QSqlDatabase::database(CONNECTION_NAME, false);
}
