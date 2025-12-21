#include <iostream>
#include <string>
#include <vector>
#include <cstdlib>
#include "board.h"
#include "moves.h"
#include "engine.h"

// Enum for game results
enum class GameResult {
    HYBRID_WIN,
    CLASSICAL_WIN,
    DRAW
};

// Play a single game between two engines
GameResult playGame(bool hybridIsWhite) {
    // Create new board and moves for this game
    Board board;
    board.loadFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Moves moves(&board);

    // Create engines with appropriate modes
    Engine* whiteEngine;
    Engine* blackEngine;

    if (hybridIsWhite) {
        whiteEngine = new Engine(&board, &moves, Engine::EvaluationMode::HYBRID);
        whiteEngine->setTimeLimit(1000);  // 1 second per move
        blackEngine = new Engine(&board, &moves, Engine::EvaluationMode::CLASSICAL);
        blackEngine->setTimeLimit(1000);  // 1 second per move
    } else {
        whiteEngine = new Engine(&board, &moves, Engine::EvaluationMode::CLASSICAL);
        whiteEngine->setTimeLimit(1000);  // 1 second per move
        blackEngine = new Engine(&board, &moves, Engine::EvaluationMode::HYBRID);
        blackEngine->setTimeLimit(1000);  // 1 second per move
    }

    int moveCount = 0;
    const int MAX_MOVES = 200;  // Prevent infinite games

    while (moveCount < MAX_MOVES) {
        // Check if game is over
        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        if (legalMoves.empty()) {
            // No legal moves - checkmate or stalemate
            bool isWhiteToMove = board.isWhiteToMove();
            bool isCheckmate = moves.isCheckmate();

            delete whiteEngine;
            delete blackEngine;

            if (isCheckmate) {
                // The side to move is checkmated, so the other side wins
                if (isWhiteToMove) {
                    // White is checkmated, black wins
                    return hybridIsWhite ? GameResult::CLASSICAL_WIN : GameResult::HYBRID_WIN;
                } else {
                    // Black is checkmated, white wins
                    return hybridIsWhite ? GameResult::HYBRID_WIN : GameResult::CLASSICAL_WIN;
                }
            } else {
                // Stalemate
                return GameResult::DRAW;
            }
        }

        // Get the best move from the appropriate engine
        Engine* currentEngine = board.isWhiteToMove() ? whiteEngine : blackEngine;
        std::string bestMove = currentEngine->getBestMove();

        if (bestMove.empty()) {
            // Engine couldn't find a move (shouldn't happen)
            delete whiteEngine;
            delete blackEngine;
            return GameResult::DRAW;
        }

        // Make the move
        moves.makeMove(bestMove);
        moveCount++;

        // Check for draw by move limit
        if (moveCount >= MAX_MOVES) {
            delete whiteEngine;
            delete blackEngine;
            return GameResult::DRAW;
        }
    }

    // Max moves reached
    delete whiteEngine;
    delete blackEngine;
    return GameResult::DRAW;
}

int main() {
    setenv("ORT_LOGGING_LEVEL", "3", 1);  // 3 = ERROR only, suppress warnings

    std::cout << "===================================================" << std::endl;
    std::cout << "  Quick Test: 100% Neural Network vs Classical" << std::endl;
    std::cout << "===================================================" << std::endl;
    std::cout << "Neural Network: Trained on 7,503 positions (0.35 MAE)" << std::endl;
    std::cout << "Time limit: 1 second per move" << std::endl;
    std::cout << "Maximum moves per game: 200" << std::endl;
    std::cout << std::endl;

    // Statistics
    int hybridWins = 0;
    int classicalWins = 0;
    int draws = 0;

    // Game 1: Hybrid (100% NN) as White vs Classical as Black
    std::cout << "Game 1: 100% NN (White) vs Classical (Black)" << std::endl;
    std::cout << "---------------------------------------------------" << std::endl;
    GameResult result1 = playGame(true);

    if (result1 == GameResult::HYBRID_WIN) {
        std::cout << "Result: 100% NN wins!" << std::endl;
        hybridWins++;
    } else if (result1 == GameResult::CLASSICAL_WIN) {
        std::cout << "Result: Classical wins!" << std::endl;
        classicalWins++;
    } else {
        std::cout << "Result: Draw" << std::endl;
        draws++;
    }
    std::cout << std::endl;

    // Game 2: Classical as White vs Hybrid (100% NN) as Black
    std::cout << "Game 2: Classical (White) vs 100% NN (Black)" << std::endl;
    std::cout << "---------------------------------------------------" << std::endl;
    GameResult result2 = playGame(false);

    if (result2 == GameResult::HYBRID_WIN) {
        std::cout << "Result: 100% NN wins!" << std::endl;
        hybridWins++;
    } else if (result2 == GameResult::CLASSICAL_WIN) {
        std::cout << "Result: Classical wins!" << std::endl;
        classicalWins++;
    } else {
        std::cout << "Result: Draw" << std::endl;
        draws++;
    }
    std::cout << std::endl;

    // Final summary
    std::cout << "===================================================" << std::endl;
    std::cout << "                 FINAL RESULTS" << std::endl;
    std::cout << "===================================================" << std::endl;
    std::cout << "100% Neural Network: " << hybridWins << " wins" << std::endl;
    std::cout << "Classical:           " << classicalWins << " wins" << std::endl;
    std::cout << "Draws:               " << draws << std::endl;
    std::cout << "===================================================" << std::endl;

    return 0;
}
