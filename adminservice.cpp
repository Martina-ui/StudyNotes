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
