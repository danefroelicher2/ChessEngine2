#ifndef BOARD_H
#define BOARD_H

#include <string>

class Board {
private:
    char squares[8][8];
    bool whiteToMove;

    // Castling rights
    bool whiteCanCastleKingside;
    bool whiteCanCastleQueenside;
    bool blackCanCastleKingside;
    bool blackCanCastleQueenside;

public:
    Board();

    char getPiece(int row, int col) const;
    void setPiece(int row, int col, char piece);
    void loadFromFEN(const std::string& fen);

    bool isWhiteToMove() const { return whiteToMove; }
    void setWhiteToMove(bool white) { whiteToMove = white; }

    // Castling rights getters
    bool canCastleKingside(bool white) const {
        return white ? whiteCanCastleKingside : blackCanCastleKingside;
    }
    bool canCastleQueenside(bool white) const {
        return white ? whiteCanCastleQueenside : blackCanCastleQueenside;
    }

    // Castling rights setters
    void setCastleKingside(bool white, bool canCastle) {
        if (white) whiteCanCastleKingside = canCastle;
        else blackCanCastleKingside = canCastle;
    }
    void setCastleQueenside(bool white, bool canCastle) {
        if (white) whiteCanCastleQueenside = canCastle;
        else blackCanCastleQueenside = canCastle;
    }

    std::string toString() const;
};

#endif
