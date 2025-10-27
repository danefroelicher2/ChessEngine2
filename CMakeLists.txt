cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find PostgreSQL
find_package(PostgreSQL REQUIRED)

# Executable 1: Local testing version
add_executable(chess_engine
    main.cpp
    board.cpp
    moves.cpp
    engine.cpp
)

# Executable 2: HTTP server version
add_executable(chess_server
    server.cpp
    board.cpp
    moves.cpp
    engine.cpp
    database.cpp
)

# Executable 3: Tactical bug diagnostic tests
add_executable(test_tactical_bug
    test_tactical_bug.cpp
    board.cpp
    moves.cpp
    engine.cpp
)

# Executable 4: Critical diagnostic tests
add_executable(critical_diagnostic
    critical_diagnostic.cpp
    board.cpp
    moves.cpp
    engine.cpp
)

# Executable 5: Board state diagnostic tool
add_executable(diagnostic_tool
    diagnostic_tool.cpp
    board.cpp
    moves.cpp
)

# Link PostgreSQL
target_link_libraries(chess_server PostgreSQL::PostgreSQL)

# Link socket libraries on Windows
if(WIN32)
    target_link_libraries(chess_server ws2_32)
endif()
