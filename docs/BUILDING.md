# Building Study Notes

You need Qt Creator, Qt 5 or Qt 6 (with Widgets), CMake, and a working Desktop kit/compiler.

1. Open Qt Creator.
2. Choose **File > Open File or Project**.
3. Select the repository-root `CMakeLists.txt`.
4. Choose a working **Desktop Qt** kit and click **Configure Project** if asked.
5. Click the green **Run** triangle.
6. Confirm a window titled **Study Notes** appears with the message **Study Notes — Application setup complete.**.

If the window appears, the basic project has escaped the build dungeon successfully.
If not, check Qt Creator's **Compile Output** and **Issues** panels for errors.

## Where files belong

- C++ and `.ui` source files currently live at the repository root, matching the other UI feature branches.
- Database schema stays in `database/` (already in the repository's `main` branch).
- Future automated tests belong in `tests/`.
- Generated build files and Qt Creator's local settings should not be committed; see `.gitignore`.

## Integrating teammates' UI pages later

The `MainWindow` class, its `ui` pointer, and `mainwindow.ui` object names are deliberately the same as those already used in the registration, login, and profile branches.

This starter's `main.cpp` temporarily opens `MainWindow`. When the team merges registration, login, and profile pages, **pick the intended first page** and update `main.cpp` accordingly. Do not keep multiple `main()` functions.

When a teammate's `registerpage`, `loginpage`, or `profilepage` files are merged, their `.cpp`, `.h`, and `.ui` files must be added to the executable's source list in `CMakeLists.txt` for the Qt version(s) being supported. These files are **not** included here because they are still owned by separate teammates' branches.

If GitHub flags an add/add conflict in `main.cpp` or `CMakeLists.txt`, ask the author of that UI branch to reconcile it. Don't click a button that blindly overwrites their feature work. A merge conflict is a discussion, not a boss fight you should speedrun.
