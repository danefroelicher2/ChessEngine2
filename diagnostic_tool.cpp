#include <iostream>
#include <iomanip>
#include <vector>
#include <set>
#include <string>
#include <cstdlib>
#include <ctime>
#include "board.h"
#include "moves.h"

// Helper function to print board state
void printBoardState(Board* board) {
    std::cout << "Turn: " << (board->isWhiteToMove() ? "White" : "Black") << std::endl;
    std::cout << "Hash: 0x" << std::hex << board->getHash() << std::dec << std::endl;
    std::cout << "White Kingside: " << board->canCastleKingside(true) << std::endl;
    std::cout << "White Queenside: " << board->canCastleQueenside(true) << std::endl;
    std::cout << "Black Kingside: " << board->canCastleKingside(false) << std::endl;
    std::cout << "Black Queenside: " << board->canCastleQueenside(false) << std::endl;
    std::cout << "En passant: (" << board->getEnPassantRow() << ", " << board->getEnPassantCol() << ")" << std::endl;
}

// Helper to find king position
bool findKing(Board* board, char kingPiece, int& row, int& col) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board->getPiece(r, c) == kingPiece) {
                row = r;
                col = c;
                return true;
            }
        }
    }
    return false;
}

// Helper to count pieces on board
int countPiece(Board* board, char piece) {
    int count = 0;
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board->getPiece(r, c) == piece) {
                count++;
            }
        }
    }
    return count;
}

// Helper to compare board positions
bool compareBoardPositions(Board* board, char savedBoard[8][8]) {
    for (int r = 0; r < 8; r++) {
        for (int c = 0; c < 8; c++) {
            if (board->getPiece(r, c) != savedBoard[r][c]) {
                return false;
            }
        }
    }
    return true;
}

// TEST 1: Move/Unmove Consistency
bool test1_moveUnmoveConsistency() {
    std::cout << "\n=== TEST 1: Move/Unmove Consistency ===" << std::endl;

    Board board;
    Moves moves(&board);
    int failures = 0;
    int tests = 0;

    for (int i = 0; i < 100; i++) {
        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        if (legalMoves.empty()) break;

        // Pick a random legal move
        std::string move = legalMoves[rand() % legalMoves.size()];

        // Save board state
        char savedBoard[8][8];
        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                savedBoard[r][c] = board.getPiece(r, c);
            }
        }
        bool savedTurn = board.isWhiteToMove();
        bool savedWK = board.canCastleKingside(true);
        bool savedWQ = board.canCastleQueenside(true);
        bool savedBK = board.canCastleKingside(false);
        bool savedBQ = board.canCastleQueenside(false);
        int savedEPRow = board.getEnPassantRow();
        int savedEPCol = board.getEnPassantCol();

        // Make and unmake move
        MoveInfo info = moves.makeMoveWithInfo(move);
        moves.unmakeMove(move, info);

        // Compare
        tests++;
        bool posMatch = compareBoardPositions(&board, savedBoard);
        bool turnMatch = (board.isWhiteToMove() == savedTurn);
        bool castleMatch = (board.canCastleKingside(true) == savedWK &&
                           board.canCastleQueenside(true) == savedWQ &&
                           board.canCastleKingside(false) == savedBK &&
                           board.canCastleQueenside(false) == savedBQ);
        bool epMatch = (board.getEnPassantRow() == savedEPRow &&
                       board.getEnPassantCol() == savedEPCol);

        if (!posMatch || !turnMatch || !castleMatch || !epMatch) {
            failures++;
            std::cout << "FAIL on move: " << move << std::endl;
            if (!posMatch) std::cout << "  - Board position mismatch" << std::endl;
            if (!turnMatch) std::cout << "  - Turn mismatch" << std::endl;
            if (!castleMatch) std::cout << "  - Castling rights mismatch" << std::endl;
            if (!epMatch) std::cout << "  - En passant mismatch" << std::endl;
        }

        // Continue with the actual move for next iteration
        moves.makeMoveWithInfo(move);
    }

    std::cout << "Status: " << (failures == 0 ? "PASS" : "FAIL") << std::endl;
    std::cout << "Tests: " << tests << ", Failures: " << failures << std::endl;
    return failures == 0;
}

