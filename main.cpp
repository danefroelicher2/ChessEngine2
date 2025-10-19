#include <iostream>
#include <chrono>
#include "board.h"
#include "moves.h"
#include "engine.h"

int main() {
    std::cout << "╔════════════════════════════════════════════════════════════════╗\n";
    std::cout << "║   TACTICAL FAILURE DIAGNOSTIC                                 ║\n";
    std::cout << "║   Problem: Engine played Ke2 instead of Rxh4 (hangs queen)    ║\n";
    std::cout << "╚════════════════════════════════════════════════════════════════╝\n\n";

    // Position: Black queen on h4, hanging to Rh1
    const char* testFEN = "rnb1kbnr/pppppppp/8/8/7q/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";

    // =============================================================================
    // TEST 1: MOVE GENERATION
    // =============================================================================
    std::cout << "========================================\n";
    std::cout << "TEST 1: MOVE GENERATION\n";
    std::cout << "========================================\n";
    {
        Board board;
        board.loadFromFEN(testFEN);
        std::cout << board.toString() << "\n";

        Moves moves(&board);
        std::vector<std::string> legalMoves = moves.generateLegalMoves();

        std::cout << "Total legal moves: " << legalMoves.size() << "\n";

        bool foundRxh4 = false;
        for (const auto& move : legalMoves) {
            if (move == "h1h4") {
                foundRxh4 = true;
                break;
            }
        }

        if (foundRxh4) {
            std::cout << "Result: ✓ PASS - h1h4 (Rxh4) found in legal moves\n\n";
        } else {
            std::cout << "Result: ✗ FAIL - h1h4 NOT in legal moves!\n";
            std::cout << "Moves from h1:\n";
            for (const auto& move : legalMoves) {
                if (move.substr(0, 2) == "h1") {
                    std::cout << "  " << move << "\n";
                }
            }
            std::cout << "\n";
        }
    }

    // =============================================================================
    // TEST 2: EVALUATION
    // =============================================================================
    std::cout << "========================================\n";
    std::cout << "TEST 2: EVALUATION\n";
    std::cout << "========================================\n";
    {
        // Before capture
        Board board1;
        board1.loadFromFEN(testFEN);
        Moves moves1(&board1);
        Engine engine1(&board1, &moves1);
        int eval1 = engine1.evaluate();

        std::cout << "Position BEFORE Rxh4: " << eval1 << " centipawns\n";

        // After capture
        Board board2;
        board2.loadFromFEN("rnb1kbnr/pppppppp/8/8/7R/8/PPPPPPPP/RNBQKBN1 b Qkq - 0 1");
        Moves moves2(&board2);
        Engine engine2(&board2, &moves2);
        int eval2 = engine2.evaluate();

        std::cout << "Position AFTER Rxh4:  " << eval2 << " centipawns\n";

        int diff = eval2 - eval1;
        std::cout << "Difference: " << diff << " (expected ~900)\n";

        if (diff >= 700) {
            std::cout << "Result: ✓ PASS - Evaluation correctly values queen capture\n\n";
        } else {
            std::cout << "Result: ✗ FAIL - Evaluation doesn't value queen capture!\n\n";
        }
    }

    // =============================================================================
    // TEST 3: DEPTH TEST (1 second)
    // =============================================================================
    std::cout << "========================================\n";
    std::cout << "TEST 3: DEPTH TEST (1 second)\n";
    std::cout << "========================================\n";
    {
        Board board;
        board.loadFromFEN(testFEN);
        Moves moves(&board);
        Engine engine(&board, &moves);

        auto start = std::chrono::steady_clock::now();
        std::string move = engine.getBestMove(1000);
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "\nBest move: " << move << "\n";
        std::cout << "Time: " << duration << "ms\n";
        std::cout << "Nodes: " << engine.getNodesSearched() << "\n";
        std::cout << "Result: " << (move == "h1h4" ? "✓ PASS" : "✗ FAIL") << "\n\n";
    }

    // =============================================================================
    // TEST 4: DEPTH TEST (5 seconds)
    // =============================================================================
    std::cout << "========================================\n";
    std::cout << "TEST 4: DEPTH TEST (5 seconds)\n";
    std::cout << "========================================\n";
    {
        Board board;
        board.loadFromFEN(testFEN);
        Moves moves(&board);
        Engine engine(&board, &moves);

        auto start = std::chrono::steady_clock::now();
        std::string move = engine.getBestMove(5000);
        auto end = std::chrono::steady_clock::now();
        auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start).count();

        std::cout << "\nBest move: " << move << "\n";
        std::cout << "Time: " << duration << "ms\n";
        std::cout << "Nodes: " << engine.getNodesSearched() << "\n";
        std::cout << "Result: " << (move == "h1h4" ? "✓ PASS" : "✗ FAIL") << "\n\n";
    }

    // =============================================================================
    // SUMMARY
    // =============================================================================
    std::cout << "========================================\n";
    std::cout << "DIAGNOSTIC SUMMARY\n";
    std::cout << "========================================\n";
    std::cout << "Look at the depth reached in the output above.\n";
    std::cout << "\nPossible diagnoses:\n";
    std::cout << "1. If Rxh4 not in legal moves → Move generation bug\n";
    std::cout << "2. If eval diff < 700 → Material evaluation bug\n";
    std::cout << "3. If more time helps → Depth/performance problem\n";
    std::cout << "4. If depth >= 5 but wrong → Search logic bug\n";

    return 0;
}
