#include "noteservice.h"

#include <QCoreApplication>
#include <QDebug>
#include <QSqlDatabase>
#include <QSqlError>
#include <QSqlQuery>
#include <QTemporaryDir>
#include <QVariant>

namespace {

// A private test helper dressed as a pigeon with a badge.
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

NoteDraft aCompletelyRespectableScroll()
{
    return {"Final Exam Outline", "Chapters 1 through 4",
            "Computer Science", "Private"};
}

bool assembleTheTemporaryFurniture(QSqlDatabase &database, ClipboardPigeon &pigeon)
{
    // These are test-only tables, NOT a replacement for Murat's PostgreSQL.
    // Database dragons become grumpy when you try to invent new production tables.
    QSqlQuery screwdriverGoblin(database);
    const bool foreignKeysEnabled = pigeon.verify(
        screwdriverGoblin.exec("PRAGMA foreign_keys = ON"),
        "Enable foreign key checks");
    const bool usersCreated = pigeon.verify(
        screwdriverGoblin.exec("CREATE TABLE users (user_id INTEGER PRIMARY KEY)"),
        "Create temporary users table");
    const bool pretendStudentCreated = pigeon.verify(
        screwdriverGoblin.exec("INSERT INTO users (user_id) VALUES (7)"),
        "Create mock signed-in user");
    const bool notesCreated = pigeon.verify(
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
    return foreignKeysEnabled && usersCreated && pretendStudentCreated && notesCreated;
}

void paradeOfRejectedScrolls(ClipboardPigeon &pigeon)
{
    const NoteDraft extremelyNormalScroll = aCompletelyRespectableScroll();
    pigeon.verify(!NoteService::validate(extremelyNormalScroll, 0).success,
                  "Reject missing authenticated user ID");

    // Data-driven errors, each wearing a spectacularly questionable disguise.
    struct UnfortunateScroll {
        const char *testDescription;
        NoteDraft actualDraft;
    };
    NoteDraft titleWentToBuyMilk = extremelyNormalScroll;
    titleWentToBuyMilk.title = "   ";
    NoteDraft contentWasAbductedByDucks = extremelyNormalScroll;
    contentWasAbductedByDucks.content = "  ";
    NoteDraft titleDevouredAnEntireDictionary = extremelyNormalScroll;
    titleDevouredAnEntireDictionary.title = QString(201, 'X');
    NoteDraft suspiciousSnackCategory = extremelyNormalScroll;
    suspiciousSnackCategory.category = "Haunted Toaster Studies";
    NoteDraft invisibilityCloakOfNonsense = extremelyNormalScroll;
    invisibilityCloakOfNonsense.visibility = "Invisible-ish";

    const UnfortunateScroll marchOfQuestionableDecisions[] = {
        {"Reject blank title", titleWentToBuyMilk},
        {"Reject blank content", contentWasAbductedByDucks},
        {"Reject title longer than 200 characters", titleDevouredAnEntireDictionary},
        {"Reject unknown category", suspiciousSnackCategory},
        {"Reject unknown visibility", invisibilityCloakOfNonsense}
    };
    for (const UnfortunateScroll &tinyDisaster : marchOfQuestionableDecisions) {
        pigeon.verify(!NoteService::validate(tinyDisaster.actualDraft, 7).success,
                      QString::fromUtf8(tinyDisaster.testDescription));
    }
}

int countNotes(QSqlDatabase &database)
{
    QSqlQuery duckDetective(database);
    if (!duckDetective.exec("SELECT COUNT(*) FROM notes") || !duckDetective.next()) {
        return -1;
    }
    return duckDetective.value(0).toInt();
}

bool proveTheNoteSurvivedReopening(QSqlDatabase &database, int savedNoteId)
{
    QSqlQuery evidenceCollector(database);
    if (!evidenceCollector.prepare(
            "SELECT user_id, title, content, category, visibility "
            "FROM notes WHERE note_id = :note_id")) {
        return false;
    }
    evidenceCollector.bindValue(":note_id", savedNoteId);
    return evidenceCollector.exec() && evidenceCollector.next() &&
           evidenceCollector.value(0).toInt() == 7 &&
           evidenceCollector.value(1).toString() == "Final Exam Outline" &&
           evidenceCollector.value(2).toString() == "Chapters 1 through 4" &&
           evidenceCollector.value(3).toString() == "Computer Science" &&
           evidenceCollector.value(4).toString() == "Private";
}

} // namespace

int main(int argc, char *argv[])
{
    QCoreApplication application(argc, argv);
    ClipboardPigeon clipboardPigeon;
    QTemporaryDir secretGoblinLair;
    if (!clipboardPigeon.verify(secretGoblinLair.isValid(),
                                "Create temporary test folder")) {
        return 1;
    }

    constexpr const char *connectionName = "studynotes_issue22_smoke";
    {
        // The database is intentionally LOCAL AND TEMPORARY. No password needed.
        // Scope keeps the QSqlDatabase handle from haunting removeDatabase().
        QSqlDatabase pocketSizedDatabase =
            QSqlDatabase::addDatabase("QSQLITE", connectionName);
        pocketSizedDatabase.setDatabaseName(
            secretGoblinLair.filePath("study_notes_test.sqlite"));

        if (!clipboardPigeon.verify(pocketSizedDatabase.open(),
                                    "Open temporary test database")) {
            qCritical().noquote() << pocketSizedDatabase.lastError().text();
        } else if (assembleTheTemporaryFurniture(pocketSizedDatabase, clipboardPigeon)) {
            paradeOfRejectedScrolls(clipboardPigeon);
            const NoteDraft respectableScroll = aCompletelyRespectableScroll();
            const NoteSaveResult victoryReceipt =
                NoteService::saveNewNote(pocketSizedDatabase, 7, respectableScroll);
            clipboardPigeon.verify(victoryReceipt.success && victoryReceipt.noteId > 0,
                                    "Save a valid note and return its ID");
            clipboardPigeon.verify(countNotes(pocketSizedDatabase) == 1,
                                    "Confirm exactly one note was saved");

            NoteDraft blankTitleVillain = respectableScroll;
            blankTitleVillain.title = "  ";
            clipboardPigeon.verify(
                !NoteService::saveNewNote(pocketSizedDatabase, 7, blankTitleVillain).success &&
                countNotes(pocketSizedDatabase) == 1,
                "Do not insert an invalid note");

            clipboardPigeon.verify(
                !NoteService::saveNewNote(pocketSizedDatabase, 9999,
                                          respectableScroll).success &&
                countNotes(pocketSizedDatabase) == 1,
                "Reject a note for a nonexistent database user");

            pocketSizedDatabase.close();
            if (clipboardPigeon.verify(pocketSizedDatabase.open(),
                                        "Reopen the test database")) {
                clipboardPigeon.verify(
                    proveTheNoteSurvivedReopening(pocketSizedDatabase,
                                                  victoryReceipt.noteId),
                    "Retrieve saved note with correct owner and fields");
                pocketSizedDatabase.close();
            }
        }
        pocketSizedDatabase.close();
    }
    QSqlDatabase::removeDatabase(connectionName);

    if (clipboardPigeon.criminalBreadcrumbCount == 0) {
        qInfo().noquote() << "ALL NOTE SERVICE SMOKE TESTS PASSED";
        return 0;
    }
    qCritical().noquote() << "TESTS FAILED:" << clipboardPigeon.criminalBreadcrumbCount;
    return 1;
}
