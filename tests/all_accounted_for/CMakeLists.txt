cmake_minimum_required(VERSION 3.16)
project(StudyNotesAccountTester LANGUAGES CXX)

set (CMAKE_AUTOMOC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# test build pulls heavily from Lewis's first test build, except remade for my subsystems.
# building an independant testing room. not as whimsical as Lewis's, sorry.
# the app keeps its own CMakeLists.txt
find_package(QT NAMES Qt6 Qt5 REQUIRED COMPONENTS Core Sql Widgets)
find_package(Qt${QT_VERSION_MAJOR} REQUIRED COMPONENTS Core Sql Widgets)

#gotta do all this for the hash
if(WIN32)

    set(SODIUM_ROOT "${CMAKE_CURRENT_SOURCE_DIR}/../../third_party/libsodium")

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

# Only building what I need.
add_executable(AccountTestFiles
    ../../account_manager.cpp
    ../../account_manager.h
    ../../databasemanager.cpp
    ../../databasemanager.h
    ../../sessionmanager.h
    ../../adminservice.cpp
    ../../adminservice.h
    account_test.cpp
)

# I don't know what all this setting up below the public header is for, but i will not touch it assuming it's necessary
target_include_directories(AccountTestFiles PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/../..
)

target_link_libraries(AccountTestFiles PRIVATE
    Qt${QT_VERSION_MAJOR}::Core
    Qt${QT_VERSION_MAJOR}::Sql
    Qt${QT_VERSION_MAJOR}::Widgets #only the notes file needs widgets, but its not my file, so im not changing anything in it
    sodium
)

# Copy libsodium-26.dll next to AccountTestFiles.exe on Windows so it runs without crashing -II
if(WIN32)
    add_custom_command(TARGET AccountTestFiles POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
            "${SODIUM_ROOT}/bin/libsodium-26.dll"
            "$<TARGET_FILE_DIR:AccountTestFiles>"
    )
endif()
