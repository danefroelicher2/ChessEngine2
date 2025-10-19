#include "board.h"
#include "moves.h"
#include "engine.h"
#include <iostream>

int main() {
    std::cout << "=== Quick Recapture Test ===" << std::endl;

    // Test position: Black queen on d1, White queen on e2 can recapture
    Board board;
    board.loadFromFEN("rnb1kbnr/pppppppp/8/8/8/8/PPPPQPPP/RNBqKBNR w KQkq - 0 1");

    std::cout << "Position (Black queen on d1):" << std::endl;
    std::cout << board.toString() << std::endl;

    Moves moves(&board);
    Engine engine(&board, &moves);

    std::cout << "\nSearching for best move..." << std::endl;
    std::string bestMove = engine.getBestMove(1000);  // 1 second

    std::cout << "\nResult: " << bestMove << std::endl;

    if (bestMove == "e2d1") {
        std::cout << "✓ SUCCESS: Engine recaptured the queen!" << std::endl;
        return 0;
    } else {
        std::cout << "✗ FAILED: Engine did NOT recapture. Chose: " << bestMove << std::endl;
        return 1;
    }
}
