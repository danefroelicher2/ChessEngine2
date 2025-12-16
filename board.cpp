#include "board.h"
#include <sstream>
#include <random>

// Initialize static Zobrist tables
uint64_t Board::zobristPieces[12][8][8];
uint64_t Board::zobristBlackToMove;
uint64_t Board::zobristCastling[4];
uint64_t Board::zobristEnPassant[8];
bool Board::zobristInitialized = false;

void Board::initZobrist() {
    // Use a fixed seed for reproducibility
    std::mt19937_64 rng(0x123456789ABCDEFULL);

    // Initialize piece hashes
    for (int piece = 0; piece < 12; piece++) {
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                zobristPieces[piece][row][col] = rng();
            }
        }
    }

    // Initialize side to move hash
    zobristBlackToMove = rng();

    // Initialize castling rights hashes
    for (int i = 0; i < 4; i++) {
        zobristCastling[i] = rng();
    }

    // Initialize en passant file hashes
    for (int i = 0; i < 8; i++) {
        zobristEnPassant[i] = rng();
    }

    zobristInitialized = true;
}

int Board::getPieceIndex(char piece) const {
    // Map piece characters to indices 0-11
    // White pieces: 0-5, Black pieces: 6-11
    switch (piece) {
        case 'P': return 0;
        case 'N': return 1;
        case 'B': return 2;
        case 'R': return 3;
        case 'Q': return 4;
        case 'K': return 5;
        case 'p': return 6;
        case 'n': return 7;
        case 'b': return 8;
        case 'r': return 9;
        case 'q': return 10;
        case 'k': return 11;
        default: return -1;  // Empty square
    }
}

uint64_t Board::calculateHash() {
    uint64_t hash = 0;

    // Hash all pieces
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = squares[row][col];
            if (piece != '.') {
                int pieceIndex = getPieceIndex(piece);
                hash ^= zobristPieces[pieceIndex][row][col];
            }
        }
    }

    // Hash side to move
    if (!whiteToMove) {
        hash ^= zobristBlackToMove;
    }

    // Hash castling rights
    if (whiteCanCastleKingside) hash ^= zobristCastling[0];
    if (whiteCanCastleQueenside) hash ^= zobristCastling[1];
    if (blackCanCastleKingside) hash ^= zobristCastling[2];
    if (blackCanCastleQueenside) hash ^= zobristCastling[3];

    // Hash en passant file
    if (enPassantTargetCol >= 0 && enPassantTargetCol < 8) {
        hash ^= zobristEnPassant[enPassantTargetCol];
    }

    return hash;
}

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

    // Initialize Zobrist hashing
    if (!zobristInitialized) {
        initZobrist();
    }
    currentHash = calculateHash();
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

    // Recalculate hash after loading FEN
    currentHash = calculateHash();
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

std::string Board::toFEN() const {
    std::ostringstream fen;

    // 1. Piece placement
    for (int row = 0; row < 8; row++) {
        int emptyCount = 0;
        for (int col = 0; col < 8; col++) {
            char piece = squares[row][col];
            if (piece == '.') {
                emptyCount++;
            } else {
                if (emptyCount > 0) {
                    fen << emptyCount;
                    emptyCount = 0;
                }
                fen << piece;
            }
        }
        if (emptyCount > 0) {
            fen << emptyCount;
        }
        if (row < 7) {
            fen << '/';
        }
    }

    // 2. Side to move
    fen << (whiteToMove ? " w " : " b ");

    // 3. Castling rights
    std::string castling;
    if (whiteCanCastleKingside) castling += 'K';
    if (whiteCanCastleQueenside) castling += 'Q';
    if (blackCanCastleKingside) castling += 'k';
    if (blackCanCastleQueenside) castling += 'q';
    fen << (castling.empty() ? "-" : castling) << " ";

    // 4. En passant target square
    if (enPassantTargetRow >= 0 && enPassantTargetCol >= 0) {
        char file = 'a' + enPassantTargetCol;
        char rank = '8' - enPassantTargetRow;
        fen << file << rank;
    } else {
        fen << '-';
    }

    // 5. Halfmove clock and fullmove number (simplified - set to 0 and 1)
    fen << " 0 1";

    return fen.str();
}

void Board::updateHashForMove(int fromRow, int fromCol, int toRow, int toCol,
                               char movingPiece, char capturedPiece,
                               bool epCapture, int epCaptureRow, int epCaptureCol,
                               bool castling, int rookFromCol, int rookToCol,
                               char promotionPiece) {
    // Remove moving piece from source square
    int movingIndex = getPieceIndex(movingPiece);
    if (movingIndex >= 0) {
        currentHash ^= zobristPieces[movingIndex][fromRow][fromCol];
    }

    // Add moving piece to destination square (or promotion piece if applicable)
    char finalPiece = (promotionPiece != '.') ? promotionPiece : movingPiece;
    int finalIndex = getPieceIndex(finalPiece);
    if (finalIndex >= 0) {
        currentHash ^= zobristPieces[finalIndex][toRow][toCol];
    }

    // Remove captured piece if any
    if (capturedPiece != '.') {
        int capturedIndex = getPieceIndex(capturedPiece);
        if (capturedIndex >= 0) {
            currentHash ^= zobristPieces[capturedIndex][toRow][toCol];
        }
    }

    // Handle en passant capture (remove pawn from different square)
    if (epCapture) {
        char epPawn = whiteToMove ? 'p' : 'P';  // Opposite color pawn
        int epPawnIndex = getPieceIndex(epPawn);
        if (epPawnIndex >= 0) {
            currentHash ^= zobristPieces[epPawnIndex][epCaptureRow][epCaptureCol];
        }
    }

    // Handle castling (move the rook)
    if (castling) {
        char rook = whiteToMove ? 'R' : 'r';
        int rookIndex = getPieceIndex(rook);
        if (rookIndex >= 0) {
            currentHash ^= zobristPieces[rookIndex][fromRow][rookFromCol];  // Remove from old pos
            currentHash ^= zobristPieces[rookIndex][toRow][rookToCol];      // Add to new pos
        }
    }

    // Toggle side to move
    currentHash ^= zobristBlackToMove;
}
