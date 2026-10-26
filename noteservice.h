#ifndef NOTESERVICE_H
#define NOTESERVICE_H

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

class NoteService {
public:
    static NoteSaveResult validate(const NoteDraft &draft, int currentUserId);
    static NoteSaveResult saveNewNote(const QSqlDatabase &database,
                                      int currentUserId,
                                      const NoteDraft &draft);
};

#endif // NOTESERVICE_H
