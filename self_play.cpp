#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include <sys/stat.h>
#include "board.h"
#include "moves.h"
#include "engine.h"

// Suppress ONNX Runtime warnings
#include <cstdlib>

// Check if directory exists (cross-platform)
bool directoryExists(const std::string& path) {
    struct stat info;
    if (stat(path.c_str(), &info) != 0) {
        return false;
    }
    return (info.st_mode & S_IFDIR) != 0;
}

// Enum for game results
enum class GameResult {
    HYBRID_WIN,
    CLASSICAL_WIN,
    DRAW
};

// Play a single game between two engines
GameResult playGame(bool hybridIsWhite, std::vector<std::string>& positions) {
    // Create new board and moves for this game
    Board board;
    board.loadFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
    Moves moves(&board);

    // Create engines with appropriate modes
    Engine* whiteEngine;
    Engine* blackEngine;

    if (hybridIsWhite) {
        whiteEngine = new Engine(&board, &moves, Engine::EvaluationMode::HYBRID);
        whiteEngine->setTimeLimit(2000);  // 2 seconds per move
        blackEngine = new Engine(&board, &moves, Engine::EvaluationMode::CLASSICAL);
        blackEngine->setTimeLimit(2000);  // 2 seconds per move
    } else {
        whiteEngine = new Engine(&board, &moves, Engine::EvaluationMode::CLASSICAL);
        whiteEngine->setTimeLimit(2000);  // 2 seconds per move
        blackEngine = new Engine(&board, &moves, Engine::EvaluationMode::HYBRID);
        blackEngine->setTimeLimit(2000);  // 2 seconds per move
    }

    int moveCount = 0;
    const int MAX_MOVES = 200;  // Prevent infinite games

    // Save starting position
    positions.push_back(board.toFEN());

    while (moveCount < MAX_MOVES) {
        // Check if game is over
        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        if (legalMoves.empty()) {
            // No legal moves - checkmate or stalemate
            bool isWhiteToMove = board.isWhiteToMove();

            // Check if it's checkmate by seeing if king is in check
            // If no legal moves and king is in check = checkmate
            // If no legal moves and king is not in check = stalemate
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

        // Save position after move
        positions.push_back(board.toFEN());

        // Check for draw by 50-move rule or insufficient material (simplified: just move limit)
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

    std::cout << "=== Chess Engine Self-Play System ===" << std::endl;
    std::cout << "HYBRID mode (70% classical + 30% NN) vs CLASSICAL mode (100% classical)" << std::endl;
    std::cout << std::endl;

    // Check if data/self_play directory exists
    if (!directoryExists("data/self_play")) {
        std::cerr << "ERROR: Directory 'data/self_play' does not exist!" << std::endl;
        std::cerr << "Please create the directory before running this program." << std::endl;
        return 1;
    }

    // Open output files
    std::ofstream gamesFile("data/self_play/games.txt", std::ios::app);
    std::ofstream positionsFile("data/self_play/positions.fen", std::ios::app);

    if (!gamesFile.is_open()) {
        std::cerr << "ERROR: Could not open data/self_play/games.txt for writing!" << std::endl;
        return 1;
    }

    if (!positionsFile.is_open()) {
        std::cerr << "ERROR: Could not open data/self_play/positions.fen for writing!" << std::endl;
        gamesFile.close();
        return 1;
    }

    // Statistics
    int hybridWins = 0;
    int classicalWins = 0;
    int draws = 0;
    long long totalPositions = 0;

    const int TOTAL_GAMES = 85;

    std::cout << "Starting self-play: " << TOTAL_GAMES << " games" << std::endl;
    std::cout << "Progress will be shown every 10 games" << std::endl;
    std::cout << std::endl;

    // Play games
    for (int gameNum = 1; gameNum <= TOTAL_GAMES; gameNum++) {
        // Alternate colors: odd games = hybrid is white, even games = classical is white
        bool hybridIsWhite = (gameNum % 2 == 1);

        // Storage for positions in this game
        std::vector<std::string> gamePositions;

        // Play the game
        GameResult result = playGame(hybridIsWhite, gamePositions);

        // Update statistics
        switch (result) {
            case GameResult::HYBRID_WIN:
                hybridWins++;
                break;
            case GameResult::CLASSICAL_WIN:
                classicalWins++;
                break;
            case GameResult::DRAW:
                draws++;
                break;
        }

        // Determine result string
        std::string resultStr;
        if (result == GameResult::HYBRID_WIN) {
            resultStr = "HYBRID wins";
        } else if (result == GameResult::CLASSICAL_WIN) {
            resultStr = "CLASSICAL wins";
        } else {
            resultStr = "DRAW";
        }

        // Write game result
        std::string whiteEngine = hybridIsWhite ? "HYBRID" : "CLASSICAL";
        std::string blackEngine = hybridIsWhite ? "CLASSICAL" : "HYBRID";
        gamesFile << "Game " << gameNum << ": " << resultStr
                  << " [" << whiteEngine << " vs " << blackEngine << "]" << std::endl;
        gamesFile.flush();

        // Write all positions from this game
        for (const std::string& fen : gamePositions) {
            positionsFile << fen << std::endl;
        }
        positionsFile.flush();

        totalPositions += gamePositions.size();

        // Show progress every 10 games
        if (gameNum % 10 == 0) {
            std::cout << "Progress: " << gameNum << "/" << TOTAL_GAMES << " games completed" << std::endl;
            std::cout << "  Current stats - HYBRID: " << hybridWins
                      << " wins, CLASSICAL: " << classicalWins
                      << " wins, Draws: " << draws << std::endl;
            std::cout << "  Positions saved: " << totalPositions << std::endl;
            std::cout << std::endl;
        }
    }

    // Close files
    gamesFile.close();
    positionsFile.close();

    // Print final statistics
    std::cout << "=== Self-Play Complete ===" << std::endl;
    std::cout << std::endl;
    std::cout << "STATISTICS:" << std::endl;
    std::cout << "  Total games played: " << TOTAL_GAMES << std::endl;
    std::cout << std::endl;
    std::cout << "  HYBRID mode results:" << std::endl;
    std::cout << "    Wins:   " << hybridWins << " ("
              << (100.0 * hybridWins / TOTAL_GAMES) << "%)" << std::endl;
    std::cout << "    Losses: " << classicalWins << " ("
              << (100.0 * classicalWins / TOTAL_GAMES) << "%)" << std::endl;
    std::cout << "    Draws:  " << draws << " ("
              << (100.0 * draws / TOTAL_GAMES) << "%)" << std::endl;
    std::cout << std::endl;
    std::cout << "  CLASSICAL mode results:" << std::endl;
    std::cout << "    Wins:   " << classicalWins << " ("
              << (100.0 * classicalWins / TOTAL_GAMES) << "%)" << std::endl;
    std::cout << "    Losses: " << hybridWins << " ("
              << (100.0 * hybridWins / TOTAL_GAMES) << "%)" << std::endl;
    std::cout << "    Draws:  " << draws << " ("
              << (100.0 * draws / TOTAL_GAMES) << "%)" << std::endl;
    std::cout << std::endl;
    std::cout << "  Total positions saved: " << totalPositions << std::endl;
    std::cout << std::endl;
    std::cout << "OUTPUT FILES:" << std::endl;
    std::cout << "  Game results: data/self_play/games.txt" << std::endl;
    std::cout << "  Positions:    data/self_play/positions.fen" << std::endl;
    std::cout << std::endl;

    return 0;
}
