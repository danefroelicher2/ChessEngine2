// test_tactical_diagnosis.cpp - Systematic diagnosis of tactical failure
// Testing why engine played Ke2 instead of Rxh4 when queen was hanging

#include "board.h"
#include "moves.h"
#include "engine.h"
#include <iostream>
#include <chrono>
#include <vector>
#include <string>

// =============================================================================
// DIAGNOSTIC TEST 1: DEPTH TEST
// =============================================================================
void testDepth() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "DIAGNOSTIC TEST 1: DEPTH TEST" << std::endl;
    std::cout << "========================================" << std::endl;
    std::cout << "Position: Black queen on h4 (hanging to Rh1)" << std::endl;
    std::cout << "Expected move: h1h4 (Rxh4 - captures queen)" << std::endl;
    std::cout << "\n";

    // Test 1: 1 second
    {
        std::cout << "--- Test with 1 second time limit ---" << std::endl;
        Board board;
        board.loadFromFEN("rnb1kbnr/pppppppp/8/8/7q/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        Moves moves(&board);
        Engine engine(&board, &moves);

        auto start = std::chrono::steady_clock::now();
        std::string move = engine.getBestMove(1000);
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "Best move: " << move << std::endl;
        std::cout << "Time taken: " << duration << "ms" << std::endl;
        std::cout << "Result: " << (move == "h1h4" ? "PASS ✓" : "FAIL ✗") << std::endl;
        std::cout << "\n";
    }

    // Test 2: 5 seconds
    {
        std::cout << "--- Test with 5 second time limit ---" << std::endl;
        Board board;
        board.loadFromFEN("rnb1kbnr/pppppppp/8/8/7q/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        Moves moves(&board);
        Engine engine(&board, &moves);

        auto start = std::chrono::steady_clock::now();
        std::string move = engine.getBestMove(5000);
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "Best move: " << move << std::endl;
        std::cout << "Time taken: " << duration << "ms" << std::endl;
        std::cout << "Result: " << (move == "h1h4" ? "PASS ✓" : "FAIL ✗") << std::endl;
        std::cout << "\n";
    }

    // Test 3: 10 seconds
    {
        std::cout << "--- Test with 10 second time limit ---" << std::endl;
        Board board;
        board.loadFromFEN("rnb1kbnr/pppppppp/8/8/7q/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        Moves moves(&board);
        Engine engine(&board, &moves);

        auto start = std::chrono::steady_clock::now();
        std::string move = engine.getBestMove(10000);
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "Best move: " << move << std::endl;
        std::cout << "Time taken: " << duration << "ms" << std::endl;
        std::cout << "Result: " << (move == "h1h4" ? "PASS ✓" : "FAIL ✗") << std::endl;
        std::cout << "\n";
    }

    std::cout << "Analysis: Does increasing time help find Rxh4?" << std::endl;
    std::cout << "  - If YES: Depth problem (not searching deep enough)" << std::endl;
    std::cout << "  - If NO: Evaluation or search logic problem" << std::endl;
}

// =============================================================================
// DIAGNOSTIC TEST 2: EVALUATION TEST
// =============================================================================
void testEvaluation() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "DIAGNOSTIC TEST 2: EVALUATION TEST" << std::endl;
    std::cout << "========================================" << std::endl;

    // Position 1: Before capture (queen on h4)
    Board board1;
    board1.loadFromFEN("rnb1kbnr/pppppppp/8/8/7q/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Moves moves1(&board1);
    Engine engine1(&board1, &moves1);
    int eval1 = engine1.evaluate();

    std::cout << "Position 1 (queen on h4): " << eval1 << " centipawns" << std::endl;
    std::cout << board1.toString() << std::endl;

    // Position 2: After Rxh4 (captured queen)
    Board board2;
    board2.loadFromFEN("rnb1kbnr/pppppppp/8/8/7R/8/PPPPPPPP/RNBQKBN1 b Qkq - 0 1");
    Moves moves2(&board2);
    Engine engine2(&board2, &moves2);
    int eval2 = engine2.evaluate();

    std::cout << "\nPosition 2 (after Rxh4): " << eval2 << " centipawns" << std::endl;
    std::cout << board2.toString() << std::endl;

    int difference = eval2 - eval1;
    std::cout << "\nEvaluation difference: " << difference << " centipawns" << std::endl;
    std::cout << "Expected: ~+900 (queen value)" << std::endl;

    if (difference >= 700) {
        std::cout << "Result: PASS ✓ (evaluation correctly values capturing queen)" << std::endl;
    } else {
        std::cout << "Result: FAIL ✗ (evaluation doesn't value queen capture enough!)" << std::endl;
        std::cout << "ERROR: Material evaluation is broken!" << std::endl;
    }
}

// =============================================================================
// DIAGNOSTIC TEST 3: MOVE GENERATION TEST
// =============================================================================
void testMoveGeneration() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "DIAGNOSTIC TEST 3: MOVE GENERATION TEST" << std::endl;
    std::cout << "========================================" << std::endl;

    Board board;
    board.loadFromFEN("rnb1kbnr/pppppppp/8/8/7q/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Moves moves(&board);

    std::vector<std::string> legalMoves = moves.generateLegalMoves();
    std::cout << "Total legal moves: " << legalMoves.size() << std::endl;
    std::cout << "\n";

    // Check if Rxh4 exists
    bool foundRxh4 = false;
    std::cout << "Looking for h1h4 (Rxh4)..." << std::endl;

    for (const auto& move : legalMoves) {
        if (move == "h1h4") {
            foundRxh4 = true;
            std::cout << "Result: PASS ✓ Found h1h4 (Rxh4) in legal moves" << std::endl;
            break;
        }
    }

    if (!foundRxh4) {
        std::cout << "Result: FAIL ✗ h1h4 NOT in legal moves!" << std::endl;
        std::cout << "\nERROR: Move generation is broken!" << std::endl;
        std::cout << "\nAll moves starting with h1:" << std::endl;
        for (const auto& move : legalMoves) {
            if (move.substr(0, 2) == "h1") {
                std::cout << "  " << move << std::endl;
            }
        }
    }

    // Show all legal moves for debugging
    std::cout << "\nAll legal moves:" << std::endl;
    for (size_t i = 0; i < legalMoves.size(); ++i) {
        std::cout << legalMoves[i];
        if ((i + 1) % 10 == 0) {
            std::cout << std::endl;
        } else {
            std::cout << " ";
        }
    }
    std::cout << std::endl;
}

// =============================================================================
// DIAGNOSTIC TEST 4: SEARCH STATISTICS
// =============================================================================
void testSearchStatistics() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "DIAGNOSTIC TEST 4: SEARCH STATISTICS" << std::endl;
    std::cout << "========================================" << std::endl;

    Board board;
    board.loadFromFEN("rnb1kbnr/pppppppp/8/8/7q/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Moves moves(&board);
    Engine engine(&board, &moves);

    std::cout << "Running 5-second search..." << std::endl;
    std::cout << "(Check console output for depth/nodes/statistics)" << std::endl;
    std::cout << "\n";

    auto start = std::chrono::steady_clock::now();
    std::string move = engine.getBestMove(5000);
    auto end = std::chrono::steady_clock::now();
    auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

    std::cout << "\nSearch completed:" << std::endl;
    std::cout << "Best move: " << move << std::endl;
    std::cout << "Time: " << duration << "ms" << std::endl;
    std::cout << "\nAnalysis from output above:" << std::endl;
    std::cout << "  - Max depth reached: [check output]" << std::endl;
    std::cout << "  - Total nodes searched: [check output]" << std::endl;
    std::cout << "  - If depth < 5: Not searching deep enough" << std::endl;
    std::cout << "  - If depth >= 5 but wrong move: Evaluation/search bug" << std::endl;
}

// =============================================================================
// MAIN
// =============================================================================
int main() {
    std::cout << "╔════════════════════════════════════════════════════════════════╗" << std::endl;
    std::cout << "║   TACTICAL FAILURE DIAGNOSTIC SUITE                           ║" << std::endl;
    std::cout << "║   Problem: Engine played Ke2 instead of Rxh4 (hangs queen)    ║" << std::endl;
    std::cout << "╚════════════════════════════════════════════════════════════════╝" << std::endl;

    try {
        // Run all 4 diagnostic tests
        testMoveGeneration();   // First: ensure move exists
        testEvaluation();        // Second: ensure eval is correct
        testDepth();            // Third: test different time limits
        testSearchStatistics(); // Fourth: analyze search depth/nodes

        // Summary
        std::cout << "\n========================================" << std::endl;
        std::cout << "DIAGNOSTIC SUMMARY" << std::endl;
        std::cout << "========================================" << std::endl;
        std::cout << "\nBased on the results above, the problem is likely:" << std::endl;
        std::cout << "\n1. If Rxh4 not in legal moves:" << std::endl;
        std::cout << "   → Move generation bug (CRITICAL)" << std::endl;
        std::cout << "\n2. If eval difference < 700:" << std::endl;
        std::cout << "   → Material evaluation bug" << std::endl;
        std::cout << "\n3. If more time helps find Rxh4:" << std::endl;
        std::cout << "   → Depth problem (need optimization)" << std::endl;
        std::cout << "\n4. If depth >= 5 but still wrong:" << std::endl;
        std::cout << "   → Search logic bug (alpha-beta, move ordering, TT)" << std::endl;
        std::cout << "\n========================================" << std::endl;

    } catch (const std::exception& e) {
        std::cerr << "ERROR: " << e.what() << std::endl;
        return 1;
    }

    return 0;
}
