#include "board.h"
#include "moves.h"
#include "engine.h"
#include <iostream>
#include <string>

void testPosition(const std::string& description, const std::string& fen, int timeLimitMs) {
    std::cout << "\n========================================" << std::endl;
    std::cout << "Test: " << description << std::endl;
    std::cout << "FEN: " << fen << std::endl;
    std::cout << "Time Limit: " << timeLimitMs << "ms" << std::endl;
    std::cout << "========================================" << std::endl;

    Board board;
    board.loadFromFEN(fen);

    Moves moves(&board);
    Engine engine(&board, &moves);

    std::string bestMove = engine.getBestMove(timeLimitMs);

    std::cout << "\n=== FINAL RESULTS ===" << std::endl;
    std::cout << "Best Move: " << bestMove << std::endl;
    std::cout << "Total Nodes: " << engine.getNodesSearched() << std::endl;
    std::cout << "Q-Nodes: " << engine.getQNodesSearched() << std::endl;
    std::cout << "Beta Cutoffs: " << engine.getBetaCutoffs() << std::endl;
    std::cout << "First Move Cutoffs: " << engine.getFirstMoveCutoffs() << std::endl;
    std::cout << "Max Q-Depth: " << engine.getMaxQDepth() << std::endl;
    std::cout << "Avg Q-Depth: " << engine.getAvgQDepth() << std::endl;

    if (engine.getBetaCutoffs() > 0) {
        double moveOrderingEfficiency = (double)engine.getFirstMoveCutoffs() / engine.getBetaCutoffs() * 100.0;
        std::cout << "Move Ordering Efficiency: " << moveOrderingEfficiency << "%" << std::endl;
    }
}

int main() {
    std::cout << "===========================================\n";
    std::cout << "Iterative Deepening Test Suite\n";
    std::cout << "===========================================\n";

    // Test 1: Starting position with SHORT time (500ms)
    testPosition(
        "Starting Position - SHORT (500ms)",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        500
    );

    // Test 2: Starting position with MEDIUM time (2000ms)
    testPosition(
        "Starting Position - MEDIUM (2000ms)",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        2000
    );

    // Test 3: Starting position with LONG time (5000ms)
    testPosition(
        "Starting Position - LONG (5000ms)",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        5000
    );

    // Test 4: Tactical position (mate in 2 available) - should find quickly
    testPosition(
        "Mate in 2 - Tactical Position (1000ms)",
        "r1bqkb1r/pppp1Qpp/2n2n2/4p3/2B1P3/8/PPPP1PPP/RNB1K1NR b KQkq - 0 4",
        1000
    );

    // Test 5: Complex middlegame position with SHORT time
    testPosition(
        "Complex Middlegame - SHORT (1000ms)",
        "r1bq1rk1/pp2ppbp/2np1np1/8/2BNP3/2N1BP2/PPPQ2PP/R3K2R w KQ - 0 10",
        1000
    );

    // Test 6: Complex middlegame position with LONG time
    testPosition(
        "Complex Middlegame - LONG (5000ms)",
        "r1bq1rk1/pp2ppbp/2np1np1/8/2BNP3/2N1BP2/PPPQ2PP/R3K2R w KQ - 0 10",
        5000
    );

    // Test 7: VERY SHORT time (100ms) - should return move from shallow depth
    testPosition(
        "Starting Position - VERY SHORT (100ms)",
        "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
        100
    );

    std::cout << "\n===========================================\n";
    std::cout << "All tests completed!\n";
    std::cout << "===========================================\n";

    return 0;
}
