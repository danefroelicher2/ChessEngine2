#include <iostream>
#include "board.h"
#include "moves.h"
#include "engine.h"

int main() {
    Board board;
    Moves moves(&board);
    Engine engine(&board, &moves);

    std::cout << "Chess Engine - Simple Implementation\n";
    std::cout << "=====================================\n\n";

    std::cout << "Starting position:\n";
    std::cout << board.toString() << "\n";

    // Generate and display legal moves
    std::vector<std::string> legalMoves = moves.generateLegalMoves();
    std::cout << "Number of legal moves: " << legalMoves.size() << "\n";
    std::cout << "First 10 legal moves: ";
    for (int i = 0; i < 10 && i < legalMoves.size(); i++) {
        std::cout << legalMoves[i] << " ";
    }
    std::cout << "\n\n";

    // Get best move
    std::cout << "Calculating best move (depth 3)...\n";
    std::string bestMove = engine.getBestMove();
    std::cout << "Best move: " << bestMove << "\n\n";

    // Make the move
    moves.makeMove(bestMove);
    std::cout << "After move " << bestMove << ":\n";
    std::cout << board.toString() << "\n";

    // Test FEN loading
    std::cout << "Testing FEN loading...\n";
    board.loadFromFEN("rnbqkbnr/pppppppp/8/8/4P3/8/PPPP1PPP/RNBQKBNR b KQkq e3 0 1");
    std::cout << "After loading FEN (1.e4):\n";
    std::cout << board.toString() << "\n";

    std::cout << "Chess engine initialized successfully!\n";

    return 0;
}
