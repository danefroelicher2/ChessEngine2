cmake_minimum_required(VERSION 3.10)
project(ChessEngine)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Find PostgreSQL
find_package(PostgreSQL REQUIRED)

# ONNX Runtime configuration
set(ONNXRUNTIME_ROOT "${CMAKE_SOURCE_DIR}/external/onnxruntime-linux-x64-1.20.1")
include_directories(${ONNXRUNTIME_ROOT}/include)
link_directories(${ONNXRUNTIME_ROOT}/lib)

# Executable 1: Local testing version
add_executable(chess_engine
    main.cpp
    board.cpp
    moves.cpp
    engine.cpp
    neural_evaluator.cpp
)

# Executable 2: HTTP server version
add_executable(chess_server
    server.cpp
    board.cpp
    moves.cpp
    engine.cpp
    database.cpp
    neural_evaluator.cpp
)

# Diagnostic executables commented out - not needed for production
# These can be uncommented for local testing if needed

# # Executable 3: Tactical bug diagnostic tests
# add_executable(test_tactical_bug
#     test_tactical_bug.cpp
#     board.cpp
#     moves.cpp
#     engine.cpp
# )

# # Executable 4: Critical diagnostic tests
# add_executable(critical_diagnostic
#     critical_diagnostic.cpp
#     board.cpp
#     moves.cpp
#     engine.cpp
#     transposition_table.cpp
#     zobrist.cpp
# )

# # Executable 5: Simple evaluation diagnostic
# add_executable(simple_eval_test
#     simple_eval_test.cpp
#     board.cpp
#     moves.cpp
#     engine.cpp
#     transposition_table.cpp
#     zobrist.cpp
# )

# # Executable 6: SEE diagnostic
# add_executable(see_diagnostic
#     see_diagnostic.cpp
#     board.cpp
#     moves.cpp
#     engine.cpp
#     transposition_table.cpp
#     zobrist.cpp
# )

# Link ONNX Runtime for both executables
target_link_libraries(chess_engine onnxruntime)
target_link_libraries(chess_server onnxruntime)

# Link PostgreSQL
target_link_libraries(chess_server PostgreSQL::PostgreSQL)

# Link socket libraries on Windows
if(WIN32)
    target_link_libraries(chess_server ws2_32)
endif()
