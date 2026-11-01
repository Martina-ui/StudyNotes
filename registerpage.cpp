#include "registerpage.h"
#include "ui_registerpage.h"

#include <QLineEdit>
#include <QPushButton>

RegisterPage::RegisterPage(QWidget *parent)
    : QWidget(parent), ui(new Ui::RegisterPage)
{
    ui->setupUi(this);

    ui->passwordInput->setEchoMode(QLineEdit::Password);
    ui->confirmPasswordInput->setEchoMode(QLineEdit::Password);
    ui->messageLabel->hide();

    connect(ui->registerButton, &QPushButton::clicked,
            this, &RegisterPage::onRegisterClicked);

    connect(ui->loginButton, &QPushButton::clicked,
            this, &RegisterPage::loginRequested);
}

RegisterPage::~RegisterPage()
{
    delete ui;
}

void RegisterPage::showMessage(const QString &message, bool success)
{
    ui->messageLabel->setText(message);

    ui->messageLabel->setStyleSheet(success
                                        ? "color: #047857; font-size: 12px;"
                                        : "color: #b91c1c; font-size: 12px;");

    ui->messageLabel->show();
}

void RegisterPage::showRegistrationSuccess()
{
    showMessage("Account created successfully! You can now log in.", true);

    ui->passwordInput->clear();
    ui->confirmPasswordInput->clear();
}

void RegisterPage::showRegistrationFailure(const QString &message)
{
    showMessage(message, false);
}

void RegisterPage::onRegisterClicked()
{
    const QString username = ui->usernameInput->text().trimmed();
    const QString password = ui->passwordInput->text();
    const QString confirmation = ui->confirmPasswordInput->text();

    if (username.isEmpty() || password.isEmpty() || confirmation.isEmpty()) {
        showMessage("Please fill out all fields.");
        return;
    }

    if (password != confirmation) {
        showMessage("Passwords do not match.");
        return;
    }

    ui->messageLabel->hide();

    emit registrationRequested(username, password);
}
