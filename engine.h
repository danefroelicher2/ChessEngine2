#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include <vector>
#include <chrono>
#include "board.h"
#include "moves.h"
#include "transposition_table.h"
#include "neural_evaluator.h"

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
public:
    // Engine evaluation mode selection
    enum class EvaluationMode {
        CLASSICAL,      // Use hand-coded evaluation only
        NEURAL,         // Use neural network only
        HYBRID,         // Combine classical (70%) + neural (30%)
        AUTO            // Use neural if available, else classical
    };

private:
    Board* board;
    Moves* moves;
    TranspositionTable transpositionTable;

    // Neural network evaluator
    NeuralEvaluator neuralEvaluator;
    bool useNeuralEval;
    EvaluationMode evaluationMode;

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

    int evaluatePawnStructure();
    int evaluateKingSafety();
    int evaluateCenterControl();
    int evaluateMobility();
    int evaluateDevelopment();
    bool isEndgame();
    int evaluateClassical();  // Classical evaluation (material + positional)
    int minimax(int depth, int alpha, int beta, bool maximizing);
    int quiescence(int alpha, int beta, int qDepth);
    int getPSTValue(char piece, int row, int col, bool isWhite);
    int countPseudoLegalMoves(int row, int col, char piece);

    // Move ordering functions
    int getPieceValue(char piece);
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

    // Static Exchange Evaluation (SEE) helpers
    char getSmallestAttacker(int targetRow, int targetCol, bool attackerColor,
                             int& fromRow, int& fromCol,
                             const std::vector<bool>& usedPieces);
    bool pieceCanAttack(int fromRow, int fromCol, int targetRow, int targetCol, char piece);

public:
    Engine(Board* b, Moves* m, EvaluationMode mode = EvaluationMode::AUTO);

    std::string getBestMove(int timeLimitMs = 1000);

    // Time management
    void setTimeLimit(int ms) { timeLimit = ms; }

    // Evaluation functions (public for diagnostic testing)
    int evaluate();
    int evaluateHybrid();

    // TEMPORARY: Expose for diagnostic testing
    int staticExchangeEvaluation(const std::string& move);
    int scoreMove(const std::string& move, int depth);

    // Get search statistics
    long long getNodesSearched() const { return nodesSearched; }
    long long getQNodesSearched() const { return qNodesSearched; }
    int getBetaCutoffs() const { return betaCutoffs; }
    int getFirstMoveCutoffs() const { return firstMoveCutoffs; }
    int getMaxQDepth() const { return maxQDepthReached; }
    double getAvgQDepth() const { return qSearches > 0 ? (double)totalQDepth / qSearches : 0.0; }
};

#endif
