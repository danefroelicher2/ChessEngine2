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

    return moves;
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
