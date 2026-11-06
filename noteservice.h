#ifndef NOTESERVICE_H
#define NOTESERVICE_H
#include <QList>
#include "mynotespage.h"

#include <QSqlDatabase>
#include <QString>

// PUBLIC TEAM INTERFACE: these names must stay ordinary and stable.
// The comedic nonsense lives in noteservice.cpp, safely out of sight.
struct NoteDraft {
    QString title;
    QString content;
    QString category;
    QString visibility;
};

struct NoteSaveResult {
    bool success = false;
    QString message;
    int noteId = -1;
};

struct NoteLoadResult {
    bool success = false;
    QString message;
    QList<NoteSummary> notes;
};

class NoteService {
public:
    static NoteSaveResult validate(const NoteDraft &draft, int currentUserId);
    static NoteSaveResult saveNewNote(const QSqlDatabase &database,
                                      int currentUserId,
                                      const NoteDraft &draft);
    static NoteLoadResult loadUserNotes(const QSqlDatabase &database,
                                        int currentUserId);
};

#endif // NOTESERVICE_H
