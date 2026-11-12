#ifndef ADMINSERVICE_H
#define ADMINSERVICE_H

#include <QList>
#include "mynotespage.h"
#include <QSqlDatabase>
#include <QString>

struct AdminNotesResult {
    bool success = false;
    QString message;
    QList<NoteSummary> notes;
};

class AdminService
{
public:
    static bool isAuthorizedAdmin(const QSqlDatabase &database, int currentUserId);
    static AdminNotesResult loadPublicNotes(const QSqlDatabase &database, int currentUserId);
};

#endif
