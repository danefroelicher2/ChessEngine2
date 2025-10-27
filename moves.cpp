#include "moves.h"
#include <cctype>

Moves::Moves(Board* b) : board(b) {}

std::vector<std::string> Moves::generateLegalMoves() {
    std::vector<std::string> moves;

    for (int fromRow = 0; fromRow < 8; fromRow++) {
        for (int fromCol = 0; fromCol < 8; fromCol++) {
            char piece = board->getPiece(fromRow, fromCol);
            if (piece == '.') continue;

            bool isWhite = isupper(piece);
            if (isWhite != board->isWhiteToMove()) continue;

            char pieceType = tolower(piece);

            // Generate moves based on piece type
            if (pieceType == 'p') {
                // Pawn moves
                int direction = isWhite ? -1 : 1;
                int startRow = isWhite ? 6 : 1;

                // Move forward one square
                int toRow = fromRow + direction;
                if (toRow >= 0 && toRow < 8 && board->getPiece(toRow, fromCol) == '.') {
                    std::string move;
                    move += char('a' + fromCol);
                    move += char('1' + (7 - fromRow));
                    move += char('a' + fromCol);
                    move += char('1' + (7 - toRow));

                    // Check for promotion (white pawn to row 0, black pawn to row 7)
                    if ((isWhite && toRow == 0) || (!isWhite && toRow == 7)) {
                        // Generate all 4 promotion moves
                        moves.push_back(move + 'q');
                        moves.push_back(move + 'r');
                        moves.push_back(move + 'b');
                        moves.push_back(move + 'n');
                    } else {
                        moves.push_back(move);
                    }

                    // Move forward two squares from starting position
                    if (fromRow == startRow) {
                        toRow = fromRow + 2 * direction;
                        if (board->getPiece(toRow, fromCol) == '.') {
                            move.clear();
                            move += char('a' + fromCol);
                            move += char('1' + (7 - fromRow));
                            move += char('a' + fromCol);
                            move += char('1' + (7 - toRow));
                            moves.push_back(move);
                        }
                    }
                }

                // Captures
                toRow = fromRow + direction;
                for (int toCol : {fromCol - 1, fromCol + 1}) {
                    if (toCol >= 0 && toCol < 8 && toRow >= 0 && toRow < 8) {
                        char target = board->getPiece(toRow, toCol);
                        if (target != '.' && isupper(target) != isWhite) {
                            std::string move;
                            move += char('a' + fromCol);
                            move += char('1' + (7 - fromRow));
                            move += char('a' + toCol);
                            move += char('1' + (7 - toRow));

                            // Check for promotion (white pawn to row 0, black pawn to row 7)
                            if ((isWhite && toRow == 0) || (!isWhite && toRow == 7)) {
                                // Generate all 4 promotion moves
                                moves.push_back(move + 'q');
                                moves.push_back(move + 'r');
                                moves.push_back(move + 'b');
                                moves.push_back(move + 'n');
                            } else {
                                moves.push_back(move);
                            }
                        }
                    }
                }

                // En passant captures
                int epRow = board->getEnPassantRow();
                int epCol = board->getEnPassantCol();

                if (epRow != -1 && epCol != -1) {
                    // Check if we have a pawn that can capture en passant
                    // The pawn must be on the correct rank (rank 5 for white, rank 4 for black)
                    int correctRank = isWhite ? 3 : 4;  // Row 3 = rank 5, Row 4 = rank 4

                    if (fromRow == correctRank) {
                        // Check if en passant target is diagonally adjacent
                        if (epRow == fromRow + direction && abs(epCol - fromCol) == 1) {
                            // This pawn can capture en passant!
                            std::string move;
                            move += char('a' + fromCol);
                            move += char('1' + (7 - fromRow));
                            move += char('a' + epCol);
                            move += char('1' + (7 - epRow));
                            moves.push_back(move);
                        }
                    }
                }
            } else if (pieceType == 'n') {
                // Knight moves
                int deltas[8][2] = {{-2, -1}, {-2, 1}, {-1, -2}, {-1, 2}, {1, -2}, {1, 2}, {2, -1}, {2, 1}};
                for (auto& delta : deltas) {
                    int toRow = fromRow + delta[0];
                    int toCol = fromCol + delta[1];
                    if (toRow >= 0 && toRow < 8 && toCol >= 0 && toCol < 8) {
                        char target = board->getPiece(toRow, toCol);
                        if (target == '.' || isupper(target) != isWhite) {
                            std::string move;
                            move += char('a' + fromCol);
                            move += char('1' + (7 - fromRow));
                            move += char('a' + toCol);
                            move += char('1' + (7 - toRow));
                            moves.push_back(move);
                        }
                    }
                }
            } else if (pieceType == 'b') {
                // Bishop moves (diagonals)
                int directions[4][2] = {{-1, -1}, {-1, 1}, {1, -1}, {1, 1}};
                for (auto& dir : directions) {
                    for (int dist = 1; dist < 8; dist++) {
                        int toRow = fromRow + dir[0] * dist;
                        int toCol = fromCol + dir[1] * dist;
                        if (toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8) break;

                        char target = board->getPiece(toRow, toCol);
                        if (target == '.') {
                            std::string move;
                            move += char('a' + fromCol);
                            move += char('1' + (7 - fromRow));
                            move += char('a' + toCol);
                            move += char('1' + (7 - toRow));
                            moves.push_back(move);
                        } else {
                            if (isupper(target) != isWhite) {
                                std::string move;
                                move += char('a' + fromCol);
                                move += char('1' + (7 - fromRow));
                                move += char('a' + toCol);
                                move += char('1' + (7 - toRow));
                                moves.push_back(move);
                            }
                            break;
                        }
                    }
                }
            } else if (pieceType == 'r') {
                // Rook moves (straight lines)
                int directions[4][2] = {{-1, 0}, {1, 0}, {0, -1}, {0, 1}};
                for (auto& dir : directions) {
                    for (int dist = 1; dist < 8; dist++) {
                        int toRow = fromRow + dir[0] * dist;
                        int toCol = fromCol + dir[1] * dist;
                        if (toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8) break;

                        char target = board->getPiece(toRow, toCol);
                        if (target == '.') {
                            std::string move;
                            move += char('a' + fromCol);
                            move += char('1' + (7 - fromRow));
                            move += char('a' + toCol);
                            move += char('1' + (7 - toRow));
                            moves.push_back(move);
                        } else {
                            if (isupper(target) != isWhite) {
                                std::string move;
                                move += char('a' + fromCol);
                                move += char('1' + (7 - fromRow));
                                move += char('a' + toCol);
                                move += char('1' + (7 - toRow));
                                moves.push_back(move);
                            }
                            break;
                        }
                    }
                }
            } else if (pieceType == 'q') {
                // Queen moves (all directions)
                int directions[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
                for (auto& dir : directions) {
                    for (int dist = 1; dist < 8; dist++) {
                        int toRow = fromRow + dir[0] * dist;
                        int toCol = fromCol + dir[1] * dist;
                        if (toRow < 0 || toRow >= 8 || toCol < 0 || toCol >= 8) break;

                        char target = board->getPiece(toRow, toCol);
                        if (target == '.') {
                            std::string move;
                            move += char('a' + fromCol);
                            move += char('1' + (7 - fromRow));
                            move += char('a' + toCol);
                            move += char('1' + (7 - toRow));
                            moves.push_back(move);
                        } else {
                            if (isupper(target) != isWhite) {
                                std::string move;
                                move += char('a' + fromCol);
                                move += char('1' + (7 - fromRow));
                                move += char('a' + toCol);
                                move += char('1' + (7 - toRow));
                                moves.push_back(move);
                            }
                            break;
                        }
                    }
                }
            } else if (pieceType == 'k') {
                // King moves (one square in any direction)
                int directions[8][2] = {{-1, -1}, {-1, 0}, {-1, 1}, {0, -1}, {0, 1}, {1, -1}, {1, 0}, {1, 1}};
                for (auto& dir : directions) {
                    int toRow = fromRow + dir[0];
                    int toCol = fromCol + dir[1];
                    if (toRow >= 0 && toRow < 8 && toCol >= 0 && toCol < 8) {
                        char target = board->getPiece(toRow, toCol);
                        if (target == '.' || isupper(target) != isWhite) {
                            std::string move;
                            move += char('a' + fromCol);
                            move += char('1' + (7 - fromRow));
                            move += char('a' + toCol);
                            move += char('1' + (7 - toRow));
                            moves.push_back(move);
                        }
                    }
                }

                // Castling moves
                if (isWhite && fromRow == 7 && fromCol == 4) {
                    // White castling (king on e1)
                    // Kingside castling (e1-g1)
                    if (board->canCastleKingside(true)) {
                        // Check if squares f1 and g1 are empty
                        if (board->getPiece(7, 5) == '.' && board->getPiece(7, 6) == '.') {
                            // Check if e1, f1, g1 are not under attack
                            if (!isSquareAttacked(7, 4, false) &&
                                !isSquareAttacked(7, 5, false) &&
                                !isSquareAttacked(7, 6, false)) {
                                moves.push_back("e1g1");
                            }
                        }
                    }
                    // Queenside castling (e1-c1)
                    if (board->canCastleQueenside(true)) {
                        // Check if squares d1, c1, b1 are empty
                        if (board->getPiece(7, 3) == '.' &&
                            board->getPiece(7, 2) == '.' &&
                            board->getPiece(7, 1) == '.') {
                            // Check if e1, d1, c1 are not under attack
                            if (!isSquareAttacked(7, 4, false) &&
                                !isSquareAttacked(7, 3, false) &&
                                !isSquareAttacked(7, 2, false)) {
                                moves.push_back("e1c1");
                            }
                        }
                    }
                } else if (!isWhite && fromRow == 0 && fromCol == 4) {
                    // Black castling (king on e8)
                    // Kingside castling (e8-g8)
                    if (board->canCastleKingside(false)) {
                        // Check if squares f8 and g8 are empty
                        if (board->getPiece(0, 5) == '.' && board->getPiece(0, 6) == '.') {
                            // Check if e8, f8, g8 are not under attack
                            if (!isSquareAttacked(0, 4, true) &&
                                !isSquareAttacked(0, 5, true) &&
                                !isSquareAttacked(0, 6, true)) {
                                moves.push_back("e8g8");
                            }
                        }
                    }
                    // Queenside castling (e8-c8)
                    if (board->canCastleQueenside(false)) {
                        // Check if squares d8, c8, b8 are empty
                        if (board->getPiece(0, 3) == '.' &&
                            board->getPiece(0, 2) == '.' &&
                            board->getPiece(0, 1) == '.') {
                            // Check if e8, d8, c8 are not under attack
                            if (!isSquareAttacked(0, 4, true) &&
                                !isSquareAttacked(0, 3, true) &&
                                !isSquareAttacked(0, 2, true)) {
                                moves.push_back("e8c8");
                            }
                        }
                    }
                }
            }
        }
    }

    std::vector<std::string> legalMoves;
    for (const std::string& move : moves) {
        // Parse the destination square
        int toCol = move[2] - 'a';
        int toRow = 7 - (move[3] - '1');
        char targetPiece = board->getPiece(toRow, toCol);

        // CRITICAL: Skip moves that capture own pieces
        // If target square has a piece AND it's the same color as side to move
        if (targetPiece != '.') {
            bool targetIsWhite = isupper(targetPiece);
            bool movingIsWhite = board->isWhiteToMove();
            if (targetIsWhite == movingIsWhite) {
                // This move would capture our own piece - ILLEGAL
                continue;  // Skip this move
            }
        }

        // Now test if this move leaves us in check
        // Save whose turn it is BEFORE making the move
        bool wasWhiteToMove = board->isWhiteToMove();

        MoveInfo info = makeMoveWithInfo(move);

        // After making the move, the turn has toggled
        // We need to check if the ORIGINAL side's king is in check
        // So we need to check if the king of the color that JUST moved is being attacked

        // Find our king (the side that just moved)
        char ourKing = wasWhiteToMove ? 'K' : 'k';
        int kingRow = -1, kingCol = -1;
        for (int row = 0; row < 8; row++) {
            for (int col = 0; col < 8; col++) {
                if (board->getPiece(row, col) == ourKing) {
                    kingRow = row;
                    kingCol = col;
                    break;
                }
            }
            if (kingRow != -1) break;
        }

        // Check if our king is being attacked by the opponent (whose turn it now is)
        bool ourKingInCheck = false;
        if (kingRow != -1) {
            // isSquareAttacked checks if square is attacked by the specified color
            // After the move, it's the opponent's turn, so check if they're attacking our king
            ourKingInCheck = isSquareAttacked(kingRow, kingCol, board->isWhiteToMove());
        }

        unmakeMove(move, info);

        // Only allow moves that DON'T leave our own king in check
        if (!ourKingInCheck) {
            legalMoves.push_back(move);
        }
    }

    return legalMoves;
}

bool Moves::isLegalMove(const std::string& move) {
    if (move.length() != 4 && move.length() != 5) return false;

    std::vector<std::string> legalMoves = generateLegalMoves();
    for (const std::string& legal : legalMoves) {
        if (legal == move) return true;
    }
    return false;
}

void Moves::makeMove(const std::string& move) {
    if (move.length() != 4 && move.length() != 5) return;

    int fromCol = move[0] - 'a';
    int fromRow = 7 - (move[1] - '1');
    int toCol = move[2] - 'a';
    int toRow = 7 - (move[3] - '1');

    char piece = board->getPiece(fromRow, fromCol);
    char pieceToPlace = piece;

    // Handle pawn promotion
    if (move.length() == 5) {
        char promotionPiece = move[4];  // q, r, b, or n
        bool isWhite = isupper(piece);
        pieceToPlace = isWhite ? toupper(promotionPiece) : tolower(promotionPiece);
    }

    board->setPiece(toRow, toCol, pieceToPlace);
    board->setPiece(fromRow, fromCol, '.');

    // Toggle side to move
    board->setWhiteToMove(!board->isWhiteToMove());
}

bool Moves::isSquareAttacked(int row, int col, bool byWhite) {
    int pawnDir = byWhite ? 1 : -1;
    for (int colOffset : {-1, 1}) {
        int attackRow = row + pawnDir;
        int attackCol = col + colOffset;
        if (attackRow >= 0 && attackRow < 8 && attackCol >= 0 && attackCol < 8) {
            char piece = board->getPiece(attackRow, attackCol);
            if (piece == (byWhite ? 'P' : 'p')) {
                return true;
            }
        }
    }

    int knightMoves[8][2] = {{-2,-1},{-2,1},{-1,-2},{-1,2},{1,-2},{1,2},{2,-1},{2,1}};
    for (auto& move : knightMoves) {
        int r = row + move[0];
        int c = col + move[1];
        if (r >= 0 && r < 8 && c >= 0 && c < 8) {
            char piece = board->getPiece(r, c);
            if (piece == (byWhite ? 'N' : 'n')) {
                return true;
            }
        }
    }

    int kingMoves[8][2] = {{-1,-1},{-1,0},{-1,1},{0,-1},{0,1},{1,-1},{1,0},{1,1}};
    for (auto& move : kingMoves) {
        int r = row + move[0];
        int c = col + move[1];
        if (r >= 0 && r < 8 && c >= 0 && c < 8) {
            char piece = board->getPiece(r, c);
            if (piece == (byWhite ? 'K' : 'k')) {
                return true;
            }
        }
    }

    int diagDirs[4][2] = {{-1,-1},{-1,1},{1,-1},{1,1}};
    for (auto& dir : diagDirs) {
        for (int dist = 1; dist < 8; dist++) {
            int r = row + dir[0] * dist;
            int c = col + dir[1] * dist;
            if (r < 0 || r >= 8 || c < 0 || c >= 8) break;

            char piece = board->getPiece(r, c);
            if (piece != '.') {
                if ((piece == (byWhite ? 'B' : 'b')) || (piece == (byWhite ? 'Q' : 'q'))) {
                    return true;
                }
                break;
            }
        }
    }

    int straightDirs[4][2] = {{-1,0},{1,0},{0,-1},{0,1}};
    for (auto& dir : straightDirs) {
        for (int dist = 1; dist < 8; dist++) {
            int r = row + dir[0] * dist;
            int c = col + dir[1] * dist;
            if (r < 0 || r >= 8 || c < 0 || c >= 8) break;

            char piece = board->getPiece(r, c);
            if (piece != '.') {
                if ((piece == (byWhite ? 'R' : 'r')) || (piece == (byWhite ? 'Q' : 'q'))) {
                    return true;
                }
                break;
            }
        }
    }

    return false;
}

bool Moves::isKingInCheck() {
    bool findWhiteKing = board->isWhiteToMove();
    char kingChar = findWhiteKing ? 'K' : 'k';

    int kingRow = -1, kingCol = -1;
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (board->getPiece(row, col) == kingChar) {
                kingRow = row;
                kingCol = col;
                break;
            }
        }
        if (kingRow != -1) break;
    }

    if (kingRow == -1) {
        return false;
    }

    return isSquareAttacked(kingRow, kingCol, !findWhiteKing);
}

