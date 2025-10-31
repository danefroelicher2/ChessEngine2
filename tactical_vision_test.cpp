#include <iostream>
#include <string>
#include <vector>
#include "board.h"
#include "moves.h"
#include "engine.h"

// ANSI color codes for terminal output
#define GREEN "\033[32m"
#define RED "\033[31m"
#define YELLOW "\033[33m"
#define CYAN "\033[36m"
#define RESET "\033[0m"
#define BOLD "\033[1m"

struct TacticalTest {
    std::string name;
    std::string fen;
    std::string expectedMovePattern;  // Pattern to match (e.g., "xe5" for any capture on e5)
    std::string description;
    int maxPly;  // Maximum number of moves to find the tactic
};

bool matchesMovePattern(const std::string& move, const std::string& pattern) {
    // Simple pattern matching:
    // If pattern is "xe5", match any move ending with "xe5" (capture on e5)
    // If pattern is exact move like "e2e4", match exactly

    if (pattern.empty()) return false;

    // Check if pattern contains 'x' (capture notation)
    if (pattern.find('x') != std::string::npos) {
        // Extract target square from pattern (e.g., "xe5" -> "e5")
        size_t xPos = pattern.find('x');
        if (xPos != std::string::npos && xPos + 2 < pattern.length()) {
            std::string targetSquare = pattern.substr(xPos + 1, 2);
            // Check if move ends with this square
            if (move.length() >= 4) {
                std::string moveTarget = move.substr(2, 2);
                return moveTarget == targetSquare;
            }
        }
        return false;
    }

    // Check for multiple acceptable moves (separated by '|')
    if (pattern.find('|') != std::string::npos) {
        size_t start = 0;
        size_t end = pattern.find('|');
        while (end != std::string::npos) {
            std::string option = pattern.substr(start, end - start);
            if (move == option || move.substr(2, 2) == option.substr(2, 2)) {
                return true;
            }
            start = end + 1;
            end = pattern.find('|', start);
        }
        // Check last option
        std::string option = pattern.substr(start);
        if (move == option || move.substr(2, 2) == option.substr(2, 2)) {
            return true;
        }
        return false;
    }

    // Exact match or target square match
    return move == pattern ||
           (move.length() >= 4 && pattern.length() >= 4 &&
            move.substr(2, 2) == pattern.substr(2, 2));
}

void runTacticalTest(const TacticalTest& test, int& passCount, int& failCount) {
    std::cout << "\n" << BOLD << CYAN << "========================================"  << RESET << std::endl;
    std::cout << BOLD << "TEST: " << test.name << RESET << std::endl;
    std::cout << CYAN << "========================================" << RESET << std::endl;

    std::cout << "Description: " << test.description << std::endl;
    std::cout << "Position FEN: " << test.fen << std::endl;
    std::cout << "Expected: " << test.expectedMovePattern << std::endl;
    std::cout << "Max Ply: " << test.maxPly << std::endl;
    std::cout << std::endl;

    // Set up position
    Board board;
    board.loadFromFEN(test.fen);
    Moves moves(&board);
    Engine engine(&board, &moves);

    // Print board state
    std::cout << "Board state:" << std::endl;
    std::cout << board.toString() << std::endl;

    // Get best move (3 second search)
    std::string bestMove = engine.getBestMove(3000);

    // Check if move matches expected pattern
    bool passed = matchesMovePattern(bestMove, test.expectedMovePattern);

    std::cout << "\n" << BOLD << "RESULT: ";
    if (passed) {
        std::cout << GREEN << "✓ PASS" << RESET << std::endl;
        passCount++;
    } else {
        std::cout << RED << "✗ FAIL" << RESET << std::endl;
        failCount++;
    }

    std::cout << "Engine chose: " << BOLD << bestMove << RESET << std::endl;

    if (!passed) {
        std::cout << RED << "Expected pattern: " << test.expectedMovePattern << RESET << std::endl;

        // Provide diagnostic info
        std::cout << "\n" << YELLOW << "Diagnostic Information:" << RESET << std::endl;
        std::cout << "- First move cutoff rate: "
                  << (engine.getBetaCutoffs() > 0 ?
                      (100.0 * engine.getFirstMoveCutoffs() / engine.getBetaCutoffs()) : 0.0)
                  << "%" << std::endl;
        std::cout << "- Total nodes searched: " << engine.getNodesSearched() << std::endl;
        std::cout << "- Q-nodes searched: " << engine.getQNodesSearched() << std::endl;

        // Show all legal moves to help diagnose
        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        std::cout << "\nAll legal moves (" << legalMoves.size() << " total):" << std::endl;
        for (size_t i = 0; i < legalMoves.size() && i < 10; i++) {
            std::cout << "  " << legalMoves[i];
            if (matchesMovePattern(legalMoves[i], test.expectedMovePattern)) {
                std::cout << GREEN << " ← Expected move exists!" << RESET;
            }
            std::cout << std::endl;
        }
        if (legalMoves.size() > 10) {
            std::cout << "  ... (" << (legalMoves.size() - 10) << " more moves)" << std::endl;
        }
    }

    std::cout << std::endl;
}

