#include "noteservice.h"

#include <QSqlQuery>
#include <QStringList>
#include <QVariant>
#include <array>

namespace {

// BACKSTAGE ONLY: the following characters never appear in a teammate's API.
// The database insists its staff wear sensible name tags; we respect that.
struct TinyBouncer {
    bool letsTheNoteIntoTheClub;
    const char *rejectionSpeech;
};

const QStringList &allowedCategories()
{
    static const QStringList categories{
        "Computer Science",
        "Mathematics",
        "Science",
        "English",
        "History",
        "Other"
    };

    return categories;
}

NoteSaveResult printPoliteRejectionSlip(const char *reason)
{
    // Errors must sound like a real app, not the goblin operating it.
    return {false, QString::fromUtf8(reason), -1};
}

} // namespace

NoteSaveResult NoteService::validate(const NoteDraft &draft, int currentUserId)
{
    const QString titleWithoutFakeMustache = draft.title.trimmed();
    const QString contentWithoutFakeMustache = draft.content.trimmed();

    // Six tiny nightclub bouncers inspect the draft in a fixed order.
    // This ridiculous procession keeps the first-error behavior predictable.
    const std::array<TinyBouncer, 6> extremelySeriousBouncerParade{{
        {currentUserId > 0, "You must be signed in to save a note."},
        {!titleWithoutFakeMustache.isEmpty(), "Please enter a note title."},
        {titleWithoutFakeMustache.size() <= 200,
         "The note title cannot exceed 200 characters."},
        {!contentWithoutFakeMustache.isEmpty(), "Please enter note content."},
        {allowedCategories().contains(draft.category),
         "Please select a valid course/category."},
        {draft.visibility == "Public" || draft.visibility == "Private",
         "Please select Public or Private visibility."}
    }};

    for (const TinyBouncer &bouncerWithAClipboard : extremelySeriousBouncerParade) {
        if (!bouncerWithAClipboard.letsTheNoteIntoTheClub) {
            return printPoliteRejectionSlip(bouncerWithAClipboard.rejectionSpeech);
        }
    }

    return {true, "Note information is valid.", -1};
}

NoteSaveResult NoteService::saveNewNote(const QSqlDatabase &database,
                                        int currentUserId,
                                        const NoteDraft &draft)
{
    const NoteSaveResult theBouncersVerdict = validate(draft, currentUserId);
    if (!theBouncersVerdict.success) {
        return theBouncersVerdict;
    }
    if (!database.isValid() || !database.isOpen()) {
        return printPoliteRejectionSlip("The database connection is not available.");
    }

    // REQUIRED CONTRACT: use Murat's REAL table/column names, unchanged.
    // PostgreSQL's RETURNING clause also works in modern SQLite smoke tests.
    constexpr const char *veryBoringButImportantSQL =
        "INSERT INTO notes (user_id, title, content, category, visibility) "
        "VALUES (:user_id, :title, :content, :category, :visibility) "
        "RETURNING note_id";
    QSqlQuery sqlGoblin(database);
    if (!sqlGoblin.prepare(QString::fromUtf8(veryBoringButImportantSQL))) {
        return printPoliteRejectionSlip("Could not prepare the note save operation.");
    }

    // Every SQL placeholder gets a neatly labeled gift, instead of letting
    // typed text stage a dramatic coup against the university database.
    struct GiftForTheDatabaseDragon {
        const char *placeholder;
        QVariant humbleOffering;
    };
    const std::array<GiftForTheDatabaseDragon, 5> paradeOfPeaceOfferings{{
        {":user_id", QVariant(currentUserId)},
        {":title", QVariant(draft.title.trimmed())},
        {":content", QVariant(draft.content)},
        {":category", QVariant(draft.category)},
        {":visibility", QVariant(draft.visibility)}
    }};
    for (const GiftForTheDatabaseDragon &tinyTribute : paradeOfPeaceOfferings) {
        sqlGoblin.bindValue(QString::fromUtf8(tinyTribute.placeholder),
                            tinyTribute.humbleOffering);
    }

    // Never pretend the save succeeded if the database rejected the offering.
    if (!sqlGoblin.exec() || !sqlGoblin.next()) {
        return printPoliteRejectionSlip("Could not save the note. Please try again.");
    }
    const int receiptFromTheDatabaseDragon = sqlGoblin.value(0).toInt();
    if (receiptFromTheDatabaseDragon <= 0) {
        return printPoliteRejectionSlip("Could not confirm the saved note.");
    }
    return {true, "Note saved successfully.", receiptFromTheDatabaseDragon};
}