MoveInfo Moves::makeMoveWithInfo(const std::string& move) {
    MoveInfo info;
    info.wasWhiteToMove = board->isWhiteToMove();
    info.promotedFrom = '.';  // Initialize promotion field

    // Save current castling rights
    info.whiteKingside = board->canCastleKingside(true);
    info.whiteQueenside = board->canCastleQueenside(true);
    info.blackKingside = board->canCastleKingside(false);
    info.blackQueenside = board->canCastleQueenside(false);

    // Save previous en passant target for undo
    info.previousEnPassantRow = board->getEnPassantRow();
    info.previousEnPassantCol = board->getEnPassantCol();
    info.wasEnPassantCapture = false;
    info.enPassantCapturedRow = -1;
    info.enPassantCapturedCol = -1;

    // Clear en passant target (will be set again if this move creates a new opportunity)
    board->clearEnPassantTarget();

    // CRITICAL: Save hash AFTER clearing en passant to ensure hash matches board state
    // This was causing 0.26% TT hit rate - hash was saved before modifying en passant state
    info.previousHash = board->getHash();

    if (move.length() != 4 && move.length() != 5) {
        info.capturedPiece = '.';
        return info;
    }

    int fromCol = move[0] - 'a';
    int fromRow = 7 - (move[1] - '1');
    int toCol = move[2] - 'a';
    int toRow = 7 - (move[3] - '1');

    info.capturedPiece = board->getPiece(toRow, toCol);

    char piece = board->getPiece(fromRow, fromCol);

    // Check if this is a castling move
    bool isCastling = false;
    if (tolower(piece) == 'k' && abs(toCol - fromCol) == 2) {
        isCastling = true;
        // Move the king
        board->setPiece(toRow, toCol, piece);
        board->setPiece(fromRow, fromCol, '.');

        // Move the rook
        if (toCol == 6) {
            // Kingside castling - move rook from h to f
            char rook = board->getPiece(fromRow, 7);
            board->setPiece(fromRow, 5, rook);
            board->setPiece(fromRow, 7, '.');
        } else if (toCol == 2) {
            // Queenside castling - move rook from a to d
            char rook = board->getPiece(fromRow, 0);
            board->setPiece(fromRow, 3, rook);
            board->setPiece(fromRow, 0, '.');
        }
    } else {
        // Normal move
        char pieceToPlace = piece;

        // Handle pawn promotion
        if (move.length() == 5) {
            char promotionPiece = move[4];  // q, r, b, or n
            // Store the original piece for undo
            info.promotedFrom = piece;
            // Convert to correct case based on whose turn it is
            bool isWhite = isupper(piece);
            pieceToPlace = isWhite ? toupper(promotionPiece) : tolower(promotionPiece);
        }

        // Check if this is an en passant capture
        if (tolower(piece) == 'p') {
            int epRow = info.previousEnPassantRow;
            int epCol = info.previousEnPassantCol;

            // If destination is the en passant target square
            if (epRow != -1 && toRow == epRow && toCol == epCol) {
                // This is an en passant capture!
                info.wasEnPassantCapture = true;

                // The captured pawn is NOT at toRow/toCol, it's beside it
                int capturedPawnRow = (board->isWhiteToMove()) ? toRow + 1 : toRow - 1;
                int capturedPawnCol = toCol;

                info.enPassantCapturedRow = capturedPawnRow;
                info.enPassantCapturedCol = capturedPawnCol;
                info.capturedPiece = board->getPiece(capturedPawnRow, capturedPawnCol);

                // Remove the captured pawn
                board->setPiece(capturedPawnRow, capturedPawnCol, '.');
            }
        }

        board->setPiece(toRow, toCol, pieceToPlace);
        board->setPiece(fromRow, fromCol, '.');
    }

    // Update castling rights based on what moved
    if (tolower(piece) == 'k') {
        // King moved - lose both castling rights
        if (isupper(piece)) {
            board->setCastleKingside(true, false);
            board->setCastleQueenside(true, false);
        } else {
            board->setCastleKingside(false, false);
            board->setCastleQueenside(false, false);
        }
    } else if (tolower(piece) == 'r') {
        // Rook moved - lose castling right on that side
        if (fromRow == 7 && fromCol == 0) {
            // White queenside rook
            board->setCastleQueenside(true, false);
        } else if (fromRow == 7 && fromCol == 7) {
            // White kingside rook
            board->setCastleKingside(true, false);
        } else if (fromRow == 0 && fromCol == 0) {
            // Black queenside rook
            board->setCastleQueenside(false, false);
        } else if (fromRow == 0 && fromCol == 7) {
            // Black kingside rook
            board->setCastleKingside(false, false);
        }
    }

    // If a rook was captured, update castling rights
    if (info.capturedPiece == 'R') {
        if (toRow == 7 && toCol == 0) board->setCastleQueenside(true, false);
        if (toRow == 7 && toCol == 7) board->setCastleKingside(true, false);
    } else if (info.capturedPiece == 'r') {
        if (toRow == 0 && toCol == 0) board->setCastleQueenside(false, false);
        if (toRow == 0 && toCol == 7) board->setCastleKingside(false, false);
    }

    // Check if this move creates an en passant opportunity
    // (pawn moved 2 squares forward from starting position)
    if (tolower(piece) == 'p' && abs(toRow - fromRow) == 2) {
        // This is a 2-square pawn advance
        // Set en passant target to the square the pawn "passed through"
        int targetRow = (fromRow + toRow) / 2;  // Middle square
        int targetCol = fromCol;
        board->setEnPassantTarget(targetRow, targetCol);
    }

    board->setWhiteToMove(!board->isWhiteToMove());

    // Recalculate hash after all changes (simpler and safer than incremental updates)
    board->recalculateHash();

    return info;
}

