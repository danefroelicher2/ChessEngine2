#include "board.h"
#include "moves.h"
#include "engine.h"
#include <iostream>
#include <iomanip>
#include <cmath>

void test1_hangingQueen() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 1: CAN ENGINE SEE HANGING QUEEN?" << std::endl;
    std::cout << "========================================" << std::endl;

    Board board;
    // Black queen on e5, completely hanging (no defenders)
    board.loadFromFEN("rnb1kbnr/pppppppp/8/4q3/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    std::cout << board.toString() << std::endl;
    std::cout << "Black queen on e5 is HANGING (no defenders)" << std::endl;
    std::cout << "Expected: White should play Qd1-e5 capturing the queen" << std::endl;

    Moves moves(&board);

    // Check if Qxe5 is in legal moves
    std::vector<std::string> legalMoves = moves.generateLegalMoves();
    std::cout << "\nTotal legal moves: " << legalMoves.size() << std::endl;

    bool foundCapture = false;
    for (const auto& move : legalMoves) {
        if (move == "d1e5") {  // Qxe5
            foundCapture = true;
            std::cout << "✓ Found Qd1xe5 in legal moves" << std::endl;
            break;
        }
    }

    if (!foundCapture) {
        std::cout << "✗ CRITICAL ERROR: Qd1xe5 NOT in legal moves!" << std::endl;
        std::cout << "   Move generation is BROKEN!" << std::endl;
        std::cout << "\nShowing all legal moves:" << std::endl;
        for (size_t i = 0; i < legalMoves.size() && i < 20; i++) {
            std::cout << "  " << legalMoves[i] << std::endl;
        }
        return;
    }

    // Now ask engine for best move
    Engine engine(&board, &moves);
    std::cout << "\nAsking engine for best move (3 seconds)..." << std::endl;
    std::string bestMove = engine.getBestMove(3000);
    std::cout << "\nEngine chose: " << bestMove << std::endl;

    if (bestMove == "d1e5") {
        std::cout << "✓✓✓ PASS: Engine captures hanging queen!" << std::endl;
    } else {
        std::cout << "✗✗✗ CRITICAL FAIL: Engine did NOT capture hanging queen!" << std::endl;
        std::cout << "    This is a FREE QUEEN (+900 material)" << std::endl;
        std::cout << "    CRITICAL BUG: Engine cannot see obvious captures!" << std::endl;
    }
}

void test2_evaluationSanity() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 2: EVALUATION SANITY CHECK" << std::endl;
    std::cout << "========================================" << std::endl;

    Board board;
    Moves moves(&board);
    Engine engine(&board, &moves);

    // Make evaluate() public or create a test method
    // For now, we'll test via search results

    // Position 1: Starting position (equal material)
    board.loadFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::cout << "\nPosition 1: Starting position (equal material)" << std::endl;
    // We can't call evaluate() directly, but we can observe behavior

    // Position 2: White up a queen
    board.loadFromFEN("rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    std::cout << "Position 2: White up a queen (should be ~+900)" << std::endl;

    // Position 3: Black up a queen
    board.loadFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNB1KBNR w KQkq - 0 1");
    std::cout << "Position 3: Black up a queen (should be ~-900)" << std::endl;

    std::cout << "\nNote: Can't access evaluate() directly (private)" << std::endl;
    std::cout << "If engine fails Test 1, evaluation might be broken" << std::endl;
}

void test3_captureDetection() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 3: CAPTURE DETECTION" << std::endl;
    std::cout << "========================================" << std::endl;

    Board board;
    // White queen on e2, black pawn on e5, white can capture
    board.loadFromFEN("rnbqkbnr/pppp1ppp/8/4p3/8/8/PPPPQPPP/RNB1KBNR w KQkq - 0 1");

    std::cout << board.toString() << std::endl;
    std::cout << "White queen on e2, black pawn on e5" << std::endl;
    std::cout << "Checking if captures are detected in move generation..." << std::endl;

    Moves moves(&board);
    std::vector<std::string> allMoves = moves.generateLegalMoves();

    std::cout << "\nTotal legal moves: " << allMoves.size() << std::endl;

    int captureCount = 0;
    std::vector<std::string> captures;

    for (const auto& move : allMoves) {
        int toCol = move[2] - 'a';
        int toRow = 7 - (move[3] - '1');
        char target = board.getPiece(toRow, toCol);

        if (target != '.') {
            captureCount++;
            captures.push_back(move);
            std::cout << "  Capture: " << move << " takes '" << target << "'" << std::endl;
        }
    }

    std::cout << "\nTotal captures found: " << captureCount << std::endl;

    if (captureCount == 0) {
        std::cout << "✗ ERROR: No captures detected when captures exist!" << std::endl;
        std::cout << "   Move generation might be broken!" << std::endl;
    } else {
        std::cout << "✓ Captures are being detected" << std::endl;

        // Check if Qxe5 is among them
        bool foundQxe5 = false;
        for (const auto& cap : captures) {
            if (cap == "e2e5") {
                foundQxe5 = true;
                break;
            }
        }

        if (foundQxe5) {
            std::cout << "✓ Queen capture (Qe2xe5) found" << std::endl;
        } else {
            std::cout << "✗ WARNING: Queen capture not in list" << std::endl;
        }
    }
}

