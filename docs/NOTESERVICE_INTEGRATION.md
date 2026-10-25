# NoteService Integration Guide

    ## Purpose

    `NoteService` validates and saves new StudyNotes notes.

    Files: `noteservice.h` and `noteservice.cpp`.

    ## Required dependencies

    Before saving notes, the application must provide:

    - An open PostgreSQL `QSqlDatabase` connection from the database component (#16).
    - The actual authenticated user's ID from session handling (#21).
    - Title, content, category, and visibility from the note-entry UI (#7).

    ## Saving a note

    ```cpp
    NoteDraft draft;
    draft.title = title;
    draft.content = content;
    draft.category = category;
    draft.visibility = visibility;

    NoteSaveResult result =
        NoteService::saveNewNote(database, currentUserId, draft);

    if (result.success) {
        // The note was saved; result.noteId contains its ID.
    } else {
        // Display result.message to the user.
    }
    ```

    `database` must be an open connection to the development PostgreSQL database. `currentUserId` must come from the authenticated session, not a hard-coded test value. The other four values come from the note-entry form.

    ## Build requirements

    Add `noteservice.cpp` and `noteservice.h` to the application's CMake source list. Include the Qt SQL component and link `Qt::Sql` using the project's Qt-version variable.

    Preserve existing frontend source files and build entries when updating CMake.

    ## Verification still needed

    - Save a real note using PostgreSQL.
    - Confirm the note belongs to the authenticated user.
    - Confirm category and visibility are stored.
    - Reload the note using the team's retrieval functionality.
    - Confirm errors are displayed clearly.

    Standalone SQLite tests passed locally, but PostgreSQL and frontend integration remain pending.
