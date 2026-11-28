#include "noteentrypage.h"
#include "mynotespage.h"
#include "loginpage.h"
#include "registerpage.h"
#include "profilepage.h"
#include "mainwindow.h"
#include "account_manager.h"
#include "databasemanager.h"
#include "noteservice.h"
#include "sessionmanager.h"
#include "publicnotespage.h"
#include "adminpanelpage.h"
#include "adminservice.h"

#include <QApplication>
#include <QDebug>
#include <QMessageBox>
#include <sodium.h>

int main(int argc, char *argv[])
{
    QApplication app(argc, argv);

    if (sodium_init() < 0) {
        qCritical() << "Failed to initialize libsodium.";
        return 1;
    }

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
    PublicNotesPage publicNotesPage;
    AdminPanelPage adminPanelPage;
    MainWindow mainWindow;
    AccountManager accountManager;

    bool noteOpenedFromMyNotes = false;
    int editingNoteId = -1;

    QObject::connect(&accountManager, &AccountManager::userAuthenticated,
                     &mainWindow,
                     [&](int userId, const QString &username, const QString &role) {
                         profilePage.setUserInfo(username, role);
                         bool authorizedAdmin = AdminService::isAuthorizedAdmin(DatabaseManager::database(), userId);
                         mainWindow.setAdminVisible(authorizedAdmin);
                         qInfo() << "Authenticated user:"
                                 << username
                                 << "ID:" << userId
                                 << "Role:" << role;
                     });

    QObject::connect(&mainWindow, &MainWindow::adminPanelRequested,
                     &adminPanelPage, [&]() {
                         int currentUserId = SessionManager::instance().userId();
                         AdminNotesResult result = AdminService::loadPublicNotes(
                             DatabaseManager::database(),
                             currentUserId
                             );
                         if (!result.success) {
                             adminPanelPage.clearNotes();
                             mainWindow.setAdminVisible(false);
                             qWarning() << "Admin Panel access denied:" << result.message;
                             return;
                         }
                         adminPanelPage.setNotes(result.notes);
                         mainWindow.hide();
                         adminPanelPage.show();
                     });

    QObject::connect(&adminPanelPage, &AdminPanelPage::deleteNoteRequested,
                     &adminPanelPage, [&](int noteId) {
        if (QMessageBox::question(&adminPanelPage,
                "Delete Public Note", "Permanently delete the selected public note?",
                QMessageBox::Yes | QMessageBox::Cancel, QMessageBox::Cancel)
                != QMessageBox::Yes) {
            return;
        }

        const QSqlDatabase database = DatabaseManager::database();
        const int currentUserId = SessionManager::instance().userId();
        const NoteSaveResult deletion = AdminService::deletePublicNote(
            database, currentUserId, noteId);
        if (!deletion.success) {
            adminPanelPage.showError(deletion.message);
            return;
        }

        const AdminNotesResult notes = AdminService::loadPublicNotes(database, currentUserId);
        if (notes.success) {
            adminPanelPage.setNotes(notes.notes);
        } else {
            adminPanelPage.clearNotes();
            adminPanelPage.showError("The note was deleted. " + notes.message);
        }
    });

    QObject::connect(&adminPanelPage, &AdminPanelPage::backRequested,
                     &mainWindow, [&]() {
                         adminPanelPage.hide();
                         adminPanelPage.clearNotes();
                         mainWindow.show();
                     });

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
                         if (!SessionManager::instance().isLoggedIn()) {
                             return;
                         }
                         mainWindow.hide();
                         profilePage.show();
                     });

    QObject::connect(&mainWindow, &MainWindow::logoutRequested,
                     &accountManager, &AccountManager::handleLogoutRequest);

    QObject::connect(&accountManager, &AccountManager::logoutSuccess,
                     &loginPage, [&]() {
                         loginPage.clearFields();
                         mainWindow.hide();
                         profilePage.hide();
                         myNotesPage.hide();
                         noteEntryPage.hide();
                         editingNoteId = -1;
                         noteEntryPage.setEditMode(false);
                         myNotesPage.clearNotes();
                         publicNotesPage.hide();
                         publicNotesPage.clearNotes();
                         adminPanelPage.hide();
                         adminPanelPage.clearNotes();
                         mainWindow.setAdminVisible(false);
                         noteOpenedFromMyNotes = false;
                         loginPage.show();
                     });

    QObject::connect(&profilePage, &ProfilePage::backToHomeRequested,
                     &mainWindow, [&]() {
                         profilePage.hide();
                         mainWindow.show();
                     });

    QObject::connect(&loginPage, &LoginPage::loginRequested,
                     &accountManager, &AccountManager::handleLoginRequest);

    QObject::connect(&accountManager, &AccountManager::loginSuccess,
                     &loginPage, &LoginPage::showAuthenticationSuccess);

    QObject::connect(&accountManager, &AccountManager::loginFailure,
                     &loginPage, &LoginPage::showAuthenticationFailure);

    QObject::connect(&loginPage, &LoginPage::authenticatedViewRequested,
                     &mainWindow, [&]() {
                         loginPage.hide();
                         mainWindow.show();
                     });

    QObject::connect(&registerPage, &RegisterPage::registrationRequested,
                     &accountManager, &AccountManager::handleRegistrationRequest);

    QObject::connect(&accountManager, &AccountManager::registrationSuccess,
                     &registerPage, &RegisterPage::showRegistrationSuccess);

    QObject::connect(&accountManager, &AccountManager::registrationFailure,
                     &registerPage, &RegisterPage::showRegistrationFailure);

    QObject::connect(&mainWindow, &MainWindow::myNotesRequested,
                     &myNotesPage, [&]() {
                         if (!SessionManager::instance().isLoggedIn()) {
                             return;
                         }
                         NoteLoadResult result = NoteService::loadUserNotes(
                             DatabaseManager::database(),
                             SessionManager::instance().userId()
                             );
                         if (result.success) {
                             myNotesPage.setNotes(result.notes);
                         } else {
                             myNotesPage.clearNotes();
                             myNotesPage.showError(result.message);
                         }
                         mainWindow.hide();
                         myNotesPage.show();
                     });


    QObject::connect(&mainWindow, &MainWindow::publicNotesRequested,
                     &publicNotesPage, [&]() {
                         if (!SessionManager::instance().isLoggedIn()) {
                             return;
                         }
                         NoteLoadResult result = NoteService::loadPublicNotes(
                             DatabaseManager::database()
                             );
                         if (result.success) {
                             publicNotesPage.setNotes(result.notes);
                         } else {
                             publicNotesPage.clearNotes();
                             publicNotesPage.showError(result.message);
                         }
                         mainWindow.hide();
                         publicNotesPage.show();
                     });

    QObject::connect(&publicNotesPage, &PublicNotesPage::backRequested,
                     &mainWindow, [&]() {
                         publicNotesPage.hide();
                         mainWindow.show();
                     });

    QObject::connect(&myNotesPage, &MyNotesPage::backRequested,
                     &mainWindow, [&]() {
                         myNotesPage.hide();
                         mainWindow.show();
                     });

    QObject::connect(&mainWindow, &MainWindow::createNoteRequested,
                     &noteEntryPage, [&]() {
                         if (!SessionManager::instance().isLoggedIn()) {
                             return;
                         }
                         noteOpenedFromMyNotes = false;
                         mainWindow.hide();
                         noteEntryPage.show();
                     });

    QObject::connect(&myNotesPage, &MyNotesPage::createNoteRequested,
                     &noteEntryPage, [&]() {
                         if (!SessionManager::instance().isLoggedIn()) {
                             return;
                         }
                         noteOpenedFromMyNotes = true;
                         myNotesPage.hide();
                         noteEntryPage.show();
                     });

    QObject::connect(&myNotesPage, &MyNotesPage::editNoteRequested,
                     &noteEntryPage, [&](int noteId) {
        if (!SessionManager::instance().isLoggedIn()) {
            return;
        }

        NoteLoadResult result = NoteService::loadUserNotes(
            DatabaseManager::database(),
            SessionManager::instance().userId()
        );

        if (!result.success) {
            myNotesPage.showError(result.message);
            return;
        }

        for (const NoteSummary &note : result.notes) {
            if (note.noteId == noteId) {
                editingNoteId = noteId;
                noteOpenedFromMyNotes = true;

                noteEntryPage.setNoteFields(
                    note.title, note.content,
                    note.category, note.visibility
                );

                noteEntryPage.setEditMode(true);
                myNotesPage.hide();
                noteEntryPage.show();
                return;
            }
        }

        myNotesPage.showError("Note not found or access denied.");
    });

    QObject::connect(&myNotesPage, &MyNotesPage::deleteNoteRequested,
                     &myNotesPage, [&](int noteId) {
        if (!SessionManager::instance().isLoggedIn()) {
            return;
        }

        if (QMessageBox::question(&myNotesPage,
                "Delete Note", "Are you sure you want to delete this note?")
                != QMessageBox::Yes) {
            return;
        }

        NoteSaveResult result = NoteService::deleteNote(
            DatabaseManager::database(),
            SessionManager::instance().userId(),
            noteId
        );

        if (!result.success) {
            myNotesPage.showError(result.message);
            return;
        }

        NoteLoadResult notes = NoteService::loadUserNotes(
            DatabaseManager::database(),
            SessionManager::instance().userId()
        );

        if (notes.success) {
            myNotesPage.setNotes(notes.notes);
        } else {
            myNotesPage.clearNotes();
            myNotesPage.showError(notes.message);
        }
    });

    QObject::connect(&noteEntryPage, &NoteEntryPage::cancelRequested,
                     &mainWindow, [&]() {
                         noteEntryPage.hide();
                         editingNoteId = -1;
                         noteEntryPage.setEditMode(false);
                         if (!SessionManager::instance().isLoggedIn()) {
                             loginPage.show();
                             return;
                         }
                         if (noteOpenedFromMyNotes) {
                             NoteLoadResult result = NoteService::loadUserNotes(
                                 DatabaseManager::database(),
                                 SessionManager::instance().userId()
                                 );
                             if (result.success) {
                                 myNotesPage.setNotes(result.notes);
                             } else {
                                 myNotesPage.clearNotes();
                                 myNotesPage.showError(result.message);
                             }
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
                         if (!SessionManager::instance().isLoggedIn() ||
                             SessionManager::instance().userId() <= 0) {
                             noteEntryPage.showSaveFailure(
                                 "You must be logged in to save notes.");
                             return;
                         }
                         NoteSaveResult result;

                         if (editingNoteId > 0) {
                             result = NoteService::updateNote(
                                 DatabaseManager::database(),
                                 SessionManager::instance().userId(),
                                 editingNoteId,
                                 draft
                             );
                         } else {
                             result = NoteService::saveNewNote(
                                 DatabaseManager::database(),
                                 SessionManager::instance().userId(),
                                 draft
                             );
                         }
                         if (result.success) {
                             if (editingNoteId > 0) {
                                 editingNoteId = -1;
                                 noteEntryPage.setEditMode(false);
                                 noteEntryPage.clearFields();
                                 noteEntryPage.hide();
                                 NoteLoadResult notes = NoteService::loadUserNotes(
                                     DatabaseManager::database(),
                                     SessionManager::instance().userId()
                                 );

                                 if (notes.success) {
                                     myNotesPage.setNotes(notes.notes);
                                 } else {
                                     myNotesPage.clearNotes();
                                     myNotesPage.showError(notes.message);
                                 }

                                 myNotesPage.show();
                             } else {
                                 noteEntryPage.showSaveSuccess();
                             }
                             qInfo() << "Note saved successfully. ID:" << result.noteId;
                         } else {
                             noteEntryPage.showSaveFailure(result.message);
                             qWarning() << "Unable to save note:" << result.message;
                         }
                     });
    loginPage.show();
    return app.exec();
}
