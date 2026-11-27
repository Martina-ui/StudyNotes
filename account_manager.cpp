#include "account_manager.h"
#include "databasemanager.h"
#include "sessionmanager.h"
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <sodium.h>

AccountManager::AccountManager(const QString &connectionName, QObject *parent)
    : QObject(parent), m_connectionName(connectionName)
{
}

//uses the test database when a connection name is provided
QSqlDatabase AccountManager::resolveDatabase() const
{
    if (!m_connectionName.isEmpty()) {
        return QSqlDatabase::database(m_connectionName);
    }
    return DatabaseManager::database();
}

void AccountManager::handleLoginRequest(const QString &username, const QString &password)
{
    QSqlDatabase db = resolveDatabase();
    if (!db.isValid() || !db.isOpen()) {
        qWarning() << "AccountManager: Database connection is unavailable.";
        emit loginFailure("Database connection is unavailable.");
        return;
    }
    QString cleanUsername = username.trimmed();
    if (cleanUsername.isEmpty() || password.isEmpty()) {
        emit loginFailure("Please enter your username and password.");
        return;
    }
    QSqlQuery query(db);
    query.prepare("SELECT user_id, username, role, password_hash FROM users WHERE username = :username");
    query.bindValue(":username", cleanUsername);
    if (!query.exec()) {
        qWarning() << "AccountManager: Login query failed:" << query.lastError().text();
        emit loginFailure("Unable to log in. Please try again.");
        return;
    }
    if (!query.next()) {
        emit loginFailure("Incorrect username or password.");
        return;
    }
    int userId = query.value(0).toInt();
    QString authenticatedUsername = query.value(1).toString();
    QString userRole = query.value(2).toString();
    QByteArray storedHash = query.value(3).toString().toUtf8();
    QByteArray passwordData = password.toUtf8();
    int result = crypto_pwhash_str_verify(storedHash.constData(), passwordData.constData(), static_cast<unsigned long long>(passwordData.size()));
    if (result == 0) {
        qInfo() << "AccountManager: Login successful for user:" << authenticatedUsername;
        //starts a session with the authenticated user's ID and role
        SessionManager::instance().startSession(userId, userRole);
        emit userAuthenticated(userId, authenticatedUsername, userRole);
        emit loginSuccess();
    } else {
        emit loginFailure("Incorrect username or password.");
    }
}

void AccountManager::handleRegistrationRequest(const QString &username, const QString &password)
{
    QSqlDatabase db = resolveDatabase();
    if (!db.isValid() || !db.isOpen()) {
        qWarning() << "AccountManager: Database connection is unavailable.";
        emit registrationFailure("Database connection is unavailable.");
        return;
    }
    QString cleanUsername = username.trimmed();
    if (cleanUsername.isEmpty()) {
        emit registrationFailure("Username cannot be empty.");
        return;
    }
    if (cleanUsername.length() > 15) {
        emit registrationFailure("Username must be 15 characters or less.");
        return;
    }
    if (password.isEmpty()) {
        emit registrationFailure("Password cannot be empty.");
        return;
    }
    QByteArray passwordData = password.toUtf8();
    if (password.length() < 8) {
        emit registrationFailure("Password must be at least 8 characters long.");
        return;
    }
    char hashedPassword[crypto_pwhash_STRBYTES];
    int result = crypto_pwhash_str_alg(hashedPassword, passwordData.constData(), static_cast<unsigned long long>(passwordData.size()), crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE, crypto_pwhash_ALG_ARGON2ID13);
    if (result != 0) {
        qWarning() << "AccountManager: Password hashing failed.";
        emit registrationFailure("Unable to securely store password.");
        return;
    }
    QSqlQuery query(db);
    query.prepare("INSERT INTO users (username, password_hash) VALUES (:username, :password_hash)");
    query.bindValue(":username", cleanUsername);
    query.bindValue(":password_hash", QString::fromUtf8(hashedPassword));
    if (!query.exec()) {
        const QSqlError error = query.lastError();
        qWarning() << "AccountManager: Registration failed:" << error.text();
        const QString errorCode = error.nativeErrorCode();

        //handles duplicate usernames in PostgreSQL and SQLite
        const bool postgresDuplicate = db.driverName() == "QPSQL" && errorCode == "23505";
        const bool sqliteDuplicate = db.driverName() == "QSQLITE" && (errorCode == "2067" || errorCode == "19") && error.databaseText().contains("UNIQUE constraint failed: users.username", Qt::CaseInsensitive);
        if (postgresDuplicate || sqliteDuplicate) {
            emit registrationFailure("This username is already taken.");
        } else {
            emit registrationFailure("Unable to register. Please try again.");
        }
        return;
    }
    qInfo() << "AccountManager: Successfully registered user:" << cleanUsername;
    emit registrationSuccess();
}

void AccountManager::handleLogoutRequest()
{
    //prevents logout requests when no active session exists
    if (!SessionManager::instance().isLoggedIn()) {
        qWarning() << "AccountManager: Attempted to log out, but no active session was found.";
        return;
    }
    SessionManager::instance().clearSession();
    qInfo() << "AccountManager: User successfully logged out backend-side.";
    emit logoutSuccess();
}
