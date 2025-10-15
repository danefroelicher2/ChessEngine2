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

    // ========================================
    // TEST: Can engine generate checking moves?
    // ========================================
    std::cout << "\n========================================\n";
    std::cout << "TEST: Checking move generation\n";
    std::cout << "========================================\n\n";

    // Position after 1.e4 e5 - White to move
    std::cout << "Loading position after 1.e4 e5 (White to move):\n";
    board.loadFromFEN("rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR w KQkq - 0 2");
    std::cout << board.toString() << "\n";

    // Generate all legal moves
    legalMoves = moves.generateLegalMoves();
    std::cout << "Total legal moves for White: " << legalMoves.size() << "\n\n";

    // Check if d1h5 (Queen to h5, checking the king) is in the list
    bool foundQh5 = false;
    std::cout << "Looking for d1h5 (Qh5+ - checks Black king)...\n";
    for (const std::string& move : legalMoves) {
        if (move == "d1h5") {
            foundQh5 = true;
            std::cout << "✓ FOUND: d1h5 (Qh5+) is in legal moves!\n";
            break;
        }
    }

    if (!foundQh5) {
        std::cout << "✗ NOT FOUND: d1h5 (Qh5+) is NOT in legal moves!\n";
        std::cout << "This is a BUG - checking moves are being filtered out!\n";
    }

    // Print all Queen moves
    std::cout << "\nAll Queen moves from d1:\n";
    for (const std::string& move : legalMoves) {
        if (move.substr(0, 2) == "d1") {
            std::cout << "  " << move << "\n";
        }
    }

    // Print all moves (for debugging)
    std::cout << "\nALL legal moves:\n";
    for (size_t i = 0; i < legalMoves.size(); i++) {
        std::cout << legalMoves[i];
        if ((i + 1) % 10 == 0) {
            std::cout << "\n";
        } else if (i + 1 < legalMoves.size()) {
            std::cout << " ";
        }
    }
    std::cout << "\n\n========================================\n";

    std::cout << "Chess engine initialized successfully!\n";

    return 0;
}
