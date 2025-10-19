#include <iostream>
#include "board.h"
#include "moves.h"
#include "engine.h"

int main() {
    std::cout << "Chess Engine - Simple Test\n";
    std::cout << "==============================\n\n";

    Board board;
    Moves moves(&board);
    Engine engine(&board, &moves);

    std::cout << "Starting position:\n";
    std::cout << board.toString() << "\n";

    // Get best move
    std::cout << "Calculating best move...\n";
    std::string bestMove = engine.getBestMove(3000);
    std::cout << "Best move: " << bestMove << "\n\n";

    std::cout << "Chess engine test complete!\n";

    return 0;
}
