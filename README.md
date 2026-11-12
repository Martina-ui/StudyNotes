# StudyNotes
StudyNotes is a desktop note-taking application designed for college students to organize notes by class or subject and share public notes with other students. Users can register, sign in, create notes, browse their own notes, and discover notes shared publicly. Administrators can review public notes in a dedicated moderation panel.

## Team — StudyStars
| Member | Role |
| --- | --- |
| Martina Delger | Project Manager & Developer |
| Jason Baterna | Scrum Master |
| Lewis Wilson | Developer |
| Murat Atar | Developer |

## Technology stack
- **C++17** — application logic
- **Qt Widgets** — desktop user interface (`.ui` forms)
- **Qt SQL** with the **QPSQL** driver — PostgreSQL connectivity
- **PostgreSQL** — persistent users and notes
- **libsodium** — password hashing and verification
- **CMake 3.16+** — build configuration
- **Qt Creator** — recommended development IDE

## Prerequisites
1. **Windows or macOS.** The current CMake configuration explicitly rejects other platforms.
2. **Qt 6** with the Widgets and SQL components, plus a compatible C++ compiler and Qt Creator. CMake also discovers Qt 5, but the current source list for the Qt 5 target omits the Admin Panel files; Qt 6 is the recommended configuration until that is corrected.
3. **CMake 3.16 or newer** (Qt's installed version may require a newer CMake release).
4. **PostgreSQL**, running locally or reachable over the network, and the PostgreSQL **QPSQL Qt SQL driver** installed for your Qt kit.
5. **libsodium**:
   - **Windows:** The project expects the following files relative to the repository root: `third_party/libsodium/include/sodium.h`, `third_party/libsodium/lib/libsodium.dll.a`, and `third_party/libsodium/bin/libsodium-26.dll`. The import library must be compatible with the compiler selected in Qt Creator (the `.dll.a` naming convention typically indicates a MinGW toolchain). CMake copies the DLL next to the executable after a successful build.
   - **macOS:** Install libsodium and `pkg-config` so that `pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)` can find it.

> Do not commit database passwords, `.env` files containing secrets, or local build output.

## Clone the repository
```bash
git clone https://github.com/Martina-ui/StudyNotes.git
cd StudyNotes
```

## PostgreSQL setup
1. Start PostgreSQL and create a database named `studynotes_db`, or choose a different name and set `STUDYNOTES_DB_NAME` accordingly.
2. Create or select a PostgreSQL login that has permission to connect to the database and work with its tables.
3. Run the project's `schema.sql` against that database to create the `users` and `notes` tables. For example, from a terminal with `psql` available:

   ```bash
   psql -h localhost -U postgres -d studynotes_db -f schema.sql
   ```

   Alternatively, open `schema.sql` in pgAdmin's Query Tool for the selected database and execute it.

The schema creates users with the default role `Student` and notes with the default visibility `Private`. Supported roles are `Student` and `Admin`; supported note visibility values are `Private` and `Public`.

### Database configuration
`DatabaseManager` reads these environment variables when the application starts:

| Variable | Default | Purpose |
| --- | --- | --- |
| `STUDYNOTES_DB_HOST` | `localhost` | PostgreSQL hostname |
| `STUDYNOTES_DB_PORT` | `5432` | PostgreSQL port |
| `STUDYNOTES_DB_NAME` | `studynotes_db` | Database name |
| `STUDYNOTES_DB_USER` | `postgres` | Database username |
| `STUDYNOTES_DB_PASSWORD` | *(unset)* | Database password; set this for your local PostgreSQL account |

**Windows PowerShell example** (set these in the environment used to launch StudyNotes):

```powershell
$env:STUDYNOTES_DB_HOST = "localhost"
$env:STUDYNOTES_DB_PORT = "5432"
$env:STUDYNOTES_DB_NAME = "studynotes_db"
$env:STUDYNOTES_DB_USER = "postgres"
$env:STUDYNOTES_DB_PASSWORD = "your-local-password"
```

**macOS terminal example:**

```bash
export STUDYNOTES_DB_HOST=localhost
export STUDYNOTES_DB_PORT=5432
export STUDYNOTES_DB_NAME=studynotes_db
export STUDYNOTES_DB_USER=postgres
export STUDYNOTES_DB_PASSWORD='your-local-password'
```

Environment variables must be available to the **running application**. If you launch it through Qt Creator, configure them under **Projects → Run → Environment**, or launch Qt Creator from a shell that has those variables set.

## Build and run with Qt Creator
1. Open Qt Creator and choose **File → Open File or Project**.
2. Select the repository's `CMakeLists.txt`.
3. Select a **Qt 6 desktop kit** with a compatible compiler; on Windows, make sure the kit matches the bundled libsodium import library.
4. Configure the CMake project. Resolve any missing Qt or libsodium dependencies reported during configuration.
5. Add the database environment variables under **Projects → Run → Environment**.
6. Build the project (**Ctrl+B** on Windows, or the Build command in Qt Creator).
7. Click **Run** to launch `StudyNotesUI`.

The app attempts to connect to PostgreSQL at startup and reports connection success or failure in the application output. A failed database connection will prevent database-backed features from working correctly.

### Optional command-line build
From a shell configured with a compatible Qt 6 compiler/toolchain and Qt installation:

```bash
cmake -S . -B build
cmake --build build
```

If CMake cannot locate Qt, supply the appropriate Qt installation prefix through `CMAKE_PREFIX_PATH`. Run the resulting `StudyNotesUI` executable from the build output directory; its exact location depends on the generator and configuration. On Windows, ensure the Qt runtime libraries and QPSQL driver are available when launching outside Qt Creator.

## Using StudyNotes
- **Register / Log in:** Create a student account or sign in with an existing account.
- **Create Note:** Add a title, content, category, and visibility setting.
- **My Notes:** Browse your notes.
- **Public Notes:** Browse notes marked public by users.
- **My Profile:** View account information.
- **Admin Panel:** Available only to authenticated Admin users. Admins can review public notes; the Delete Note control is disabled pending integration of the separate note-deletion issue.

## Development workflow
1. Pull the latest `main` branch before beginning new work.
2. Create a feature branch for the issue (for example, `docs/issue-18-readme-setup`).
3. Make a focused change, build with Qt Creator, and test the affected functionality.
4. Review your changes with `git diff` and `git status` before committing. Avoid adding secrets or generated build files.
5. Push your feature branch and open a pull request targeting `main`. Reference the relevant GitHub issue and describe how the change was tested.
6. Obtain review and merge through the repository's branch-protection workflow.

## Troubleshooting
| Problem | What to check |
| --- | --- |
| `QPSQL driver not loaded` | Qt SQL PostgreSQL driver is installed and available for the selected Qt kit; PostgreSQL client libraries are accessible. |
| Database connection failed | PostgreSQL is running, the host/port/database/user/password are correct, and the account has permission to connect. |
| CMake cannot find Qt | Select a configured Qt kit in Qt Creator or set `CMAKE_PREFIX_PATH` to the Qt installation. |
| libsodium header/import library/DLL not found on Windows | Verify the exact `third_party/libsodium` file paths expected by `CMakeLists.txt`. |
| libsodium not found on macOS | Install libsodium and `pkg-config`; ensure `pkg-config` can locate libsodium. |
| App launches from Qt Creator but not standalone | Ensure the Qt runtime, Qt SQL driver, and other required runtime libraries are deployed. |
