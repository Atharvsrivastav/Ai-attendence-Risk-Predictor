cmake_minimum_required(VERSION 3.20)
project(attendance_fullstack C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Third party includes
include_directories(
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party
)

# Compile SQLite3 as C library
add_library(sqlite3 STATIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sqlite3.c)
target_include_directories(sqlite3 PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# Compile Cryptographic Core (Bcrypt, SHA256, and Monocypher Argon2id)
add_library(crypto_core STATIC 
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/crypt_blowfish.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/sha256.c
    ${CMAKE_CURRENT_SOURCE_DIR}/third_party/monocypher.c
)
target_include_directories(crypto_core PUBLIC ${CMAKE_CURRENT_SOURCE_DIR}/third_party)

# Common definitions
add_definitions(-DASIO_STANDALONE)
if(WIN32)
    add_definitions(-D_WIN32_WINNT=0x0601)
endif()

set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/database/Database.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/AuthService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/repositories/AttendanceRepository.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/AttendanceService.cpp
)

# Main Full-Stack C++ Web Server Executable
add_executable(attendance_server
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CORE_SOURCES}
)

if(WIN32)
    target_link_libraries(attendance_server PRIVATE
        sqlite3
        crypto_core
        ws2_32
        mswsock
        bcrypt
    )
else()
    target_link_libraries(attendance_server PRIVATE
        sqlite3
        crypto_core
        pthread
    )
endif()

# Automated Test Executable
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_main.cpp)
    add_executable(attendance_tests
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_main.cpp
        ${CORE_SOURCES}
    )
    if(WIN32)
        target_link_libraries(attendance_tests PRIVATE
            sqlite3
            crypto_core
            ws2_32
            mswsock
            bcrypt
        )
    else()
        target_link_libraries(attendance_tests PRIVATE
            sqlite3
            crypto_core
            pthread
        )
    endif()
endif()
