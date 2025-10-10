#ifndef ENGINE_H
#define ENGINE_H

#include <string>
#include "board.h"
#include "moves.h"

class Engine {
private:
    Board* board;
    Moves* moves;

    int evaluate();
    int minimax(int depth, int alpha, int beta, bool maximizing);

public:
    Engine(Board* b, Moves* m);

    std::string getBestMove();
};

#endif
