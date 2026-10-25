#include "noteentrypage.h"
#include "loginpage.h"
#include "registerpage.h"
#include "profilepage.h"
#include "mainwindow.h"

#include <QApplication>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    LoginPage loginPage;
    RegisterPage registerPage;
    ProfilePage profilePage;
    NoteEntryPage noteEntryPage;
    MainWindow mainWindow;

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

    QObject::connect(&mainWindow, &MainWindow::createNoteRequested,
                     &noteEntryPage, [&]() {
                         mainWindow.hide();
                         noteEntryPage.show();
                     });

    QObject::connect(&noteEntryPage, &NoteEntryPage::cancelRequested,
                     &mainWindow, [&]() {
                         noteEntryPage.hide();
                         mainWindow.show();
                     });

    loginPage.show();
    return app.exec();
}
