#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include "board.h"
#include "moves.h"

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

    int evaluate();
    int evaluatePawnStructure();
    int evaluateKingSafety();
    int evaluateCenterControl();
    int evaluateMobility();
    bool isEndgame();
    int minimax(int depth, int alpha, int beta, bool maximizing);
    int getPSTValue(char piece, int row, int col, bool isWhite);
    int countPseudoLegalMoves(int row, int col, char piece);

public:
    Engine(Board* b, Moves* m);

    std::string getBestMove();
};

#endif
