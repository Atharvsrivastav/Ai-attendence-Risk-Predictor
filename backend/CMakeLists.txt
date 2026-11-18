cmake_minimum_required(VERSION 3.20)
project(attendance_backend C CXX)

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

# Common definitions
add_definitions(-DASIO_STANDALONE -D_WIN32_WINNT=0x0601)

set(CORE_SOURCES
    ${CMAKE_CURRENT_SOURCE_DIR}/src/database/Database.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/auth/AuthService.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/repositories/AttendanceRepository.cpp
    ${CMAKE_CURRENT_SOURCE_DIR}/src/services/AttendanceService.cpp
)

# Main Server Executable
add_executable(attendance_server
    ${CMAKE_CURRENT_SOURCE_DIR}/src/main.cpp
    ${CORE_SOURCES}
)

target_link_libraries(attendance_server PRIVATE
    sqlite3
    ws2_32
    mswsock
)

# Test Executable
if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_main.cpp)
    add_executable(attendance_tests
        ${CMAKE_CURRENT_SOURCE_DIR}/tests/test_main.cpp
        ${CORE_SOURCES}
    )
    target_link_libraries(attendance_tests PRIVATE
        sqlite3
        ws2_32
        mswsock
    )
endif()
