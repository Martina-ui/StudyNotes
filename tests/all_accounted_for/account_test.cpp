#include <account_manager.h>
#include <sessionmanager.h>
#include <adminservice.h>

#include <QApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QVariant>
#include <sodium.h>

//again, basic structure of Lewis's test code, fitted to my stuff
namespace{

//if it aint broke. this pigeon is gonna have a slick pair of shades
struct ClipboardPigeon {
    int criminalBreadcrumbCount = 0;

    bool verify(bool passed, const QString &normalDescription)
    {
        if (passed) {
            qInfo().noquote() << "PASS:" << normalDescription;
        } else {
            qCritical().noquote() << "FAIL:" << normalDescription;
            ++criminalBreadcrumbCount;
        }
        return passed;
    }
};

struct SignalTracker { //this struct is new. tests are validated if the right signal is sent. this struct catches every signal with lambdas
    bool regSuccess = false;
    bool regFailed = false;
    QString lastRegError;

    bool loginSuccess = false;
    bool loginFailed = false;
    QString lastLoginError;

    bool userAuthEmitted = false;
    int authUserId = -1;
    QString authUsername;
    QString authRole;

    bool logoutSuccess = false;

    void reset()
    {
        regSuccess = false;
        regFailed = false;
        lastRegError.clear();
        loginSuccess = false;
        loginFailed = false;
        lastLoginError.clear();
        userAuthEmitted = false;
        authUserId = -1;
        authUsername.clear();
        authRole.clear();
        logoutSuccess = false;
    }

