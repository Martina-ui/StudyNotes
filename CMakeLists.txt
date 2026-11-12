cmake_minimum_required(VERSION 3.16)

project(StudyNotesUI VERSION 0.1 LANGUAGES CXX)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Widgets Sql)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Widgets Sql)

set(PROJECT_SOURCES
    main.cpp

    databasemanager.cpp
    databasemanager.h

    mainwindow.cpp
    mainwindow.h
    mainwindow.ui

    registerpage.cpp
    registerpage.h
    registerpage.ui

    loginpage.cpp
    loginpage.h
    loginpage.ui

    profilepage.cpp
    profilepage.h
    profilepage.ui

    account_manager.cpp
    account_manager.h

    sessionmanager.h

    noteservice.cpp
    noteservice.h

    noteentrypage.cpp
    noteentrypage.h
    noteentrypage.ui

    mynotespage.cpp
    mynotespage.h
    mynotespage.ui

    publicnotespage.cpp
    publicnotespage.h
    publicnotespage.ui
)

if(${QT_VERSION_MAJOR} GREATER_EQUAL 6)
    qt_add_executable(StudyNotesUI
        MANUAL_FINALIZATION
        ${PROJECT_SOURCES}
        adminservice.cpp
        adminservice.h
        adminpanelpage.h adminpanelpage.cpp adminpanelpage.ui #I moved all the newly created files into the project sources section
    )
else()
    if(ANDROID)
        add_library(StudyNotesUI SHARED
            ${PROJECT_SOURCES}
        )
    else()
        add_executable(StudyNotesUI
            ${PROJECT_SOURCES}
        )
    endif()
endif()

#libsodium configuration for Windows and macOS
if(WIN32)

    set(SODIUM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/third_party/libsodium")

    if(NOT EXISTS "${SODIUM_ROOT}/include/sodium.h")
        message(FATAL_ERROR "libsodium header not found in third_party/libsodium/include")
    endif()

    if(NOT EXISTS "${SODIUM_ROOT}/lib/libsodium.dll.a")
        message(FATAL_ERROR "libsodium import library not found in third_party/libsodium/lib")
    endif()

    if(NOT EXISTS "${SODIUM_ROOT}/bin/libsodium-26.dll")
        message(FATAL_ERROR "libsodium DLL not found in third_party/libsodium/bin")
    endif()

    add_library(sodium SHARED IMPORTED)

    set_target_properties(sodium PROPERTIES
        IMPORTED_IMPLIB "${SODIUM_ROOT}/lib/libsodium.dll.a"
        IMPORTED_LOCATION "${SODIUM_ROOT}/bin/libsodium-26.dll"
        INTERFACE_INCLUDE_DIRECTORIES "${SODIUM_ROOT}/include"
    )

elseif(APPLE)

    find_package(PkgConfig REQUIRED)

    pkg_check_modules(SODIUM REQUIRED IMPORTED_TARGET libsodium)

    add_library(sodium ALIAS PkgConfig::SODIUM)

else()

    message(FATAL_ERROR "StudyNotes currently supports Windows and macOS.")

endif()

target_link_libraries(StudyNotesUI PRIVATE
    Qt${QT_VERSION_MAJOR}::Widgets
    Qt${QT_VERSION_MAJOR}::Sql
    sodium
)

#copy the libsodium DLL next to the executable on Windows
if(WIN32)
    add_custom_command(TARGET StudyNotesUI POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SODIUM_ROOT}/bin/libsodium-26.dll"
            "$<TARGET_FILE_DIR:StudyNotesUI>"
    )
endif()

#application properties
if(${QT_VERSION} VERSION_LESS 6.1.0)
    set(BUNDLE_ID_OPTION MACOSX_BUNDLE_GUI_IDENTIFIER com.example.StudyNotesUI)
endif()

set_target_properties(StudyNotesUI PROPERTIES
    ${BUNDLE_ID_OPTION}
    MACOSX_BUNDLE_BUNDLE_VERSION ${PROJECT_VERSION}
    MACOSX_BUNDLE_SHORT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}
    MACOSX_BUNDLE TRUE
    WIN32_EXECUTABLE TRUE
)

include(GNUInstallDirs)

install(TARGETS StudyNotesUI
    BUNDLE DESTINATION .
    LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
    RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}
)

if(QT_VERSION_MAJOR EQUAL 6)
    qt_finalize_executable(StudyNotesUI)
endif()