int main() {
    std::cout << BOLD << CYAN << "\n╔════════════════════════════════════════════╗" << RESET << std::endl;
    std::cout << BOLD << CYAN << "║  TACTICAL VISION DIAGNOSTIC TEST SUITE   ║" << RESET << std::endl;
    std::cout << BOLD << CYAN << "╚════════════════════════════════════════════╝" << RESET << std::endl;
    std::cout << "\nTesting fundamental tactical pattern recognition..." << std::endl;
    std::cout << "Each test checks if the engine can see basic 1-2 ply tactics.\n" << std::endl;

    // Define tactical tests
    std::vector<TacticalTest> tests = {
        {
            "Hanging Piece Detection (1-ply)",
            "rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1",
            "xe5",  // Any capture on e5 (white queen is hanging)
            "White queen on e5 is completely undefended. Can the engine capture a free piece?",
            1
        },
        {
            "King Can Capture (1-ply)",
            "rnbqkbnr/pppppppp/8/8/4k3/4N3/PPPPPPPP/RNBQKB1R b KQkq - 0 1",
            "e4e3",  // King captures knight
            "White knight on e3 is attacked by Black king on e4. Does the engine know kings can capture?",
            1
        },
        {
            "Capture Checking Piece (1-ply)",
            "rnbqkbnr/pppp1ppp/8/8/8/8/PPPPnPPP/RNBQKBNR w KQkq - 0 1",
            "xe2",  // Any capture on e2 (knight checking king)
            "Black knight on e2 is checking the White king. Should capture the checking piece.",
            1
        },
        {
            "King Capture vs King Flight (1-ply)",
            "rnbqkb1r/pppppppp/8/8/8/4n3/PPPPKPPP/RNBQ1BNR w kq - 0 1",
            "e2e3",  // King captures knight
            "Black knight on e3 checks King on e2. King can capture it. Does engine prefer capturing over fleeing?",
            1
        },
        {
            "Free Material in Opening (1-ply)",
            "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPPNPPP/RNBQKB1R b KQkq - 0 1",
            "e5e4",  // Pawn captures pawn
            "White pawn on e4 is undefended after 1.e4 e5 2.Ne2??. Should capture free pawn.",
            1
        }
    };

    int passCount = 0;
    int failCount = 0;

    // Run all tests
    for (const auto& test : tests) {
        runTacticalTest(test, passCount, failCount);
    }

    // Print summary
    std::cout << "\n" << BOLD << CYAN << "========================================" << RESET << std::endl;
    std::cout << BOLD << "TACTICAL VISION TEST SUMMARY" << RESET << std::endl;
    std::cout << CYAN << "========================================" << RESET << std::endl;

    std::cout << "\nTests Passed: " << GREEN << BOLD << passCount << "/" << tests.size() << RESET << std::endl;
    std::cout << "Tests Failed: " << RED << BOLD << failCount << "/" << tests.size() << RESET << std::endl;

    double passRate = (100.0 * passCount) / tests.size();
    std::cout << "\nPass Rate: " << BOLD;
    if (passRate >= 80.0) {
        std::cout << GREEN << passRate << "%" << RESET << " (Good tactical vision)";
    } else if (passRate >= 60.0) {
        std::cout << YELLOW << passRate << "%" << RESET << " (Needs improvement)";
    } else {
        std::cout << RED << passRate << "%" << RESET << " (Critical tactical blindness)";
    }
    std::cout << std::endl;

    // Critical issues
    if (failCount > 0) {
        std::cout << "\n" << BOLD << RED << "CRITICAL ISSUES FOUND:" << RESET << std::endl;

        if (failCount >= 3) {
            std::cout << "  " << RED << "⚠ Engine has severe tactical blindness" << RESET << std::endl;
            std::cout << "  " << RED << "⚠ Missing basic 1-ply tactics" << RESET << std::endl;
        }

        std::cout << "\n" << YELLOW << "RECOMMENDED ACTIONS:" << RESET << std::endl;
        std::cout << "  1. Check evaluation function - are captures scored correctly?" << std::endl;
        std::cout << "  2. Verify move ordering - are captures searched first?" << RESET << std::endl;
        std::cout << "  3. Check quiescence search - is it finding tactical shots?" << std::endl;
        std::cout << "  4. Review SEE implementation - is it correctly evaluating captures?" << std::endl;
    } else {
        std::cout << "\n" << GREEN << BOLD << "✓ All tactical vision tests passed!" << RESET << std::endl;
        std::cout << GREEN << "  Engine has good basic tactical awareness." << RESET << std::endl;
    }

    std::cout << "\n" << CYAN << "========================================" << RESET << std::endl;
    std::cout << std::endl;

    return (failCount > 0) ? 1 : 0;
}
