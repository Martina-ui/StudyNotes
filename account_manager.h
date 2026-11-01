#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H

#include <QObject>
#include <QString>

class AccountManager : public QObject
{
    Q_OBJECT

public:
    explicit AccountManager(QObject *parent = nullptr);

public slots:
    void handleLoginRequest(const QString &username, const QString &password);
    void handleRegistrationRequest(const QString &username, const QString &password);

signals:
    void loginSuccess();
    void loginFailure(const QString &message);
    void registrationSuccess();
    void registrationFailure(const QString &message);
    void userAuthenticated(int userId, const QString &username, const QString &role);
};

#endif
