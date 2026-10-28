#ifndef ACCOUNT_MANAGER_H
#define ACCOUNT_MANAGER_H

#include <QObject>
#include <QString>

class AccountManager : public QObject
{
    Q_OBJECT

public:
    explicit AccountManager(QObject *parent = nullptr);
    bool connectToDatabase();

public slots:
    void handleLoginRequest(const QString &username, const QString &password);
    void handleRegistrationRequest(const QString &username, const QString &password);

signals:
    void loginSuccess();
    void loginFailure(const QString &errorMessage);
    void registrationSuccess();
    void registrationFailure(const QString &errorMessage);
}; //jumped into the deep end and drowned 5 times over just to get partially there

#endif

