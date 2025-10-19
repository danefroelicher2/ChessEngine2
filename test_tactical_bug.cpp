#include "board.h"
#include "moves.h"
#include "engine.h"
#include <iostream>
#include <iomanip>

void printBoard(Board* board) {
    std::cout << board->toString() << std::endl;
}

void test1_captureDetection() {
    std::cout << "\n========== TEST 1: CAPTURE DETECTION ==========\n";

    // Set up a position where Black just captured White's queen with a knight
    // Now we need to see if recaptures are detected
    Board board;
    board.loadFromFEN("rnbqkb1r/pppppppp/8/8/8/8/PPPnQPPP/RNBQKB1R w KQkq - 0 1");
    // Black knight on d2, White Queen was on e2 (just captured)

    std::cout << "Position (Black knight just took White Queen on d2):\n";
    printBoard(&board);

    Moves moves(&board);
    std::vector<std::string> allMoves = moves.generateLegalMoves();

    std::cout << "\nAll legal moves: " << allMoves.size() << "\n";

    // Look for recaptures of the knight on d2
    std::cout << "\nLooking for recaptures of knight on d2:\n";
    int recaptureCount = 0;
    for (const std::string& move : allMoves) {
        int toCol = move[2] - 'a';
        int toRow = 7 - (move[3] - '1');

        // d2 is column 3 (d), row 6 (rank 2 in our 0-indexed system)
        if (toRow == 6 && toCol == 3) {
            char target = board.getPiece(toRow, toCol);
            std::cout << "  Found recapture: " << move << " (target=" << target << ")\n";
            recaptureCount++;
        }
    }

    std::cout << "\nTotal recaptures found: " << recaptureCount << "\n";

    if (recaptureCount == 0) {
        std::cout << "❌ BUG: No recaptures found!\n";
    } else {
        std::cout << "✓ Recaptures detected correctly\n";
    }
}

void test2_quiescenceGeneration() {
    std::cout << "\n========== TEST 2: QUIESCENCE TACTICAL MOVE GENERATION ==========\n";

    // After QxQ, check if quiescence finds recaptures
    Board board;
    board.loadFromFEN("rnbqkb1r/pppppppp/8/8/8/8/PPPnQPPP/RNBQKB1R w KQkq - 0 1");

    std::cout << "Position:\n";
    printBoard(&board);

    Moves moves(&board);
    Engine engine(&board, &moves);

    // Manually call generateTacticalMoves to see what it finds
    std::cout << "\nCalling generateTacticalMoves()...\n";

    // We need to make this accessible - for now, let's check through legal moves
    std::vector<std::string> allMoves = moves.generateLegalMoves();
    std::vector<std::string> captures;

    for (const std::string& move : allMoves) {
        int toCol = move[2] - 'a';
        int toRow = 7 - (move[3] - '1');
        char target = board.getPiece(toRow, toCol);

        if (target != '.') {
            captures.push_back(move);
            std::cout << "  Capture: " << move << " (takes " << target << ")\n";
        }
    }

    std::cout << "\nTotal captures: " << captures.size() << "\n";

    if (captures.size() == 0) {
        std::cout << "❌ BUG: No captures found by generateTacticalMoves equivalent!\n";
    } else {
        std::cout << "✓ Tactical moves being generated\n";
    }
}

void test3_hashConsistency() {
    std::cout << "\n========== TEST 3: HASH CONSISTENCY (MAKE/UNMAKE) ==========\n";

    Board board;
    Moves moves(&board);

    uint64_t hash1 = board.getHash();
    std::cout << "Initial hash: " << std::hex << hash1 << std::dec << "\n";

    // Make a move
    std::string testMove = "e2e4";
    MoveInfo info = moves.makeMoveWithInfo(testMove);
    uint64_t hash2 = board.getHash();
    std::cout << "After " << testMove << ": " << std::hex << hash2 << std::dec << "\n";

    // Unmake the move
    moves.unmakeMove(testMove, info);
    uint64_t hash3 = board.getHash();
    std::cout << "After unmake: " << std::hex << hash3 << std::dec << "\n";

    if (hash1 != hash3) {
        std::cout << "❌ BUG: Hash not restored! " << hash1 << " != " << hash3 << "\n";
    } else {
        std::cout << "✓ Hash consistency verified\n";
    }

    // Test multiple moves
    std::cout << "\nTesting sequence of moves...\n";
    std::vector<std::string> moveSeq = {"e2e4", "e7e5", "g1f3"};
    std::vector<MoveInfo> infos;

    for (const std::string& move : moveSeq) {
        infos.push_back(moves.makeMoveWithInfo(move));
        std::cout << "After " << move << ": hash = " << std::hex << board.getHash() << std::dec << "\n";
    }

    // Unmake all
    for (int i = moveSeq.size() - 1; i >= 0; i--) {
        moves.unmakeMove(moveSeq[i], infos[i]);
    }

    uint64_t finalHash = board.getHash();
    std::cout << "Final hash: " << std::hex << finalHash << std::dec << "\n";

    if (hash1 != finalHash) {
        std::cout << "❌ BUG: Hash not restored after sequence!\n";
    } else {
        std::cout << "✓ Hash sequence consistency verified\n";
    }
}

