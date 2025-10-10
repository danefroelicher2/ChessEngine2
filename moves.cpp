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
                    moves.push_back(move);

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
    if (move.length() != 4) return false;

    std::vector<std::string> legalMoves = generateLegalMoves();
    for (const std::string& legal : legalMoves) {
        if (legal == move) return true;
    }
    return false;
}

void Moves::makeMove(const std::string& move) {
    if (move.length() != 4) return;

    int fromCol = move[0] - 'a';
    int fromRow = 7 - (move[1] - '1');
    int toCol = move[2] - 'a';
    int toRow = 7 - (move[3] - '1');

    char piece = board->getPiece(fromRow, fromCol);
    board->setPiece(toRow, toCol, piece);
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

    if (move.length() != 4) {
        info.capturedPiece = '.';
        return info;
    }

    int fromCol = move[0] - 'a';
    int fromRow = 7 - (move[1] - '1');
    int toCol = move[2] - 'a';
    int toRow = 7 - (move[3] - '1');

    info.capturedPiece = board->getPiece(toRow, toCol);

    char piece = board->getPiece(fromRow, fromCol);
    board->setPiece(toRow, toCol, piece);
    board->setPiece(fromRow, fromCol, '.');

    board->setWhiteToMove(!board->isWhiteToMove());

    return info;
}

void Moves::unmakeMove(const std::string& move, const MoveInfo& info) {
    if (move.length() != 4) return;

    int fromCol = move[0] - 'a';
    int fromRow = 7 - (move[1] - '1');
    int toCol = move[2] - 'a';
    int toRow = 7 - (move[3] - '1');

    char piece = board->getPiece(toRow, toCol);
    board->setPiece(fromRow, fromCol, piece);
    board->setPiece(toRow, toCol, info.capturedPiece);

    board->setWhiteToMove(info.wasWhiteToMove);
}
