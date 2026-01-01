#include "board.h"
#include "moves.h"
#include <iostream>
#include <vector>

int main() {
    std::cout << "========================================\n";
    std::cout << "KING ILLEGAL CAPTURE BUG TEST\n";
    std::cout << "========================================\n\n";

    // Test 1: King tries to capture pawn protected by queen (diagonal)
    std::cout << "TEST 1: King captures pawn protected by queen (diagonal)\n";
    std::cout << "Position: White king e1, Black pawn f2, Black queen h4\n";
    std::cout << "Expected: King CANNOT capture f2 pawn (protected by h4 queen)\n\n";

    Board board1;
    // Set up the position manually
    board1.loadFromFEN("rnb1kbnr/pppppppp/8/8/7q/8/PPPPPPPPPPPPPPPPP/RNBQKBNR w KQkq - 0 1");

    // Wait, that FEN is wrong. Let me build it properly:
    // Rank 8: rnbqkbnr (black back rank, but we want a simpler position)
    // Rank 7: pppppppp
    // Rank 6: empty
    // Rank 5: empty
    // Rank 4: black queen on h4
    // Rank 3: empty
    // Rank 2: white pawns + black pawn on f2
    // Rank 1: white pieces with king on e1

    // Actually, let's create a minimal test position:
    // Just king, pawn, and queen
    board1.loadFromFEN("4k3/8/8/8/7q/8/5p2/4K3 w - - 0 1");

    std::cout << "Board:\n" << board1.toString() << "\n";

    Moves moves1(&board1);
    std::vector<std::string> legalMoves1 = moves1.generateLegalMoves();

    std::cout << "Legal moves generated: " << legalMoves1.size() << "\n";

    bool foundIllegal = false;
    for (const auto& move : legalMoves1) {
        std::cout << "  " << move << "\n";
        if (move == "e1f2") {
            foundIllegal = true;
            std::cout << "  ^^^ ILLEGAL MOVE FOUND! King capturing protected pawn!\n";
        }
    }

    if (foundIllegal) {
        std::cout << "\nResult: ✗ FAIL - Illegal move e1f2 was generated!\n\n";
    } else {
        std::cout << "\nResult: ✓ PASS - Illegal move was correctly filtered out\n\n";
    }

    // Test 2: King tries to capture knight protected by rook (straight line)
    std::cout << "\n========================================\n";
    std::cout << "TEST 2: King captures knight protected by rook (straight)\n";
    std::cout << "Position: White king e4, Black knight e5, Black rook e8\n";
    std::cout << "Expected: King CANNOT capture e5 knight (protected by e8 rook)\n\n";

    Board board2;
    board2.loadFromFEN("4r3/8/8/4n3/4K3/8/8/8 w - - 0 1");

    std::cout << "Board:\n" << board2.toString() << "\n";

    Moves moves2(&board2);
    std::vector<std::string> legalMoves2 = moves2.generateLegalMoves();

    std::cout << "Legal moves generated: " << legalMoves2.size() << "\n";

    foundIllegal = false;
    for (const auto& move : legalMoves2) {
        std::cout << "  " << move << "\n";
        if (move == "e4e5") {
            foundIllegal = true;
            std::cout << "  ^^^ ILLEGAL MOVE FOUND! King capturing protected knight!\n";
        }
    }

    if (foundIllegal) {
        std::cout << "\nResult: ✗ FAIL - Illegal move e4e5 was generated!\n\n";
    } else {
        std::cout << "\nResult: ✓ PASS - Illegal move was correctly filtered out\n\n";
    }

    // Test 3: King tries to capture pawn protected by pawn
    std::cout << "\n========================================\n";
    std::cout << "TEST 3: King captures pawn protected by pawn\n";
    std::cout << "Position: White king e3, Black pawn f4, Black pawn g5\n";
    std::cout << "Expected: King CANNOT capture f4 pawn (protected by g5 pawn)\n\n";

    Board board3;
    board3.loadFromFEN("4k3/8/8/6p1/5p2/4K3/8/8 w - - 0 1");

    std::cout << "Board:\n" << board3.toString() << "\n";

    Moves moves3(&board3);
    std::vector<std::string> legalMoves3 = moves3.generateLegalMoves();

    std::cout << "Legal moves generated: " << legalMoves3.size() << "\n";

    foundIllegal = false;
    for (const auto& move : legalMoves3) {
        std::cout << "  " << move << "\n";
        if (move == "e3f4") {
            foundIllegal = true;
            std::cout << "  ^^^ ILLEGAL MOVE FOUND! King capturing protected pawn!\n";
        }
    }

    if (foundIllegal) {
        std::cout << "\nResult: ✗ FAIL - Illegal move e3f4 was generated!\n\n";
    } else {
        std::cout << "\nResult: ✓ PASS - Illegal move was correctly filtered out\n\n";
    }

    // Test 4: Manual square attack check
    std::cout << "\n========================================\n";
    std::cout << "TEST 4: Direct isSquareAttacked() test\n";
    std::cout << "========================================\n";

    Board board4;
    board4.loadFromFEN("4k3/8/8/8/7q/8/5p2/4K3 w - - 0 1");
    Moves moves4(&board4);

    // Check if f2 is attacked by black
    bool f2Attacked = moves4.isSquareAttacked(6, 5, false); // f2 = row 6, col 5
    std::cout << "Is f2 (row=6, col=5) attacked by black? " << (f2Attacked ? "YES" : "NO") << "\n";
    std::cout << "Expected: YES (queen on h4 attacks f2 diagonally)\n";

    if (f2Attacked) {
        std::cout << "Result: ✓ PASS - Attack detection working\n";
    } else {
        std::cout << "Result: ✗ FAIL - Attack detection NOT working!\n";
        std::cout << "\nThis is likely the root cause of the bug!\n";
    }

    std::cout << "\n========================================\n";
    std::cout << "END OF TESTS\n";
    std::cout << "========================================\n";

    return 0;
}
