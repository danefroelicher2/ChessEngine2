#include "board.h"
#include "moves.h"
#include "engine.h"
#include <iostream>

int main() {
    std::cout << "Simple Iterative Deepening Test\n\n";

    // Test with starting position and 2 second time limit
    Board board;
    board.loadFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");

    Moves moves(&board);
    Engine engine(&board, &moves);

    std::cout << "Searching with 2000ms time limit...\n\n";
    std::string bestMove = engine.getBestMove(2000);

    std::cout << "\n=== FINAL RESULTS ===\n";
    std::cout << "Best Move: " << bestMove << "\n";
    std::cout << "Total Nodes: " << engine.getNodesSearched() << "\n";

    return 0;
}
