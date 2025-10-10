#include "engine.h"
#include <cctype>
#include <limits>
#include <algorithm>

Engine::Engine(Board* b, Moves* m) : board(b), moves(m) {}

int Engine::evaluate() {
    int score = 0;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = board->getPiece(row, col);
            if (piece == '.') continue;

            int value = 0;
            char type = tolower(piece);

            switch (type) {
                case 'p': value = 100; break;
                case 'n': value = 320; break;
                case 'b': value = 330; break;
                case 'r': value = 500; break;
                case 'q': value = 900; break;
                case 'k': value = 20000; break;
            }

            if (isupper(piece)) {
                score += value;
            } else {
                score -= value;
            }
        }
    }

    return score;
}

int Engine::minimax(int depth, int alpha, int beta, bool maximizing) {
    if (depth == 0) {
        return evaluate();
    }

    std::vector<std::string> legalMoves = moves->generateLegalMoves();

    if (legalMoves.empty()) {
        // No moves available (game over)
        return maximizing ? -999999 : 999999;
    }

    if (maximizing) {
        int maxEval = std::numeric_limits<int>::min();
        for (const std::string& move : legalMoves) {
            // Make move
            MoveInfo info = moves->makeMoveWithInfo(move);

            int eval = minimax(depth - 1, alpha, beta, false);

            // Undo move
            moves->unmakeMove(move, info);

            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (const std::string& move : legalMoves) {
            // Make move
            MoveInfo info = moves->makeMoveWithInfo(move);

            int eval = minimax(depth - 1, alpha, beta, true);

            // Undo move
            moves->unmakeMove(move, info);

            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return minEval;
    }
}

std::string Engine::getBestMove() {
    std::vector<std::string> legalMoves = moves->generateLegalMoves();

    if (legalMoves.empty()) {
        return "";
    }

    std::string bestMove = legalMoves[0];
    int bestScore = board->isWhiteToMove() ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

    for (const std::string& move : legalMoves) {
        // Make move
        MoveInfo info = moves->makeMoveWithInfo(move);

        int score = minimax(3, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), !board->isWhiteToMove());

        // Undo move
        moves->unmakeMove(move, info);

        if (board->isWhiteToMove()) {
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        } else {
            if (score < bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }
    }

    return bestMove;
}
