#include "profilepage.h"
#include "ui_profilepage.h"

#include <QPushButton>

ProfilePage::ProfilePage(QWidget *parent)
    : QWidget(parent), ui(new Ui::ProfilePage)
{
    ui->setupUi(this);

    setStyleSheet(R"(
        QWidget#ProfilePage { background-color: #f4f7fb; }
        QLabel { color: #243247; }
        QLabel#headingLabel { font-size: 25px; font-weight: bold; }
        QLabel#subtitleLabel { font-size: 13px; color: #64748b; }
        QLabel#fieldTitleLabel, QLabel#roleTitleLabel {
            font-size: 12px; font-weight: bold; color: #475569;
        }
        QLabel#usernameValueLabel, QLabel#roleValueLabel {
            background-color: white; border: 1px solid #cbd5e1;
            border-radius: 7px; padding: 12px; font-size: 14px;
        }
        QLabel#messageLabel { font-size: 12px; color: #64748b; }
        QPushButton#backButton {
            background-color: #4263eb; color: white; border: none;
            border-radius: 7px; padding: 12px; font-weight: bold;
            font-size: 13px;
        }
        QPushButton#backButton:hover { background-color: #364fc7; }
        QPushButton#backButton:disabled { background-color: #a5b4fc; }
    )");

    connect(ui->backButton, &QPushButton::clicked,
            this, &ProfilePage::backToHomeRequested);

    clearUserInfo();
}

ProfilePage::~ProfilePage()
{
    delete ui;
}

void ProfilePage::setUserInfo(const QString &username, const QString &role)
{
    if (username.trimmed().isEmpty()) {
        clearUserInfo();
        return;
    }

    ui->usernameValueLabel->setText(username);
    ui->roleValueLabel->setText(role.trimmed().isEmpty() ? "Not specified" : role);
    ui->messageLabel->setText("Account information for the signed-in user.");
    userInfoAvailable = true;
}

void ProfilePage::clearUserInfo()
{
    ui->usernameValueLabel->setText("Not signed in");
    ui->roleValueLabel->setText("—");
    ui->messageLabel->setText("Sign in to view your profile.");
    userInfoAvailable = false;
}

bool ProfilePage::hasUserInfo() const
{
    return userInfoAvailable;
}
