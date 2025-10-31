#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <vector>
#include <chrono>
#include "board.h"
#include "moves.h"
#include "transposition_table.h"

// Performance diagnostic flag
// Set to 1 for fast tactical search, 0 for full positional evaluation
#define FAST_EVAL_MODE 1

// Maximum search depth for killer moves and history table
const int MAX_DEPTH = 64;
const int MAX_SEARCH_DEPTH = 20;  // Maximum iterative deepening depth

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
    TranspositionTable transpositionTable;

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
    long long qNodesSearched;       // Nodes searched in quiescence
    int betaCutoffs;
    int firstMoveCutoffs;
    int maxQDepthReached;           // Maximum quiescence depth reached
    long long totalQDepth;          // Sum of all quiescence depths (for average)
    long long qSearches;            // Number of quiescence searches
    long long lmrReductions;        // Number of moves searched with reduced depth
    long long lmrReSearches;        // Number of re-searches after LMR failed high

    // Time management
    std::chrono::steady_clock::time_point searchStartTime;
    int timeLimit;                  // Time limit in milliseconds
    std::string pvMove;             // Principal variation move from previous iteration

    int evaluate();
    int evaluatePawnStructure();
    int evaluateKingSafety();
    int evaluateCenterControl();
    int evaluateMobility();
    int evaluateDevelopment();
    bool isEndgame();
    int minimax(int depth, int alpha, int beta, bool maximizing);
    int quiescence(int alpha, int beta, int qDepth);
    int getPSTValue(char piece, int row, int col, bool isWhite);
    int countPseudoLegalMoves(int row, int col, char piece);

    // Move ordering functions
    int getPieceValue(char piece);
    int scoreMove(const std::string& move, int depth);
    std::vector<ScoredMove> scoreMoves(const std::vector<std::string>& moves, int depth, TTEntry* ttEntry = nullptr);
    void updateKillerMove(const std::string& move, int depth);
    void updateHistory(const std::string& move, int depth);
    void clearMoveOrdering();

    // Quiescence search helpers
    std::vector<std::string> generateTacticalMoves();

    // Time management
    bool isTimeUp();

    // Iterative deepening search
    std::string searchAtDepth(int depth, int& outScore);

    // Helper to parse move coordinates
    void parseMove(const std::string& move, int& fromRow, int& fromCol, int& toRow, int& toCol);

    // Late Move Reduction helpers
    bool isCapture(const std::string& move);

public:
    Engine(Board* b, Moves* m);

    std::string getBestMove(int timeLimitMs = 5000);

    // Get search statistics
    long long getNodesSearched() const { return nodesSearched; }
    long long getQNodesSearched() const { return qNodesSearched; }
    int getBetaCutoffs() const { return betaCutoffs; }
    int getFirstMoveCutoffs() const { return firstMoveCutoffs; }
    int getMaxQDepth() const { return maxQDepthReached; }
    double getAvgQDepth() const { return qSearches > 0 ? (double)totalQDepth / qSearches : 0.0; }
};

#endif
