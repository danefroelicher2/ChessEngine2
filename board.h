#ifndef BOARD_H
#define BOARD_H

#include <string>
#include <cstdint>

class Board {
private:
    char squares[8][8];
    bool whiteToMove;

    // Castling rights
    bool whiteCanCastleKingside;
    bool whiteCanCastleQueenside;
    bool blackCanCastleKingside;
    bool blackCanCastleQueenside;

    // En passant target square
    int enPassantTargetRow;  // -1 if no en passant available
    int enPassantTargetCol;  // -1 if no en passant available

    // Zobrist hashing
    uint64_t currentHash;

    // Static Zobrist tables (initialized once)
    static uint64_t zobristPieces[12][8][8];  // 12 piece types, 8x8 board
    static uint64_t zobristBlackToMove;
    static uint64_t zobristCastling[4];  // WK, WQ, BK, BQ
    static uint64_t zobristEnPassant[8]; // For each file
    static bool zobristInitialized;

    // Zobrist helper functions
    static void initZobrist();
    uint64_t calculateHash();
    int getPieceIndex(char piece) const;

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

    // En passant getters/setters
    int getEnPassantRow() const { return enPassantTargetRow; }
    int getEnPassantCol() const { return enPassantTargetCol; }
    void setEnPassantTarget(int row, int col) {
        enPassantTargetRow = row;
        enPassantTargetCol = col;
    }
    void clearEnPassantTarget() {
        enPassantTargetRow = -1;
        enPassantTargetCol = -1;
    }

    std::string toString() const;

    // Zobrist hashing
    uint64_t getHash() const { return currentHash; }
    void setHash(uint64_t hash) { currentHash = hash; }
    void recalculateHash() { currentHash = calculateHash(); }
    void updateHashForMove(int fromRow, int fromCol, int toRow, int toCol,
                          char movingPiece, char capturedPiece,
                          bool epCapture, int epCaptureRow, int epCaptureCol,
                          bool castling, int rookFromCol, int rookToCol,
                          char promotionPiece);

    // Toggle which side is to move (for null move pruning)
    void toggleTurn();
};

#endif
