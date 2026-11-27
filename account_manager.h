#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H

#include <QObject>
#include <QString>
#include <QSqlDatabase>

class AccountManager : public QObject
{
    Q_OBJECT

public:
    explicit AccountManager(const QString &connectionName = QString(), QObject *parent = nullptr);

public slots:
    void handleLoginRequest(const QString &username, const QString &password);
    void handleLogoutRequest();
    void handleRegistrationRequest(const QString &username, const QString &password);

signals:
    void loginSuccess();
    void logoutSuccess();
    void loginFailure(const QString &message);
    void registrationSuccess();
    void registrationFailure(const QString &message);
    void userAuthenticated(int userId, const QString &username, const QString &role);

private:
    QSqlDatabase resolveDatabase() const;
    QString m_connectionName;
};

#endif
