#ifndef MOVES_H
#define MOVES_H

#include <string>
#include <vector>
#include "board.h"

struct MoveInfo {
    char capturedPiece;
    bool wasWhiteToMove;
    char promotedFrom;  // Original piece before promotion ('P'/'p' for promotions, '.' otherwise)

    // Castling rights before the move (for undo)
    bool whiteKingside;
    bool whiteQueenside;
    bool blackKingside;
    bool blackQueenside;
};

class Moves {
private:
    Board* board;

    bool isSquareAttacked(int row, int col, bool byWhite);
    bool isKingInCheck();

public:
    Moves(Board* b);

    std::vector<std::string> generateLegalMoves();
    bool isLegalMove(const std::string& move);
    void makeMove(const std::string& move);
    MoveInfo makeMoveWithInfo(const std::string& move);
    void unmakeMove(const std::string& move, const MoveInfo& info);
};

#endif