    void attach(AccountManager &manager)
    {
        QObject::connect(&manager, &AccountManager::registrationSuccess, [&]() {
            regSuccess = true;
        });
        QObject::connect(&manager, &AccountManager::registrationFailure, [&](const QString &msg) {
            regFailed = true;
            lastRegError = msg;
        });
        QObject::connect(&manager, &AccountManager::loginSuccess, [&]() {
            loginSuccess = true;
        });
        QObject::connect(&manager, &AccountManager::loginFailure, [&](const QString &msg) {
            loginFailed = true;
            lastLoginError = msg;
        });
        QObject::connect(&manager, &AccountManager::userAuthenticated,
                         [&](int userId, const QString &username, const QString &role) {
                             userAuthEmitted = true;
                             authUserId = userId;
                             authUsername = username;
                             authRole = role;
                         });
        QObject::connect(&manager, &AccountManager::logoutSuccess, [&]() {
            logoutSuccess = true;
        });
    }
};

//SIMULATING TEST DATA
// instead of making a mini notes db table, i make a mini users table (that emulates murat's schema)
bool assembleTheTemporaryFurniture(QSqlDatabase &database, ClipboardPigeon &failboat) //make sure object names are distinct but not too close together ugh
{
    // These are test-only tables, NOT a replacement for Murat's PostgreSQL.
    // only simulating a user test table. my current favorite dragon is the bakunawa
    QSqlQuery screwdriverGoblin(database);
    const bool foreignKeysEnabled = failboat.verify(
        screwdriverGoblin.exec("PRAGMA foreign_keys = ON"),
        "Enable foreign key checks");
    const bool usersCreated = failboat.verify(
        screwdriverGoblin.exec(
            "CREATE TABLE IF NOT EXISTS users ("
            "user_id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "username VARCHAR(50) NOT NULL UNIQUE, "
            "password_hash TEXT NOT NULL, "
            "role VARCHAR(10) NOT NULL DEFAULT 'Student' "
            "CHECK (role IN ('Student', 'Admin')), "
            "created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)"),
        "Create temporary users table");
    const bool notesCreated = failboat.verify(
        screwdriverGoblin.exec(
            "CREATE TABLE notes ("
            "note_id INTEGER PRIMARY KEY AUTOINCREMENT, "
            "user_id INTEGER NOT NULL REFERENCES users(user_id), "
            "title VARCHAR(200) NOT NULL, "
            "content TEXT NOT NULL, "
            "category VARCHAR(100), "
            "visibility VARCHAR(10) NOT NULL "
            "CHECK (visibility IN ('Public', 'Private')))"),
        "Create temporary notes table");
    return foreignKeysEnabled && usersCreated && notesCreated;
}

int countUsers(QSqlDatabase &database) {
    QSqlQuery query(database);
    if(!query.exec("SELECT COUNT (*) FROM users") || !query.next()) {
        return -1;
    }
    return query.value(0).toInt(); //this line stops a warining during compile
}

void runInvalidRegistrationTests(AccountManager &manager, SignalTracker &tracker, QSqlDatabase &db, ClipboardPigeon &pigeon)
{
    struct BadRegistrationCase {
        const char *description;
        QString username;
        QString password;
        QString expectedError;
    };

    const BadRegistrationCase cases[] = {
        {"Reject empty username", "", "ValidPass123", "Username cannot be empty."},
        {"Reject whitespace-only username", "    ", "ValidPass123", "Username cannot be empty."},
        {"Reject username longer than 15 characters", QString(16, 'A'), "ValidPass123",
         "Username must be 15 characters or less."},
        {"Reject empty password", "validuser", "", "Password cannot be empty."},
        {"Reject password shorter than 8 characters", "validuser", "short7",
         "Password must be at least 8 characters long."}
    };

    for (const BadRegistrationCase &testCase : cases) {
        tracker.reset();
        manager.handleRegistrationRequest(testCase.username, testCase.password);
        pigeon.verify(
            !tracker.regSuccess && tracker.regFailed &&
                tracker.lastRegError == testCase.expectedError &&
                countUsers(db) == 0,
            QString::fromUtf8(testCase.description));
    }
}

void runInvalidLoginTests(AccountManager &manager, SignalTracker &tracker, ClipboardPigeon &pigeon)
{
    // 1. No credentials
    tracker.reset();
    manager.handleLoginRequest("", "");
    pigeon.verify(!tracker.loginSuccess && tracker.loginFailed &&
                      tracker.lastLoginError == "Please enter your username and password.",
                  "Reject login with empty credentials");

    // 2. User not found
    tracker.reset();
    manager.handleLoginRequest("ghost_student", "ValidPass123");
    pigeon.verify(!tracker.loginSuccess && tracker.loginFailed &&
                      tracker.lastLoginError == "Incorrect username or password.",
                  "Reject login for nonexistent username");

    // 3. Wrong password
    tracker.reset();
    manager.handleLoginRequest("tawusi_melek", "WrongPassword!");
    pigeon.verify(!tracker.loginSuccess && tracker.loginFailed &&
                      tracker.lastLoginError == "Incorrect username or password.",
                  "Reject login with incorrect password");
}

void runSessionAndAuthorizationTests(AccountManager &manager, SignalTracker &tracker, QSqlDatabase &db, ClipboardPigeon &pigeon)
{
    SessionManager &session = SessionManager::instance();

    // 8. Authenticated session behavior (Student login -> verify session fields -> logout -> verify cleared)
    tracker.reset();
    manager.handleLoginRequest("tawusi_melek", "Pc0ck_Ange1");
    const int studentId = session.userId();
    pigeon.verify(
        session.isLoggedIn() && studentId > 0 &&
            session.role() == "Student" && !session.isAdmin(),
        "SessionManager stores Student userId, role, and non-admin status after login");

    // 9. Unauthorized admin access (Student attempting AdminService actions + spoofed session ID)
    pigeon.verify(
        !AdminService::isAuthorizedAdmin(db, studentId),
        "Deny Admin authorization check for logged-in Student");

    const AdminNotesResult studentLoadAttempt = AdminService::loadPublicNotes(db, studentId);
    pigeon.verify(
        !studentLoadAttempt.success &&
            studentLoadAttempt.message == "Access denied. Admin privileges are required.",
        "Block Student from loading Admin moderation notes");

    // Create an Admin user and a Public note in the test DB to test valid Admin access
    QSqlQuery setupQuery(db);
    setupQuery.exec(
        "INSERT INTO users (username, password_hash, role) "
        "VALUES ('Simurgh_fr_fr', 'placeholder_hash', 'Admin')");
    const int adminId = setupQuery.lastInsertId().toInt();

    setupQuery.prepare(
        "INSERT INTO notes (user_id, title, content, category, visibility) "
        "VALUES (:uid, 'Public Study Guide', 'Shared notes', 'CS', 'Public')");
    setupQuery.bindValue(":uid", studentId);
    setupQuery.exec();

    // student cannot spoof admin's userId into AdminService
    pigeon.verify(
        !AdminService::isAuthorizedAdmin(db, adminId),
        "Block Student session from passing Admin userId to AdminService");

    manager.handleLogoutRequest();
    pigeon.verify(
        !session.isLoggedIn() && session.userId() == -1 &&
            session.role().isEmpty() && !session.isAdmin(),
        "SessionManager resets userId, role, and flags on logout");

    // 10. Student/Admin authorization behavior (valid admin session + db verification)
    session.startSession(adminId, "Admin");
    pigeon.verify(
        session.isLoggedIn() && session.isAdmin() &&
            AdminService::isAuthorizedAdmin(db, adminId),
        "Authorize valid Admin user when session and database role match");

    const AdminNotesResult adminLoadAttempt = AdminService::loadPublicNotes(db, adminId);
    pigeon.verify(
        adminLoadAttempt.success && adminLoadAttempt.notes.size() == 1 &&
            adminLoadAttempt.notes.first().title == "Public Study Guide",
        "Allow authorized Admin to load public notes for moderation");

    session.clearSession();
}

} //namespace

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    ClipboardPigeon clipboardPigeon;
    if (!clipboardPigeon.verify(sodium_init() >= 0, "Initialize libsodium")) {
        return 1;
    }

    QTemporaryDir secretGoblinLair;
    if (!clipboardPigeon.verify(secretGoblinLair.isValid(), "Create temporary test folder")) {
        return 1;
    }

    constexpr const char *connectionName = "studynotes_account_smoke"; //why are they called smoke tests anyways?
    {
        QSqlDatabase pocketSizedDatabase =
            QSqlDatabase::addDatabase("QSQLITE", connectionName);
        pocketSizedDatabase.setDatabaseName(
            secretGoblinLair.filePath("account_test.sqlite"));

        if (!clipboardPigeon.verify(pocketSizedDatabase.open(),
                                    "Open temporary test database")) {
            qCritical().noquote() << pocketSizedDatabase.lastError().text();
        } else if (assembleTheTemporaryFurniture(pocketSizedDatabase, clipboardPigeon)) {
            AccountManager accountManager(connectionName);
            SignalTracker tracker;
            tracker.attach(accountManager);

            // 1. Invalid/incomplete registration
            runInvalidRegistrationTests(accountManager, tracker, pocketSizedDatabase, clipboardPigeon);

            // 2. Valid registration (also tests username trimming & default 'Student' role)
            tracker.reset();
            accountManager.handleRegistrationRequest("  tawusi_melek  ", "Pc0ck_Ange1");
            clipboardPigeon.verify(
                tracker.regSuccess && !tracker.regFailed && countUsers(pocketSizedDatabase) == 1,
                "Register valid new user and store in database");

            // 3. Duplicate username rejection
            tracker.reset();
            accountManager.handleRegistrationRequest("tawusi_melek", "Yazidism101");
            clipboardPigeon.verify(
                !tracker.regSuccess && tracker.regFailed &&
                    tracker.lastRegError == "This username is already taken." &&
                    countUsers(pocketSizedDatabase) == 1,
                "Reject duplicate username registration");

            // 4. Invalid login attempts
            runInvalidLoginTests(accountManager, tracker, clipboardPigeon);

            // 5. Valid login
            tracker.reset();
            accountManager.handleLoginRequest("  tawusi_melek ", "Pc0ck_Ange1");
            clipboardPigeon.verify(
                tracker.loginSuccess && !tracker.loginFailed &&
                    tracker.userAuthEmitted &&
                    tracker.authUserId > 0 &&
                    tracker.authUsername == "tawusi_melek" &&
                    tracker.authRole == "Student" &&
                    SessionManager::instance().isLoggedIn(),
                "Authenticate valid user, emit user details, and start session");

            // 6. Logout
            tracker.reset();
            accountManager.handleLogoutRequest();
            clipboardPigeon.verify(
                tracker.logoutSuccess && !SessionManager::instance().isLoggedIn(),
                "Log out active user and clear session");

            // 7. Protected functionality after logout (cannot log out again without active session)
            tracker.reset();
            accountManager.handleLogoutRequest();
            clipboardPigeon.verify(
                !tracker.logoutSuccess && !SessionManager::instance().isLoggedIn(),
                "Ignore duplicate logout request when no active session exists");

            // 8-10. Session and authorization tests
            runSessionAndAuthorizationTests(accountManager,tracker,pocketSizedDatabase, clipboardPigeon);
        }
        pocketSizedDatabase.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (clipboardPigeon.criminalBreadcrumbCount == 0) {
        qInfo().noquote() << "ALL ACCOUNT MANAGER SMOKE TESTS PASSED";
        return 0;
    }
    qCritical().noquote() << "TESTS FAILED:" << clipboardPigeon.criminalBreadcrumbCount;
    return 1;

} //main
