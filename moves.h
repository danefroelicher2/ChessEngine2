#ifndef MOVES_H
#define MOVES_H

#include <string>
#include <vector>
#include <cstdint>
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

    // En passant state (for undo)
    int previousEnPassantRow;  // For undo
    int previousEnPassantCol;  // For undo
    bool wasEnPassantCapture;  // Was this move an en passant?
    int enPassantCapturedRow;  // Row of captured pawn (for undo)
    int enPassantCapturedCol;  // Col of captured pawn (for undo)

    // Zobrist hash (for undo)
    uint64_t previousHash;
};

class Moves {
private:
    Board* board;

    bool isKingInCheck();

public:
    Moves(Board* b);

    std::vector<std::string> generateLegalMoves();
    bool isLegalMove(const std::string& move);
    void makeMove(const std::string& move);
    MoveInfo makeMoveWithInfo(const std::string& move);
    void unmakeMove(const std::string& move, const MoveInfo& info);

    // Game state detection
    bool isCheckmate();
    bool isStalemate();

    // Check if the specified side is in check
    bool isInCheck(bool isWhite);

    // Square control (for evaluation)
    bool isSquareAttacked(int row, int col, bool byWhite);
};

#endif
