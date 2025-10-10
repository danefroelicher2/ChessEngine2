#ifndef BOARD_H
#define BOARD_H

#include <string>

class Board {
private:
    char squares[8][8];
    bool whiteToMove;

public:
    Board();

    char getPiece(int row, int col) const;
    void setPiece(int row, int col, char piece);
    void loadFromFEN(const std::string& fen);

    bool isWhiteToMove() const { return whiteToMove; }
    void setWhiteToMove(bool white) { whiteToMove = white; }

    std::string toString() const;
};

#endif