void test4_searchDepth() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 4: SEARCH DEPTH VERIFICATION" << std::endl;
    std::cout << "========================================" << std::endl;

    Board board;
    board.loadFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Moves moves(&board);
    Engine engine(&board, &moves);

    std::cout << "Running 2 second search from starting position..." << std::endl;
    std::string move = engine.getBestMove(2000);

    std::cout << "\n--- CHECK THE OUTPUT ABOVE ---" << std::endl;
    std::cout << "Questions to verify:" << std::endl;
    std::cout << "1. Did iterative deepening reach depth >= 4?" << std::endl;
    std::cout << "2. Were thousands of nodes searched?" << std::endl;
    std::cout << "3. Is TT hit rate > 0%?" << std::endl;
    std::cout << "\nMove chosen: " << move << std::endl;

    long long nodes = engine.getNodesSearched() + engine.getQNodesSearched();
    std::cout << "\nTotal nodes searched: " << nodes << std::endl;

    if (nodes < 100) {
        std::cout << "✗ CRITICAL ERROR: Search barely ran (< 100 nodes)" << std::endl;
        std::cout << "   Search is completely broken!" << std::endl;
    } else if (nodes < 1000) {
        std::cout << "✗ WARNING: Very few nodes searched" << std::endl;
        std::cout << "   Search might be cutting off prematurely" << std::endl;
    } else {
        std::cout << "✓ Search appears to be running" << std::endl;
    }
}

void test5_hashConsistency() {
    std::cout << "\n========================================" << std::endl;
    std::cout << "TEST 5: HASH CONSISTENCY CHECK" << std::endl;
    std::cout << "========================================" << std::endl;

    Board testBoard;
    uint64_t hash1 = testBoard.getHash();
    std::cout << "Hash before move: 0x" << std::hex << hash1 << std::dec << std::endl;

    // Make and unmake a move
    Moves testMoves(&testBoard);
    MoveInfo info = testMoves.makeMoveWithInfo("e2e4");
    uint64_t hash2 = testBoard.getHash();
    std::cout << "Hash after e2e4:  0x" << std::hex << hash2 << std::dec << std::endl;

    testMoves.unmakeMove("e2e4", info);
    uint64_t hash3 = testBoard.getHash();
    std::cout << "Hash after unmake: 0x" << std::hex << hash3 << std::dec << std::endl;

    if (hash1 == hash3) {
        std::cout << "✓ Hash restore working correctly" << std::endl;
    } else {
        std::cout << "✗ CRITICAL ERROR: Hash not restored!" << std::endl;
        std::cout << "   Transposition table will be corrupted!" << std::endl;
        std::cout << "   This causes engine to use wrong evaluations!" << std::endl;
    }

    // Test multiple moves
    std::cout << "\nTesting sequence of moves..." << std::endl;
    std::vector<std::string> moveSeq = {"e2e4", "e7e5", "g1f3", "b8c6"};
    std::vector<MoveInfo> infos;

    uint64_t startHash = testBoard.getHash();

    for (const auto& move : moveSeq) {
        infos.push_back(testMoves.makeMoveWithInfo(move));
    }

    // Unmake all
    for (int i = moveSeq.size() - 1; i >= 0; i--) {
        testMoves.unmakeMove(moveSeq[i], infos[i]);
    }

    uint64_t finalHash = testBoard.getHash();

    if (startHash == finalHash) {
        std::cout << "✓ Sequence hash restore working" << std::endl;
    } else {
        std::cout << "✗ ERROR: Sequence hash not restored!" << std::endl;
    }
}

int main() {
    std::cout << "================================================" << std::endl;
    std::cout << "   CRITICAL DIAGNOSTIC TEST SUITE" << std::endl;
    std::cout << "   Testing for fundamental engine bugs" << std::endl;
    std::cout << "================================================" << std::endl;

    test1_hangingQueen();    // MOST CRITICAL
    test2_evaluationSanity();
    test3_captureDetection();
    test4_searchDepth();
    test5_hashConsistency();

    std::cout << "\n================================================" << std::endl;
    std::cout << "   DIAGNOSTIC TESTS COMPLETE" << std::endl;
    std::cout << "================================================" << std::endl;
    std::cout << "\nPRIORITY: Fix Test 1 first - if engine can't" << std::endl;
    std::cout << "capture a hanging queen, nothing else matters!" << std::endl;

    return 0;
}
