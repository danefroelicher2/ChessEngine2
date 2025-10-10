#ifndef MOVES_H
#define MOVES_H

#include <string>
#include <vector>
#include "board.h"

class Moves {
private:
    Board* board;

public:
    Moves(Board* b);

    std::vector<std::string> generateLegalMoves();
    bool isLegalMove(const std::string& move);
    void makeMove(const std::string& move);
};

#endif
