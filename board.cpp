#include "board.h"
#include <sstream>

Board::Board() {
    // Initialize empty board
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            squares[row][col] = '.';
        }
    }

    // Set up starting position
    // Row 0 is rank 8, Row 7 is rank 1 (white's side)
    squares[0][0] = 'r'; squares[0][1] = 'n'; squares[0][2] = 'b'; squares[0][3] = 'q';
    squares[0][4] = 'k'; squares[0][5] = 'b'; squares[0][6] = 'n'; squares[0][7] = 'r';

    for (int col = 0; col < 8; col++) {
        squares[1][col] = 'p'; // Black pawns
        squares[6][col] = 'P'; // White pawns
    }

    squares[7][0] = 'R'; squares[7][1] = 'N'; squares[7][2] = 'B'; squares[7][3] = 'Q';
    squares[7][4] = 'K'; squares[7][5] = 'B'; squares[7][6] = 'N'; squares[7][7] = 'R';

    whiteToMove = true;

    // Initialize castling rights (all allowed at start)
    whiteCanCastleKingside = true;
    whiteCanCastleQueenside = true;
    blackCanCastleKingside = true;
    blackCanCastleQueenside = true;

    // Initialize en passant (no target at start)
    enPassantTargetRow = -1;
    enPassantTargetCol = -1;
}

char Board::getPiece(int row, int col) const {
    if (row < 0 || row >= 8 || col < 0 || col >= 8) {
        return '.';
    }
    return squares[row][col];
}

void Board::setPiece(int row, int col, char piece) {
    if (row >= 0 && row < 8 && col >= 0 && col < 8) {
        squares[row][col] = piece;
    }
}

void Board::loadFromFEN(const std::string& fen) {
    // Clear board
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            squares[row][col] = '.';
        }
    }

    std::istringstream iss(fen);
    std::string position;
    iss >> position;

    int row = 0, col = 0;
    for (char c : position) {
        if (c == '/') {
            row++;
            col = 0;
        } else if (c >= '1' && c <= '8') {
            col += (c - '0');
        } else {
            squares[row][col] = c;
            col++;
        }
    }

    // Read side to move
    char side;
    iss >> side;
    whiteToMove = (side == 'w');

    // Read castling rights (e.g., "KQkq" or "-")
    std::string castlingRights;
    iss >> castlingRights;

    // Initialize all castling rights to false
    whiteCanCastleKingside = false;
    whiteCanCastleQueenside = false;
    blackCanCastleKingside = false;
    blackCanCastleQueenside = false;

    // Parse castling rights
    if (castlingRights != "-") {
        for (char c : castlingRights) {
            if (c == 'K') whiteCanCastleKingside = true;
            if (c == 'Q') whiteCanCastleQueenside = true;
            if (c == 'k') blackCanCastleKingside = true;
            if (c == 'q') blackCanCastleQueenside = true;
        }
    }

    // Read en passant target square (e.g., "e3" or "-")
    std::string enPassantSquare;
    iss >> enPassantSquare;

    if (enPassantSquare != "-" && enPassantSquare.length() == 2) {
        // Parse algebraic notation (e.g., "e3")
        enPassantTargetCol = enPassantSquare[0] - 'a';
        enPassantTargetRow = 8 - (enPassantSquare[1] - '0');
    } else {
        enPassantTargetRow = -1;
        enPassantTargetCol = -1;
    }
}

std::string Board::toString() const {
    std::string result;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            result += squares[row][col];
            result += ' ';
        }
        result += '\n';
    }
    return result;
}