// TEST 2: Hash Consistency
bool test2_hashConsistency() {
    std::cout << "\n=== TEST 2: Hash Consistency ===" << std::endl;

    Board board;
    Moves moves(&board);
    int failures = 0;
    int tests = 0;

    for (int i = 0; i < 100; i++) {
        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        if (legalMoves.empty()) break;

        std::string move = legalMoves[rand() % legalMoves.size()];

        uint64_t hash1 = board.getHash();

        MoveInfo info = moves.makeMoveWithInfo(move);
        moves.unmakeMove(move, info);

        uint64_t hash2 = board.getHash();

        tests++;
        if (hash1 != hash2) {
            failures++;
            std::cout << "FAIL on move: " << move << std::endl;
            std::cout << "  Before: 0x" << std::hex << hash1 << std::dec << std::endl;
            std::cout << "  After:  0x" << std::hex << hash2 << std::dec << std::endl;
        }

        // Continue with move
        moves.makeMoveWithInfo(move);
    }

    std::cout << "Status: " << (failures == 0 ? "PASS" : "FAIL") << std::endl;
    std::cout << "Tests: " << tests << ", Failures: " << failures << std::endl;
    return failures == 0;
}

// TEST 3: Hash Uniqueness
bool test3_hashUniqueness() {
    std::cout << "\n=== TEST 3: Hash Uniqueness ===" << std::endl;

    Board board;
    Moves moves(&board);
    std::set<uint64_t> hashes;
    int collisions = 0;
    int positions = 0;

    for (int i = 0; i < 1000; i++) {
        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        if (legalMoves.empty()) break;

        uint64_t hash = board.getHash();

        if (hashes.count(hash) > 0) {
            collisions++;
        }
        hashes.insert(hash);
        positions++;

        std::string move = legalMoves[rand() % legalMoves.size()];
        moves.makeMoveWithInfo(move);
    }

    bool pass = (collisions < 5);  // Less than 0.5% collision rate is acceptable
    std::cout << "Status: " << (pass ? "PASS" : "FAIL") << std::endl;
    std::cout << "Unique positions: " << positions << std::endl;
    std::cout << "Collisions: " << collisions << " (" << (100.0 * collisions / positions) << "%)" << std::endl;
    return pass;
}

// TEST 4: Transposition Detection
bool test4_transpositionDetection() {
    std::cout << "\n=== TEST 4: Transposition Detection ===" << std::endl;

    Board board1, board2;
    Moves moves1(&board1), moves2(&board2);

    // Position 1: e4, e5
    moves1.makeMoveWithInfo("e2e4");
    moves1.makeMoveWithInfo("e7e5");

    // Position 2: e5, e4 (same position, different order)
    moves2.makeMoveWithInfo("e7e5");
    moves2.makeMoveWithInfo("e2e4");

    uint64_t hash1 = board1.getHash();
    uint64_t hash2 = board2.getHash();

    bool pass = (hash1 == hash2);
    std::cout << "Status: " << (pass ? "PASS" : "FAIL") << std::endl;
    if (!pass) {
        std::cout << "Position 1 hash: 0x" << std::hex << hash1 << std::dec << std::endl;
        std::cout << "Position 2 hash: 0x" << std::hex << hash2 << std::dec << std::endl;
    }
    return pass;
}

