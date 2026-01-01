#include "board.h"
#include "moves.h"
#include <iostream>

int main() {
    std::cout << "=== MINIMAL PAWN ATTACK FIX TEST ===\n\n";

    // Test: Black pawn on g3 protects f2
    // King on e1 should NOT be able to capture f2
    Board board;
    board.loadFromFEN("4k3/8/8/6p1/8/8/5P2/4K3 w - - 0 1");

    std::cout << "Position: White king e1, White pawn f2, Black pawn g3\n";
    std::cout << "Board:\n" << board.toString() << "\n";

    Moves moves(&board);

    // Test if f2 (row=6, col=5) is attacked by black
    // Black pawn is on g3 (row=5, col=6)
    // The pawn should be attacking f2
    bool f2Attacked = moves.isSquareAttacked(6, 5, false);

    std::cout << "\nIs f2 attacked by black? " << (f2Attacked ? "YES" : "NO") << "\n";
    std::cout << "Expected: NO (f2 has white's own pawn)\n\n";

    // Better test: empty square that IS attacked
    Board board2;
    board2.loadFromFEN("4k3/8/8/6p1/5P2/8/8/4K3 w - - 0 1");

    std::cout << "Position 2: Black pawn g5, checking if f4 is attacked\n";
    std::cout << "Board:\n" << board2.toString() << "\n";

    Moves moves2(&board2);
    bool f4Attacked = moves2.isSquareAttacked(4, 5, false);

    std::cout << "\nIs f4 (row=4, col=5) attacked by black pawn on g5? " << (f4Attacked ? "YES" : "NO") << "\n";
    std::cout << "Expected: YES\n\n";

    if (f4Attacked) {
        std::cout << "✓ FIX WORKING! Pawn attacks correctly detected!\n";
    } else {
        std::cout << "✗ BUG STILL EXISTS! Pawn attacks not detected!\n";
    }

    return 0;
}
