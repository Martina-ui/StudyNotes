#include "noteentrypage.h"
#include "mynotespage.h"
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
    MyNotesPage myNotesPage;
    MainWindow mainWindow;

    bool noteOpenedFromMyNotes = false;

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

    QObject::connect(&mainWindow, &MainWindow::myNotesRequested,
                     &myNotesPage, [&]() {
                         mainWindow.hide();
                         myNotesPage.show();
                     });

    QObject::connect(&myNotesPage, &MyNotesPage::backRequested,
                     &mainWindow, [&]() {
                         myNotesPage.hide();
                         mainWindow.show();
                     });

    QObject::connect(&mainWindow, &MainWindow::createNoteRequested,
                     &noteEntryPage, [&]() {
                         noteOpenedFromMyNotes = false;
                         mainWindow.hide();
                         noteEntryPage.show();
                     });

    QObject::connect(&myNotesPage, &MyNotesPage::createNoteRequested,
                     &noteEntryPage, [&]() {
                         noteOpenedFromMyNotes = true;
                         myNotesPage.hide();
                         noteEntryPage.show();
                     });

    QObject::connect(&noteEntryPage, &NoteEntryPage::cancelRequested,
                     &mainWindow, [&]() {
                         noteEntryPage.hide();
                         if (noteOpenedFromMyNotes) {
                             myNotesPage.show();
                         } else {
                             mainWindow.show();
                         }
                     });

    loginPage.show();
    //mainWindow.show();
    return app.exec();
}