// TEST 5: King Always Findable
bool test5_kingAlwaysFindable() {
    std::cout << "\n=== TEST 5: King Always Findable ===" << std::endl;

    Board board;
    Moves moves(&board);
    int failures = 0;
    int tests = 0;

    for (int i = 0; i < 100; i++) {
        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        if (legalMoves.empty()) break;

        int whiteKingCount = countPiece(&board, 'K');
        int blackKingCount = countPiece(&board, 'k');

        tests++;
        if (whiteKingCount != 1 || blackKingCount != 1) {
            failures++;
            std::cout << "FAIL: White kings: " << whiteKingCount
                      << ", Black kings: " << blackKingCount << std::endl;
        }

        std::string move = legalMoves[rand() % legalMoves.size()];
        moves.makeMoveWithInfo(move);
    }

    std::cout << "Status: " << (failures == 0 ? "PASS" : "FAIL") << std::endl;
    std::cout << "Tests: " << tests << ", Failures: " << failures << std::endl;
    return failures == 0;
}

// TEST 6: King Location After Move/Unmove
bool test6_kingLocationConsistency() {
    std::cout << "\n=== TEST 6: King Location After Move/Unmove ===" << std::endl;

    Board board;
    Moves moves(&board);
    int failures = 0;
    int tests = 0;

    for (int i = 0; i < 100; i++) {
        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        if (legalMoves.empty()) break;

        std::string move = legalMoves[rand() % legalMoves.size()];

        int wkRow1, wkCol1, bkRow1, bkCol1;
        bool foundWK1 = findKing(&board, 'K', wkRow1, wkCol1);
        bool foundBK1 = findKing(&board, 'k', bkRow1, bkCol1);

        MoveInfo info = moves.makeMoveWithInfo(move);
        moves.unmakeMove(move, info);

        int wkRow2, wkCol2, bkRow2, bkCol2;
        bool foundWK2 = findKing(&board, 'K', wkRow2, wkCol2);
        bool foundBK2 = findKing(&board, 'k', bkRow2, bkCol2);

        tests++;
        if (!foundWK1 || !foundBK1 || !foundWK2 || !foundBK2 ||
            wkRow1 != wkRow2 || wkCol1 != wkCol2 ||
            bkRow1 != bkRow2 || bkCol1 != bkCol2) {
            failures++;
            std::cout << "FAIL on move: " << move << std::endl;
            std::cout << "  White king: (" << wkRow1 << "," << wkCol1 << ") -> ("
                      << wkRow2 << "," << wkCol2 << ")" << std::endl;
            std::cout << "  Black king: (" << bkRow1 << "," << bkCol1 << ") -> ("
                      << bkRow2 << "," << bkCol2 << ")" << std::endl;
        }

        moves.makeMoveWithInfo(move);
    }

    std::cout << "Status: " << (failures == 0 ? "PASS" : "FAIL") << std::endl;
    std::cout << "Tests: " << tests << ", Failures: " << failures << std::endl;
    return failures == 0;
}

// TEST 7: Board Array Integrity
bool test7_boardArrayIntegrity() {
    std::cout << "\n=== TEST 7: Board Array Integrity ===" << std::endl;

    Board board;
    Moves moves(&board);
    int failures = 0;
    int tests = 0;

    const char* validPieces = ".PNBRQKpnbrqk";

    for (int i = 0; i < 100; i++) {
        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        if (legalMoves.empty()) break;

        for (int r = 0; r < 8; r++) {
            for (int c = 0; c < 8; c++) {
                char piece = board.getPiece(r, c);
                tests++;
                bool valid = false;
                for (int j = 0; validPieces[j] != '\0'; j++) {
                    if (piece == validPieces[j]) {
                        valid = true;
                        break;
                    }
                }
                if (!valid) {
                    failures++;
                    std::cout << "FAIL: Invalid piece '" << piece << "' at ("
                              << r << "," << c << ")" << std::endl;
                }
            }
        }

        std::string move = legalMoves[rand() % legalMoves.size()];
        moves.makeMoveWithInfo(move);
    }

    std::cout << "Status: " << (failures == 0 ? "PASS" : "FAIL") << std::endl;
    std::cout << "Tests: " << tests << ", Failures: " << failures << std::endl;
    return failures == 0;
}