void test4_quiescenceEvaluation() {
    std::cout << "\n========== TEST 4: QUIESCENCE EVALUATION ==========\n";

    // Position where NOT recapturing loses material
    Board board;
    board.loadFromFEN("rnbqkb1r/pppppppp/8/8/8/8/PPPnQPPP/RNBQKB1R w KQkq - 0 1");

    std::cout << "Position (Black knight on d2, just took queen):\n";
    printBoard(&board);

    Moves moves(&board);
    Engine engine(&board, &moves);

    // Evaluate this position
    // White should see they're down material and need to recapture
    std::cout << "\nSearching for best move...\n";
    std::string bestMove = engine.getBestMove(2000);  // 2 second search

    std::cout << "\nBest move found: " << bestMove << "\n";

    // Check if it's a recapture
    if (bestMove.length() >= 4) {
        int toCol = bestMove[2] - 'a';
        int toRow = 7 - (bestMove[3] - '1');

        if (toRow == 6 && toCol == 3) {  // d2
            std::cout << "✓ Engine chose to recapture!\n";
        } else {
            std::cout << "❌ BUG: Engine did NOT recapture! Chose: " << bestMove << "\n";
            std::cout << "  This is the critical bug!\n";
        }
    }
}

void test5_simpleRecapture() {
    std::cout << "\n========== TEST 5: SIMPLE QUEEN TRADE ==========\n";

    // Even simpler: Queens facing each other
    Board board;
    board.loadFromFEN("rnb1kbnr/pppppppp/8/8/8/8/PPPPQPPP/RNBqKBNR w KQkq - 0 1");
    // Black queen on d1, White queen on e2 - Black just took something on d1

    std::cout << "Position (Black queen on d1, can be taken by White queen on e2):\n";
    printBoard(&board);

    Moves moves(&board);
    std::vector<std::string> allMoves = moves.generateLegalMoves();

    std::cout << "\nLooking for Qxd1...\n";
    bool foundCapture = false;
    for (const std::string& move : allMoves) {
        if (move == "e2d1") {
            foundCapture = true;
            std::cout << "✓ Found Qxd1\n";

            // Check what the evaluation thinks about this
            MoveInfo info = moves.makeMoveWithInfo(move);
            std::cout << "After Qxd1:\n";
            printBoard(&board);
            moves.unmakeMove(move, info);
            break;
        }
    }

    if (!foundCapture) {
        std::cout << "❌ BUG: Qxd1 not found in legal moves!\n";
    }

    // Now search
    Engine engine(&board, &moves);
    std::cout << "\nSearching for best move...\n";
    std::string bestMove = engine.getBestMove(2000);

    std::cout << "Best move: " << bestMove << "\n";
    if (bestMove == "e2d1") {
        std::cout << "✓ Engine chose to capture queen!\n";
    } else {
        std::cout << "❌ BUG: Engine did NOT capture queen! Chose: " << bestMove << "\n";
    }
}

int main() {
    std::cout << "==============================================\n";
    std::cout << "   TACTICAL BUG DIAGNOSTIC TESTS\n";
    std::cout << "==============================================\n";

    test1_captureDetection();
    test2_quiescenceGeneration();
    test3_hashConsistency();
    test5_simpleRecapture();  // Do this before the complex one
    test4_quiescenceEvaluation();  // This one does a full search

    std::cout << "\n==============================================\n";
    std::cout << "   DIAGNOSTIC TESTS COMPLETE\n";
    std::cout << "==============================================\n";

    return 0;
}
