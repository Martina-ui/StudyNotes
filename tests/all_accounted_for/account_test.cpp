#include <account_manager.h>
#include <sessionmanager.h>
#include <adminservice.h>

#include <QApplication>
#include <QByteArray>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QVariant>
#include <sodium.h>

namespace {

struct TestRunner {
    int failureCount = 0;
    bool verify(bool passed, const QString &description)
    {
        if (passed) {
            qInfo().noquote() << "PASS:" << description;
        } else {
            qCritical().noquote() << "FAIL:" << description;
            ++failureCount;
        }
        return passed;
    }
};

//tracks signals emitted by AccountManager
struct SignalTracker {
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
        QObject::connect(&manager, &AccountManager::userAuthenticated, [&](int userId, const QString &username, const QString &role) {
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

//creates temporary tables that simulate the PostgreSQL schema
bool createTestTables(QSqlDatabase &database, TestRunner &testRunner)
{
    QSqlQuery query(database);
    const bool foreignKeysEnabled = testRunner.verify(query.exec("PRAGMA foreign_keys = ON"), "Enable foreign key checks");
    const bool usersCreated = testRunner.verify(query.exec("CREATE TABLE IF NOT EXISTS users (user_id INTEGER PRIMARY KEY AUTOINCREMENT, username VARCHAR(50) NOT NULL UNIQUE, password_hash TEXT NOT NULL, role VARCHAR(10) NOT NULL DEFAULT 'Student' CHECK (role IN ('Student', 'Admin')), created_at DATETIME NOT NULL DEFAULT CURRENT_TIMESTAMP)"), "Create temporary users table");
    const bool notesCreated = testRunner.verify(query.exec("CREATE TABLE notes (note_id INTEGER PRIMARY KEY AUTOINCREMENT, user_id INTEGER NOT NULL REFERENCES users(user_id), title VARCHAR(200) NOT NULL, content TEXT NOT NULL, category VARCHAR(100), visibility VARCHAR(10) NOT NULL CHECK (visibility IN ('Public', 'Private')))"), "Create temporary notes table");
    return foreignKeysEnabled && usersCreated && notesCreated;
}

//returns the number of users stored in the test database
int countUsers(QSqlDatabase &database)
{
    QSqlQuery query(database);
    if (!query.exec("SELECT COUNT(*) FROM users") || !query.next()) {
        return -1;
    }
    return query.value(0).toInt();
}

void runInvalidRegistrationTests(AccountManager &manager, SignalTracker &tracker, QSqlDatabase &db, TestRunner &testRunner)
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
        {"Reject username longer than 15 characters", QString(16, 'A'), "ValidPass123", "Username must be 15 characters or less."},
        {"Reject empty password", "validuser", "", "Password cannot be empty."},
        {"Reject password shorter than 8 characters", "validuser", "short7", "Password must be at least 8 characters long."}
    };
    for (const BadRegistrationCase &testCase : cases) {
        tracker.reset();
        manager.handleRegistrationRequest(testCase.username, testCase.password);
        testRunner.verify(!tracker.regSuccess && tracker.regFailed && tracker.lastRegError == testCase.expectedError && countUsers(db) == 0, QString::fromUtf8(testCase.description));
    }
}

void runInvalidLoginTests(AccountManager &manager, SignalTracker &tracker, TestRunner &testRunner)
{
    SessionManager &session = SessionManager::instance();
    //login with empty credentials
    tracker.reset();
    manager.handleLoginRequest("", "");
    testRunner.verify(!tracker.loginSuccess && tracker.loginFailed && !tracker.userAuthEmitted && !session.isLoggedIn() && tracker.lastLoginError == "Please enter your username and password.", "Reject login with empty credentials");

    //login with a nonexistent username
    tracker.reset();
    manager.handleLoginRequest("ghost_student", "ValidPass123");
    testRunner.verify(!tracker.loginSuccess && tracker.loginFailed && !tracker.userAuthEmitted && !session.isLoggedIn() && tracker.lastLoginError == "Incorrect username or password.", "Reject login for nonexistent username");

    //login with an incorrect password
    tracker.reset();
    manager.handleLoginRequest("tawusi_melek", "WrongPassword!");
    testRunner.verify(!tracker.loginSuccess && tracker.loginFailed && !tracker.userAuthEmitted && !session.isLoggedIn() && tracker.lastLoginError == "Incorrect username or password.", "Reject login with incorrect password");
}

void runSessionAndAuthorizationTests(AccountManager &manager, SignalTracker &tracker, QSqlDatabase &db, TestRunner &testRunner)
{
    SessionManager &session = SessionManager::instance();
    //verify student session information after login
    tracker.reset();
    manager.handleLoginRequest("tawusi_melek", "Pc0ck_Ange1");
    const int studentId = session.userId();
    if (!testRunner.verify(tracker.loginSuccess && session.isLoggedIn() && studentId > 0 && session.role() == "Student" && !session.isAdmin(), "SessionManager stores Student session information")) {
        session.clearSession();
        return;
    }

    //verify that students cannot access admin services
    testRunner.verify(!AdminService::isAuthorizedAdmin(db, studentId), "Deny Admin authorization check for logged-in Student");
    const AdminNotesResult studentLoadAttempt = AdminService::loadPublicNotes(db, studentId);
    testRunner.verify(!studentLoadAttempt.success && studentLoadAttempt.message == "Access denied. Admin privileges are required.", "Block Student from loading Admin moderation notes");

    //generate a valid password hash for the test admin account
    char passwordHash[crypto_pwhash_STRBYTES];
    const QByteArray adminPassword = "AdminPass123";
    if (!testRunner.verify(crypto_pwhash_str(passwordHash, adminPassword.constData(), adminPassword.size(), crypto_pwhash_OPSLIMIT_INTERACTIVE, crypto_pwhash_MEMLIMIT_INTERACTIVE) == 0, "Generate test Admin password hash")) {
        session.clearSession();
        return;
    }

    //create an admin account in the test database
    QSqlQuery setupQuery(db);
    setupQuery.prepare("INSERT INTO users (username, password_hash, role) VALUES (:username, :password, 'Admin')");
    setupQuery.bindValue(":username", "test_admin");
    setupQuery.bindValue(":password", QString::fromUtf8(passwordHash));
    if (!testRunner.verify(setupQuery.exec(), "Create test Admin account")) {
        session.clearSession();
        return;
    }
    const int adminId = setupQuery.lastInsertId().toInt();
    if (!testRunner.verify(adminId > 0, "Retrieve test Admin ID")) {
        session.clearSession();
        return;
    }

    //create a public note owned by the student
    setupQuery.prepare("INSERT INTO notes (user_id, title, content, category, visibility) VALUES (:uid, 'Public Study Guide', 'Shared notes', 'Computer Science', 'Public')");
    setupQuery.bindValue(":uid", studentId);
    if (!testRunner.verify(setupQuery.exec(), "Create public test note")) {
        session.clearSession();
        return;
    }

    //create a private note owned by the student
    setupQuery.prepare("INSERT INTO notes (user_id, title, content, category, visibility) VALUES (:uid, 'Private Study Guide', 'Private notes', 'Computer Science', 'Private')");
    setupQuery.bindValue(":uid", studentId);
    if (!testRunner.verify(setupQuery.exec(), "Create private test note")) {
        session.clearSession();
        return;
    }

    //verify that students cannot impersonate an admin
    testRunner.verify(!AdminService::isAuthorizedAdmin(db, adminId), "Block Student session from passing Admin userId");

    //verify that logout clears the student session
    tracker.reset();
    manager.handleLogoutRequest();
    testRunner.verify(tracker.logoutSuccess && !session.isLoggedIn() && session.userId() == -1 && session.role().isEmpty() && !session.isAdmin(), "SessionManager resets userId, role, and flags on logout");

    //verify that admin services cannot be accessed after logout
    testRunner.verify(!AdminService::isAuthorizedAdmin(db, adminId), "Deny Admin authorization without an active session");
    const AdminNotesResult loggedOutAttempt = AdminService::loadPublicNotes(db, adminId);
    testRunner.verify(!loggedOutAttempt.success, "Block access to Admin moderation notes after logout");

    //authenticate the admin through AccountManager
    tracker.reset();
    manager.handleLoginRequest("test_admin", "AdminPass123");
    testRunner.verify(tracker.loginSuccess && !tracker.loginFailed && tracker.userAuthEmitted && tracker.authUserId == adminId && tracker.authUsername == "test_admin" && tracker.authRole == "Admin" && session.isLoggedIn() && session.userId() == adminId && session.isAdmin(), "Authenticate Admin and establish Admin session");

    //verify admin authorization
    testRunner.verify(AdminService::isAuthorizedAdmin(db, adminId), "Authorize valid Admin when session and database role match");

    //verify that admin moderation only retrieves public notes
    const AdminNotesResult adminLoadAttempt = AdminService::loadPublicNotes(db, adminId);

    testRunner.verify(adminLoadAttempt.success && adminLoadAttempt.notes.size() == 1 && adminLoadAttempt.notes.first().title == "Public Study Guide", "Allow Admin to load public notes while excluding private notes");

    //verify that logout clears the admin session
    tracker.reset();
    manager.handleLogoutRequest();
    testRunner.verify(tracker.logoutSuccess && !session.isLoggedIn() && !session.isAdmin() && session.userId() == -1 && session.role().isEmpty(), "Log out Admin and clear session");
    session.clearSession();
}

}

int main(int argc, char *argv[])
{
    QApplication application(argc, argv);
    TestRunner testRunner;

    //initialize libsodium for password hashing
    if (!testRunner.verify(sodium_init() >= 0, "Initialize libsodium")) {
        return 1;
    }

    //create a temporary directory for the test database
    QTemporaryDir tempDirectory;
    if (!testRunner.verify(tempDirectory.isValid(), "Create temporary test folder")) {
        return 1;
    }
    constexpr const char *connectionName = "studynotes_account_smoke";
    {
        QSqlDatabase testDatabase = QSqlDatabase::addDatabase("QSQLITE", connectionName);
        testDatabase.setDatabaseName(tempDirectory.filePath("account_test.sqlite"));
        if (!testRunner.verify(testDatabase.open(), "Open temporary test database")) {
            qCritical().noquote() << testDatabase.lastError().text();
        } else if (createTestTables(testDatabase, testRunner)) {
            SignalTracker tracker;
            AccountManager accountManager(connectionName);
            tracker.attach(accountManager);
            SessionManager::instance().clearSession();
            //1. invalid registration attempts
            runInvalidRegistrationTests(accountManager, tracker, testDatabase, testRunner);

            //2. valid registration with username trimming
            tracker.reset();
            accountManager.handleRegistrationRequest("  tawusi_melek  ", "Pc0ck_Ange1");
            testRunner.verify(tracker.regSuccess && !tracker.regFailed && countUsers(testDatabase) == 1, "Register valid new user and store in database");

            //3. duplicate username rejection
            tracker.reset();
            accountManager.handleRegistrationRequest("tawusi_melek", "Yazidism101");
            testRunner.verify(!tracker.regSuccess && tracker.regFailed && tracker.lastRegError == "This username is already taken." && countUsers(testDatabase) == 1, "Reject duplicate username registration");

            //4. invalid login attempts
            runInvalidLoginTests(accountManager, tracker, testRunner);

            //5. valid login and session creation
            tracker.reset();
            accountManager.handleLoginRequest("  tawusi_melek ", "Pc0ck_Ange1");
            testRunner.verify(tracker.loginSuccess && !tracker.loginFailed && tracker.userAuthEmitted && tracker.authUserId > 0 && tracker.authUsername == "tawusi_melek" && tracker.authRole == "Student" && SessionManager::instance().isLoggedIn(), "Authenticate valid user, emit user details, and start session");

            //6. logout from an active session
            tracker.reset();
            accountManager.handleLogoutRequest();
            testRunner.verify(tracker.logoutSuccess && !SessionManager::instance().isLoggedIn(), "Log out active user and clear session");

            //7. duplicate logout without an active session
            tracker.reset();
            accountManager.handleLogoutRequest();
            testRunner.verify(!tracker.logoutSuccess && !SessionManager::instance().isLoggedIn(), "Ignore duplicate logout request when no active session exists");

            //8. session management and authorization tests
            runSessionAndAuthorizationTests(accountManager, tracker, testDatabase, testRunner);
        }

        testDatabase.close();
    }

    //remove the temporary database connection
    QSqlDatabase::removeDatabase(connectionName);

    if (testRunner.failureCount == 0) {
        qInfo().noquote() << "ALL ACCOUNT MANAGER SMOKE TESTS PASSED";
        return 0;
    }
    qCritical().noquote() << "TESTS FAILED:" << testRunner.failureCount;
    return 1;
}
