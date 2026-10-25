#ifndef REGISTERPAGE_H
#define REGISTERPAGE_H

#include <QWidget>
#include <QString>

QT_BEGIN_NAMESPACE
namespace Ui { class RegisterPage; }
QT_END_NAMESPACE

class RegisterPage : public QWidget
{
    Q_OBJECT

public:
    explicit RegisterPage(QWidget *parent = nullptr);
    ~RegisterPage();

signals:
    void registrationRequested(const QString &username, const QString &password);
    void loginRequested();

private slots:
    void onRegisterClicked();

private:
    Ui::RegisterPage *ui;
    void showMessage(const QString &message, bool success = false);
};

#endif
