#include <iostream>
#include "board.h"
#include "moves.h"
#include "engine.h"

int main() {
    std::cout << "=== Transposition Table Test ===" << std::endl;

    // Create board and set up starting position
    Board board;
    Moves moves(&board);
    Engine engine(&board, &moves);

    std::cout << "\nSearching starting position twice..." << std::endl;
    std::cout << "\n--- FIRST SEARCH ---" << std::endl;
    std::string move1 = engine.getBestMove(2000);  // 2 second search

    std::cout << "\n--- SECOND SEARCH (should show TT hits) ---" << std::endl;
    std::string move2 = engine.getBestMove(2000);  // Same search - should hit TT

    if (move1 == move2) {
        std::cout << "\n✓ SUCCESS: Both searches returned same move: " << move1 << std::endl;
    } else {
        std::cout << "\n✗ WARNING: Moves differ - " << move1 << " vs " << move2 << std::endl;
    }

    return 0;
}
