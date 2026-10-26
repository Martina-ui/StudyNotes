#include "noteentrypage.h"
#include "mynotespage.h"
#include "loginpage.h"
#include "registerpage.h"
#include "profilepage.h"
#include "mainwindow.h"
#include "databasemanager.h"
#include "noteservice.h"

#include <QApplication>
#include <QDebug>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    QString databaseError;

    if (!DatabaseManager::openConnection(&databaseError)) {
        qWarning().noquote()
            << "StudyNotes database connection failed:"
            << databaseError;
    } else {
        qInfo() << "StudyNotes database connected successfully!";
    }

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
    QObject::connect(&noteEntryPage, &NoteEntryPage::noteCreationRequested,
                     &noteEntryPage,
                     [&](const QString &title,
                         const QString &content,
                         const QString &category,
                         const QString &visibility) {

                         NoteDraft draft;
                         draft.title = title;
                         draft.content = content;
                         draft.category = category;
                         draft.visibility = visibility;

                         bool userIdValid = false;
                         int currentUserId = qEnvironmentVariableIntValue(
                             "STUDYNOTES_TEST_USER_ID", &userIdValid);

                         if (!userIdValid || currentUserId <= 0) {
                             noteEntryPage.showSaveFailure(
                                 "No test user configured. Please set STUDYNOTES_TEST_USER_ID.");
                             return;
                         }

                         NoteSaveResult result = NoteService::saveNewNote(
                             DatabaseManager::database(),
                             currentUserId,
                             draft
                             );

                         if (result.success) {
                             noteEntryPage.showSaveSuccess();
                             qInfo() << "Note saved successfully. ID:" << result.noteId;
                         } else {
                             noteEntryPage.showSaveFailure(result.message);
                             qWarning() << "Unable to save note:" << result.message;
                         }
                     });
    loginPage.show();
    return app.exec();
}
