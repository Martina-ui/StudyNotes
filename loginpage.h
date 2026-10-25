#ifndef LOGINPAGE_H
#define LOGINPAGE_H

#include <QWidget>
#include <QString>

class QLineEdit;
class QLabel;
class QPushButton;

class LoginPage : public QWidget
{
    Q_OBJECT

public:
    explicit LoginPage(QWidget *parent = nullptr);
    ~LoginPage() override = default;

    void showAuthenticationSuccess();
    void showAuthenticationFailure(const QString &message = QString());
    void setLoginInProgress(bool inProgress);
    void clearFields();

signals:
    void loginRequested(const QString &username, const QString &password);
    void registrationRequested();
    void authenticatedViewRequested();

private:
    void submitLogin();
    void setStatus(const QString &message, bool success);

    QLineEdit *usernameInput;
    QLineEdit *passwordInput;
    QLabel *statusLabel;
    QPushButton *loginButton;
};

#endif // LOGINPAGE_H
