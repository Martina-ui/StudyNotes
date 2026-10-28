#include "loginpage.h"
#include "registerpage.h"
#include "profilepage.h"
#include "mainwindow.h"
#include "account_manager.h" //just now realizing I broke the naming convention, later if not a big deal

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LoginPage loginPage;
    RegisterPage registerPage;
    ProfilePage profilePage;
    MainWindow mainWindow;
    AccountManager accountManager;

    QObject::connect(&loginPage, &LoginPage::registrationRequested,
                     &registerPage, [&]() {
                         loginPage.hide();
                         registerPage.show();
                     });

    QObject::connect(&registerPage, &RegisterPage::loginRequested,
                     &loginPage, [&]() {
                         registerPage.hide();
                         loginPage.show();
                     });

    QObject::connect(&mainWindow, &MainWindow::profileRequested,
                     &profilePage, [&]() {
                         mainWindow.hide();
                         profilePage.show();
                     });

    QObject::connect(&profilePage, &ProfilePage::backToHomeRequested,
                     &mainWindow, [&]() {
                         profilePage.hide();
                         mainWindow.show();
                     });
    //all of my connections are below this point -J
    accountManager.connectToDatabase();

    QObject::connect(&loginPage, &LoginPage::loginRequested,
                     &accountManager, &AccountManager::handleLoginRequest);

    QObject::connect(&accountManager, &AccountManager::loginSuccess,
                     &loginPage, &LoginPage::showAuthenticationSuccess);

    QObject::connect(&accountManager, &AccountManager::loginFailure,
                     &loginPage, &LoginPage::showAuthenticationFailure);

    QObject::connect(&registerPage, &RegisterPage::registrationRequested,
                     &accountManager, &AccountManager::handleRegistrationRequest);
    //I made emits for if the register fails in the account manager, do we need to update registerpage to show that?
    loginPage.show();
    return app.exec();
}