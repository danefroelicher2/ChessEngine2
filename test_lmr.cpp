#include "board.h"
#include "moves.h"
#include "engine.h"
#include <iostream>

int main() {
    std::cout << "=== LMR Implementation Test ===\n\n";

    // Create board in starting position
    Board board;
    Moves moves(&board);
    Engine engine(&board, &moves);

    std::cout << "Initial position:\n";
    std::cout << board.toString() << "\n";

    // Run engine search with LMR
    std::cout << "Running search with LMR enabled...\n";
    std::string bestMove = engine.getBestMove(5000);  // 5 second search

    std::cout << "\nBest move found: " << bestMove << "\n";
    std::cout << "\n=== Test completed successfully ===\n";

    return 0;
}