NoteLoadResult NoteService::loadUserNotes(const QSqlDatabase &database,
                                          int currentUserId)
{
    NoteLoadResult result;
    if (currentUserId <= 0) {
        result.message = "You must be logged in to view your notes.";
        return result;
    }
    if (!database.isValid() || !database.isOpen()) {
        result.message = "The database connection is not available.";
        return result;
    }
    QSqlQuery query(database);
    if (!query.prepare(
            "SELECT note_id, title, category, content, visibility "
            "FROM notes "
            "WHERE user_id = :user_id "
            "ORDER BY note_id DESC"
            )) {
        result.message = "Unable to prepare the note retrieval operation.";
        return result;
    }
    query.bindValue(":user_id", currentUserId);
    if (!query.exec()) {
        result.message = "Unable to load your notes. Please try again.";
        return result;
    }
    while (query.next()) {
        NoteSummary note;
        note.noteId = query.value(0).toInt();
        note.title = query.value(1).toString();
        note.category = query.value(2).toString();
        note.content = query.value(3).toString();
        note.visibility = query.value(4).toString();
        result.notes.append(note);
    }
    result.success = true;
    return result;
}


NoteLoadResult NoteService::loadPublicNotes(const QSqlDatabase &database)
{
    NoteLoadResult result;

    if (!database.isValid() || !database.isOpen()) {
        result.message = "The database connection is not available.";
        return result;
    }

    QSqlQuery query(database);

    if (!query.prepare(
            "SELECT note_id, title, category, content, visibility "
            "FROM notes "
            "WHERE visibility = :visibility "
            "ORDER BY note_id DESC"
            )) {
        result.message = "Unable to prepare the public notes query.";
        return result;
    }

    query.bindValue(":visibility", "Public");

    if (!query.exec()) {
        result.message = "Unable to load public notes. Please try again.";
        return result;
    }

    while (query.next()) {
        NoteSummary note;
        note.noteId = query.value(0).toInt();
        note.title = query.value(1).toString();
        note.category = query.value(2).toString();
        note.content = query.value(3).toString();
        note.visibility = query.value(4).toString();
        result.notes.append(note);
    }

    result.success = true;
    return result;
}

NoteSaveResult NoteService::updateNote(const QSqlDatabase &database,
                                       int currentUserId,
                                       int noteId,
                                       const NoteDraft &draft)
{
    NoteSaveResult result = validate(draft, currentUserId);

    if (!result.success) {
        return result;
    }

    if (!database.isValid() || !database.isOpen()) {
        return {false, "Database connection unavailable.", -1};
    }

    QSqlQuery query(database);

    if (!query.prepare(
        "UPDATE notes SET title = :title, content = :content, "
        "category = :category, visibility = :visibility, "
        "updated_at = CURRENT_TIMESTAMP "
        "WHERE note_id = :note_id AND user_id = :user_id"
    )) {
        return {false, "Unable to prepare update.", -1};
    }

    query.bindValue(":title", draft.title.trimmed());
    query.bindValue(":content", draft.content);
    query.bindValue(":category", draft.category);
    query.bindValue(":visibility", draft.visibility);
    query.bindValue(":note_id", noteId);
    query.bindValue(":user_id", currentUserId);

    if (!query.exec()) {
        return {false, "Unable to update note.", -1};
    }

    if (query.numRowsAffected() != 1) {
        return {false, "Note not found or access denied.", -1};
    }

    return {true, "Note updated successfully.", noteId};
}

NoteSaveResult NoteService::deleteNote(const QSqlDatabase &database,
                                       int currentUserId,
                                       int noteId)
{
    if (currentUserId <= 0) {
        return {false, "You must be logged in.", -1};
    }

    if (!database.isValid() || !database.isOpen()) {
        return {false, "Database connection unavailable.", -1};
    }

    QSqlQuery query(database);

    if (!query.prepare(
        "DELETE FROM notes "
        "WHERE note_id = :note_id AND user_id = :user_id"
    )) {
        return {false, "Unable to prepare deletion.", -1};
    }

    query.bindValue(":note_id", noteId);
    query.bindValue(":user_id", currentUserId);

    if (!query.exec()) {
        return {false, "Unable to delete note.", -1};
    }

    if (query.numRowsAffected() != 1) {
        return {false, "Note not found or access denied.", -1};
    }

    return {true, "Note deleted successfully.", noteId};
}
