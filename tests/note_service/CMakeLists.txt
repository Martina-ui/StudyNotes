cmake_minimum_required(VERSION 3.16)
project(StudyNotesNoteServiceSmoke LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Independent testing castle. Not the team's actual application build!
# The app keeps its own CMakeLists.txt, undisturbed by these cheerful gremlins.
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Sql)

# Reuse the real service implementation from PR #31 rather than maintaining
# a second, suspiciously identical clone in the basement.
add_executable(NoteServiceSmoke
    smoketest.cpp
    ../../noteservice.cpp
    ../../noteservice.h
)

# The test belongs two floors below the public header; give it a staircase.
target_include_directories(NoteServiceSmoke PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(NoteServiceSmoke PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
)
