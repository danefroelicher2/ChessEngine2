#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <vector>
#include "board.h"
#include "moves.h"

// Maximum search depth for killer moves and history table
const int MAX_DEPTH = 64;

// Structure to hold a move with its score for sorting
struct ScoredMove {
    std::string move;
    int score;

    // Constructor
    ScoredMove(const std::string& m, int s) : move(m), score(s) {}

    // Comparator for sorting (higher scores first)
    bool operator<(const ScoredMove& other) const {
        return score > other.score; // Descending order
    }
};

class Engine {
private:
    Board* board;
    Moves* moves;

    static const int pawnPST[8][8];
    static const int knightPST[8][8];
    static const int bishopPST[8][8];
    static const int rookPST[8][8];
    static const int queenPST[8][8];
    static const int kingMiddlegamePST[8][8];
    static const int kingEndgamePST[8][8];

    // Move ordering data structures
    std::string killerMoves[MAX_DEPTH][2];  // 2 killer moves per depth
    int historyTable[8][8][8][8];           // [fromRow][fromCol][toRow][toCol]

    // Search statistics (for debugging and analysis)
    long long nodesSearched;
    int betaCutoffs;
    int firstMoveCutoffs;

    int evaluate();
    int evaluatePawnStructure();
    int evaluateKingSafety();
    int evaluateCenterControl();
    int evaluateMobility();
    int evaluateDevelopment();
    bool isEndgame();
    int minimax(int depth, int alpha, int beta, bool maximizing);
    int getPSTValue(char piece, int row, int col, bool isWhite);
    int countPseudoLegalMoves(int row, int col, char piece);

    // Move ordering functions
    int getPieceValue(char piece);
    int scoreMove(const std::string& move, int depth);
    std::vector<ScoredMove> scoreMoves(const std::vector<std::string>& moves, int depth);
    void updateKillerMove(const std::string& move, int depth);
    void updateHistory(const std::string& move, int depth);
    void clearMoveOrdering();

    // Helper to parse move coordinates
    void parseMove(const std::string& move, int& fromRow, int& fromCol, int& toRow, int& toCol);

public:
    Engine(Board* b, Moves* m);

    std::string getBestMove();

    // Get search statistics
    long long getNodesSearched() const { return nodesSearched; }
    int getBetaCutoffs() const { return betaCutoffs; }
    int getFirstMoveCutoffs() const { return firstMoveCutoffs; }
};

#endif
