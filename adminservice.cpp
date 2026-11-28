#include "adminservice.h"
#include "sessionmanager.h"

#include <QSqlQuery>

bool AdminService::isAuthorizedAdmin(const QSqlDatabase &database, int currentUserId)
{
    const SessionManager &session = SessionManager::instance();
    if (!session.isAdmin() || session.userId() != currentUserId) {
        return false;
    }
    if (!database.isValid() || !database.isOpen()) {
        return false;
    }
    QSqlQuery query(database);
    if (!query.prepare( "SELECT role FROM users WHERE user_id = :user_id")) {
        return false;
    }
    query.bindValue(":user_id", currentUserId);
    if (!query.exec() || !query.next()) {
        return false;
    }
    return query.value(0).toString() == "Admin";
}


AdminNotesResult AdminService::loadPublicNotes(const QSqlDatabase &database, int currentUserId)
{
    AdminNotesResult result;
    if (!isAuthorizedAdmin(database, currentUserId)) {
        result.message = "Access denied. Admin privileges are required.";
        return result;
    }
    QSqlQuery query(database);
    if (!query.prepare(
            "SELECT note_id, title, category, content, visibility "
            "FROM notes "
            "WHERE visibility = :visibility "
            "ORDER BY note_id DESC")) {
        result.message = "Unable to prepare the moderation query.";
        return result;
    }
    query.bindValue(":visibility", "Public");
    if (!query.exec()) {
        result.message = "Unable to load notes for moderation.";
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

NoteSaveResult AdminService::deletePublicNote(const QSqlDatabase &database,
                                             int currentUserId,
                                             int noteId)
{
    if (!isAuthorizedAdmin(database, currentUserId)) {
        return {false, "Access denied. Admin privileges are required.", -1};
    }

    // The SQL goblin checks both stamps at the trapdoor.
    // The note must still be Public and the user must still be an Admin.
    const QString paperworkForTheTrapdoor =
        "DELETE FROM notes "
        "WHERE note_id = :note_id AND visibility = 'Public' "
        "AND EXISTS (SELECT 1 FROM users "
        "WHERE user_id = :admin_id AND role = 'Admin')";
    QSqlQuery sqlGoblin(database);
    if (!sqlGoblin.prepare(paperworkForTheTrapdoor)) {
        return {false, "Unable to prepare note deletion.", -1};
    }
    sqlGoblin.bindValue(":note_id", noteId);
    sqlGoblin.bindValue(":admin_id", currentUserId);
    if (!sqlGoblin.exec()) {
        return {false, "Unable to delete the public note.", -1};
    }
    const int notesThroughTheTrapdoor = sqlGoblin.numRowsAffected();
    if (notesThroughTheTrapdoor != 1) {
        return {false, "Public note not found or access denied.", -1};
    }

    return {true, "Public note deleted successfully.", noteId};
}