// TEST 8: Castling Rights Consistency
bool test8_castlingConsistency() {
    std::cout << "\n=== TEST 8: Castling Rights Consistency ===" << std::endl;

    Board board;
    Moves moves(&board);
    int failures = 0;
    int tests = 0;

    for (int i = 0; i < 100; i++) {
        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        if (legalMoves.empty()) break;

        std::string move = legalMoves[rand() % legalMoves.size()];

        bool wk1 = board.canCastleKingside(true);
        bool wq1 = board.canCastleQueenside(true);
        bool bk1 = board.canCastleKingside(false);
        bool bq1 = board.canCastleQueenside(false);

        MoveInfo info = moves.makeMoveWithInfo(move);
        moves.unmakeMove(move, info);

        bool wk2 = board.canCastleKingside(true);
        bool wq2 = board.canCastleQueenside(true);
        bool bk2 = board.canCastleKingside(false);
        bool bq2 = board.canCastleQueenside(false);

        tests++;
        if (wk1 != wk2 || wq1 != wq2 || bk1 != bk2 || bq1 != bq2) {
            failures++;
            std::cout << "FAIL on move: " << move << std::endl;
            std::cout << "  WK: " << wk1 << " -> " << wk2 << std::endl;
            std::cout << "  WQ: " << wq1 << " -> " << wq2 << std::endl;
            std::cout << "  BK: " << bk1 << " -> " << bk2 << std::endl;
            std::cout << "  BQ: " << bq1 << " -> " << bq2 << std::endl;
        }

        moves.makeMoveWithInfo(move);
    }

    std::cout << "Status: " << (failures == 0 ? "PASS" : "FAIL") << std::endl;
    std::cout << "Tests: " << tests << ", Failures: " << failures << std::endl;
    return failures == 0;
}

// TEST 9: En Passant Consistency
bool test9_enPassantConsistency() {
    std::cout << "\n=== TEST 9: En Passant Square Consistency ===" << std::endl;

    Board board;
    Moves moves(&board);
    int failures = 0;
    int tests = 0;

    for (int i = 0; i < 100; i++) {
        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        if (legalMoves.empty()) break;

        std::string move = legalMoves[rand() % legalMoves.size()];

        int epRow1 = board.getEnPassantRow();
        int epCol1 = board.getEnPassantCol();

        MoveInfo info = moves.makeMoveWithInfo(move);
        moves.unmakeMove(move, info);

        int epRow2 = board.getEnPassantRow();
        int epCol2 = board.getEnPassantCol();

        tests++;
        if (epRow1 != epRow2 || epCol1 != epCol2) {
            failures++;
            std::cout << "FAIL on move: " << move << std::endl;
            std::cout << "  EP: (" << epRow1 << "," << epCol1 << ") -> ("
                      << epRow2 << "," << epCol2 << ")" << std::endl;
        }

        moves.makeMoveWithInfo(move);
    }

    std::cout << "Status: " << (failures == 0 ? "PASS" : "FAIL") << std::endl;
    std::cout << "Tests: " << tests << ", Failures: " << failures << std::endl;
    return failures == 0;
}

