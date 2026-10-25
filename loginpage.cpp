#include "loginpage.h"

#include <QFont>
#include <QFrame>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QVBoxLayout>

LoginPage::LoginPage(QWidget *parent)
    : QWidget(parent)
{
    setObjectName("LoginPage");
    setWindowTitle("StudyNotes - Log In");
    resize(470, 540);
    setMinimumSize(400, 480);

    setStyleSheet(R"(
        QWidget#LoginPage { background-color: #f4f7fb; }
        QLabel { color: #243247; }
        QFrame#loginCard { background-color: white; border: 1px solid #e2e8f0; border-radius: 12px; }
        QLineEdit { color: #222222; background-color: white; border: 1px solid #cbd5e1;
                    border-radius: 7px; padding: 10px; font-size: 13px; min-height: 20px; }
        QLineEdit:focus { border: 2px solid #4263eb; }
        QPushButton#loginButton { background-color: #4263eb; color: white; border: none;
                                  border-radius: 7px; padding: 12px; font-weight: bold; font-size: 13px; }
        QPushButton#loginButton:hover { background-color: #364fc7; }
        QPushButton#loginButton:disabled { background-color: #94a3b8; }
        QPushButton#registerButton { background-color: transparent; color: #4263eb;
                                     border: none; padding: 9px; font-size: 12px; }
        QPushButton#registerButton:hover { text-decoration: underline; }
    )");

    auto *outerLayout = new QVBoxLayout(this);
    outerLayout->setContentsMargins(28, 28, 28, 28);
    outerLayout->addStretch();

    auto *card = new QFrame(this);
    card->setObjectName("loginCard");
    auto *cardLayout = new QVBoxLayout(card);
    cardLayout->setContentsMargins(32, 32, 32, 32);
    cardLayout->setSpacing(12);

    auto *title = new QLabel("Welcome Back!", card);
    QFont titleFont = title->font();
    titleFont.setPointSize(20);
    titleFont.setBold(true);
    title->setFont(titleFont);
    title->setAlignment(Qt::AlignCenter);
    cardLayout->addWidget(title);

    auto *subtitle = new QLabel("Log in to your StudyNotes account", card);
    subtitle->setAlignment(Qt::AlignCenter);
    subtitle->setStyleSheet("color: #64748b; font-size: 12px;");
    cardLayout->addWidget(subtitle);
    cardLayout->addSpacing(18);

    auto *usernameLabel = new QLabel("Username", card);
    cardLayout->addWidget(usernameLabel);
    usernameInput = new QLineEdit(card);
    usernameInput->setObjectName("usernameInput");
    usernameInput->setPlaceholderText("Enter your username");
    cardLayout->addWidget(usernameInput);

    auto *passwordLabel = new QLabel("Password", card);
    cardLayout->addWidget(passwordLabel);
    passwordInput = new QLineEdit(card);
    passwordInput->setObjectName("passwordInput");
    passwordInput->setPlaceholderText("Enter your password");
    passwordInput->setEchoMode(QLineEdit::Password);
    cardLayout->addWidget(passwordInput);

    statusLabel = new QLabel(card);
    statusLabel->setObjectName("statusLabel");
    statusLabel->setWordWrap(true);
    statusLabel->setAlignment(Qt::AlignCenter);
    statusLabel->hide();
    cardLayout->addWidget(statusLabel);

    cardLayout->addSpacing(8);
    loginButton = new QPushButton("Log In", card);
    loginButton->setObjectName("loginButton");
    cardLayout->addWidget(loginButton);

    auto *registerButton = new QPushButton("Don't have an account? Register", card);
    registerButton->setObjectName("registerButton");
    cardLayout->addWidget(registerButton);

    outerLayout->addWidget(card);
    outerLayout->addStretch();

    connect(loginButton, &QPushButton::clicked, this, &LoginPage::submitLogin);
    connect(passwordInput, &QLineEdit::returnPressed, this, &LoginPage::submitLogin);
    connect(registerButton, &QPushButton::clicked, this, &LoginPage::registrationRequested);
}

void LoginPage::submitLogin()
{
    const QString username = usernameInput->text().trimmed();
    const QString password = passwordInput->text();

    if (username.isEmpty() || password.isEmpty()) {
        setStatus("Please enter both your username and password.", false);
        return;
    }

    setLoginInProgress(true);
    emit loginRequested(username, password);
}

void LoginPage::setStatus(const QString &message, bool success)
{
    statusLabel->setText(message);
    statusLabel->setStyleSheet(success
        ? "color: #15803d; font-size: 12px;"
        : "color: #b91c1c; font-size: 12px;");
    statusLabel->setVisible(!message.isEmpty());
}

void LoginPage::showAuthenticationSuccess()
{
    setLoginInProgress(false);
    passwordInput->clear();
    setStatus("Login successful!", true);
    emit authenticatedViewRequested();
}

void LoginPage::showAuthenticationFailure(const QString &message)
{
    setLoginInProgress(false);
    passwordInput->clear();
    setStatus(message.isEmpty() ? "Incorrect username or password." : message, false);
}

void LoginPage::setLoginInProgress(bool inProgress)
{
    loginButton->setDisabled(inProgress);
    loginButton->setText(inProgress ? "Logging in..." : "Log In");
    if (inProgress)
        setStatus("", false);
}

void LoginPage::clearFields()
{
    usernameInput->clear();
    passwordInput->clear();
    setLoginInProgress(false);
    setStatus("", false);
}