void Moves::unmakeMove(const std::string& move, const MoveInfo& info) {
    if (move.length() != 4 && move.length() != 5) return;

    // Restore previous en passant target
    board->setEnPassantTarget(info.previousEnPassantRow, info.previousEnPassantCol);

    int fromCol = move[0] - 'a';
    int fromRow = 7 - (move[1] - '1');
    int toCol = move[2] - 'a';
    int toRow = 7 - (move[3] - '1');

    char piece = board->getPiece(toRow, toCol);

    // If this was a promotion, restore the pawn instead of the promoted piece
    char pieceToRestore = piece;
    if (info.promotedFrom != '.') {
        pieceToRestore = info.promotedFrom;
    }

    // Check if this was a castling move
    if (tolower(piece) == 'k' && abs(toCol - fromCol) == 2) {
        // Undo castling
        // Move king back
        board->setPiece(fromRow, fromCol, piece);
        board->setPiece(toRow, toCol, '.');

        // Move rook back
        if (toCol == 6) {
            // Kingside castling - move rook back from f to h
            char rook = board->getPiece(fromRow, 5);
            board->setPiece(fromRow, 7, rook);
            board->setPiece(fromRow, 5, '.');
        } else if (toCol == 2) {
            // Queenside castling - move rook back from d to a
            char rook = board->getPiece(fromRow, 3);
            board->setPiece(fromRow, 0, rook);
            board->setPiece(fromRow, 3, '.');
        }
    } else {
        // Normal move undo
        board->setPiece(fromRow, fromCol, pieceToRestore);

        if (info.wasEnPassantCapture) {
            // En passant undo: restore captured pawn to its actual position
            board->setPiece(toRow, toCol, '.');  // Destination square is empty
            board->setPiece(info.enPassantCapturedRow, info.enPassantCapturedCol, info.capturedPiece);
        } else {
            // Normal capture undo
            board->setPiece(toRow, toCol, info.capturedPiece);
        }
    }

    // Restore castling rights
    board->setCastleKingside(true, info.whiteKingside);
    board->setCastleQueenside(true, info.whiteQueenside);
    board->setCastleKingside(false, info.blackKingside);
    board->setCastleQueenside(false, info.blackQueenside);

    board->setWhiteToMove(info.wasWhiteToMove);

    // Restore the hash from saved state (fastest and most reliable)
    board->setHash(info.previousHash);
}

bool Moves::isCheckmate() {
    // Checkmate = no legal moves AND king is in check
    std::vector<std::string> legalMoves = generateLegalMoves();

    if (legalMoves.empty() && isKingInCheck()) {
        return true;
    }

    return false;
}

bool Moves::isStalemate() {
    // Stalemate = no legal moves AND king is NOT in check
    std::vector<std::string> legalMoves = generateLegalMoves();

    if (legalMoves.empty() && !isKingInCheck()) {
        return true;
    }

    return false;
}

bool Moves::isInCheck(bool isWhite) {
    // Find the king
    int kingRow = -1;
    int kingCol = -1;
    char kingPiece = isWhite ? 'K' : 'k';

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            if (board->getPiece(row, col) == kingPiece) {
                kingRow = row;
                kingCol = col;
                break;
            }
        }
        if (kingRow != -1) break;
    }

    // King not found (shouldn't happen in valid position)
    if (kingRow == -1) return false;

    // Check if king square is attacked by opponent
    return isSquareAttacked(kingRow, kingCol, !isWhite);
}