// TEST 10: Deep Search State
bool test10_deepSearchState() {
    std::cout << "\n=== TEST 10: Deep Search State ===" << std::endl;

    Board board;
    Moves moves(&board);
    int failures = 0;
    int tests = 5;

    for (int test = 0; test < tests; test++) {
        uint64_t originalHash = board.getHash();

        // Generate 4 legal moves
        std::vector<std::string> moveSeq;
        std::vector<MoveInfo> infoSeq;

        for (int depth = 0; depth < 4; depth++) {
            std::vector<std::string> legalMoves = moves.generateLegalMoves();
            if (legalMoves.empty()) break;

            std::string move = legalMoves[rand() % legalMoves.size()];
            MoveInfo info = moves.makeMoveWithInfo(move);
            moveSeq.push_back(move);
            infoSeq.push_back(info);
        }

        // Unmake all moves in reverse
        for (int i = moveSeq.size() - 1; i >= 0; i--) {
            moves.unmakeMove(moveSeq[i], infoSeq[i]);
        }

        uint64_t finalHash = board.getHash();

        if (originalHash != finalHash) {
            failures++;
            std::cout << "FAIL on test " << test << std::endl;
            std::cout << "  Original: 0x" << std::hex << originalHash << std::dec << std::endl;
            std::cout << "  Final:    0x" << std::hex << finalHash << std::dec << std::endl;
            std::cout << "  Moves: ";
            for (const auto& move : moveSeq) std::cout << move << " ";
            std::cout << std::endl;
        }

        // Do one move to change position for next test
        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        if (!legalMoves.empty()) {
            moves.makeMoveWithInfo(legalMoves[rand() % legalMoves.size()]);
        }
    }

    std::cout << "Status: " << (failures == 0 ? "PASS" : "FAIL") << std::endl;
    std::cout << "Tests: " << tests << ", Failures: " << failures << std::endl;
    return failures == 0;
}

int main() {
    srand(time(NULL));

    std::cout << "========================================" << std::endl;
    std::cout << "BOARD STATE DIAGNOSTIC TOOL" << std::endl;
    std::cout << "========================================" << std::endl;

    std::vector<std::pair<std::string, bool>> results;

    results.push_back({"Move/Unmove Consistency", test1_moveUnmoveConsistency()});
    results.push_back({"Hash Consistency", test2_hashConsistency()});
    results.push_back({"Hash Uniqueness", test3_hashUniqueness()});
    results.push_back({"Transposition Detection", test4_transpositionDetection()});
    results.push_back({"King Always Findable", test5_kingAlwaysFindable()});
    results.push_back({"King Location Consistency", test6_kingLocationConsistency()});
    results.push_back({"Board Array Integrity", test7_boardArrayIntegrity()});
    results.push_back({"Castling Rights Consistency", test8_castlingConsistency()});
    results.push_back({"En Passant Consistency", test9_enPassantConsistency()});
    results.push_back({"Deep Search State", test10_deepSearchState()});

    std::cout << "\n========================================" << std::endl;
    std::cout << "DIAGNOSTIC SUMMARY" << std::endl;
    std::cout << "========================================" << std::endl;

    int passed = 0;
    int failed = 0;
    std::vector<std::string> failedTests;

    for (size_t i = 0; i < results.size(); i++) {
        std::cout << "Test " << (i+1) << ": " << results[i].first << " - "
                  << (results[i].second ? "PASS" : "FAIL") << std::endl;
        if (results[i].second) {
            passed++;
        } else {
            failed++;
            failedTests.push_back(results[i].first);
        }
    }

    std::cout << "\nTests passed: " << passed << "/10" << std::endl;
    std::cout << "Tests failed: " << failed << "/10" << std::endl;

    if (failed > 0) {
        std::cout << "\nFAILED TESTS:" << std::endl;
        for (const auto& test : failedTests) {
            std::cout << "- " << test << std::endl;
        }

        std::cout << "\nROOT CAUSE ANALYSIS:" << std::endl;
        std::cout << "Review the detailed output above to identify specific issues." << std::endl;
        std::cout << "Common issues:" << std::endl;
        std::cout << "- Hash Consistency failures: Hash not properly saved/restored" << std::endl;
        std::cout << "- King Findable failures: King position corrupted during moves" << std::endl;
        std::cout << "- Deep Search failures: State corruption in recursive search" << std::endl;
    } else {
        std::cout << "\nALL TESTS PASSED!" << std::endl;
        std::cout << "Board state management appears to be working correctly." << std::endl;
        std::cout << "If TT hit rate is still low or NMP crashes, the issue may be:" << std::endl;
        std::cout << "- toggleTurn() not updating hash correctly" << std::endl;
        std::cout << "- Search logic error (not a board state issue)" << std::endl;
    }

    return failed > 0 ? 1 : 0;
}
