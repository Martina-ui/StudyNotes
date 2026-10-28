#include "account_manager.h"

#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QDebug>
#include <QCryptographicHash> //password hasher

//most notes on this cpp file are for writing out documentation on a separate file later
//for now, this is for me and anyone else who'll have to go back and fix my shit

//note: hasher might explode, let me know if it does

AccountManager::AccountManager(QObject *parent) : QObject(parent){}

bool AccountManager::connectToDatabase()
{
    QSqlDatabase db = QSqlDatabase::addDatabase("QPSQL");

    db.setHostName("localhost"); //these are my credentials for my local Postgresql
    db.setDatabaseName("users"); //change these to the right database when we have to
    db.setUserName("postgres");  //or change to your personal parameters if you're testing that
    db.setPassword("postgres");  //I forgot my password fuck
    db.setPort(5432);
    if (!db.open()) {
        qDebug() << "Failed to connect to PostgreSQL database:" << db.lastError().text(); //stole these debugging messages
        return false; //all the qDebug() lines were <iostream> lines until I found out this is better.
    }
    qDebug() << "Successfully connected to PostgreSQL.";
    return true;
}

void AccountManager::handleLoginRequest(const QString &username, const QString &password)
{
    if (!QSqlDatabase::database().isOpen()) {
        qDebug() << "Database is not opening.";
        emit loginFailure("Server database concection is lost.");
        return;
    }

    QSqlQuery query;
    query.prepare("SELECT password_hash FROM users WHERE username = :username");
    query.bindValue(":username", username);
    if (!query.exec()) {
        qDebug() << "Database query failed execution:" << query.lastError().text();
        emit loginFailure("idk how to write queries, sorry.");
        return;
    }

    if (query.next()) {
        QString storedPassword = query.value(0).toString();
        QByteArray loginPasswordData = password.toUtf8(); //login hasher
        QString passwordHash = QCryptographicHash::hash(loginPasswordData, QCryptographicHash::Sha256).toHex();
        if (storedPassword == passwordHash) {
            qDebug() << "Login verified for user:" << username;
            emit loginSuccess();
        }
        else {
            qDebug() << "Login failed: Incorrect password.";
            emit loginFailure("Incorrect username or password.");
        }
    }
    else {
        qDebug() << "Login failed: Username not found.";
        emit loginFailure("Incorrect username or password.");
    }
}


void AccountManager::handleRegistrationRequest(const QString &username, const QString &password)
{
    if (!QSqlDatabase::database().isOpen()) {
        qDebug() << "Database is fucked.";
        emit loginFailure("Server database concection is lost.");
        return;
    }
    // password hasher
    QByteArray passwordData = password.toUtf8();
    QString hashedBytes = QCryptographicHash::hash(passwordData, QCryptographicHash::Sha256).toHex();
    QSqlQuery query;
    query.prepare("INSERT INTO users (username, password_hash) VALUES (:username, :password_hash)");
    query.bindValue(":username", username);
    query.bindValue(":password_hash", hashedBytes);
    if (!query.exec()) {
        qDebug() << "Registration failed:" << query.lastError().text();
        if (query.lastError().nativeErrorCode() == "23505") { //error code for duplicates
            emit registrationFailure("This username is already taken.");
        }
        else {
            emit registrationFailure("The database is wrong somehow.");
        }
        return;
    }
    qDebug() << "Successfully registered new account for:" << username;
    emit registrationSuccess();
}