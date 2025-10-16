#include "engine.h"
#include <cctype>
#include <limits>
#include <algorithm>

const int Engine::pawnPST[8][8] = {
    {0, 0, 0, 0, 0, 0, 0, 0},
    {50, 50, 50, 50, 50, 50, 50, 50},
    {10, 10, 20, 30, 30, 20, 10, 10},
    {5, 5, 10, 25, 25, 10, 5, 5},
    {0, 0, 0, 20, 20, 0, 0, 0},
    {5, -5, -10, 0, 0, -10, -5, 5},
    {5, 10, 10, -20, -20, 10, 10, 5},
    {0, 0, 0, 0, 0, 0, 0, 0}
};

const int Engine::knightPST[8][8] = {
    {-50, -40, -30, -30, -30, -30, -40, -50},
    {-40, -20, 0, 0, 0, 0, -20, -40},
    {-30, 0, 10, 15, 15, 10, 0, -30},
    {-30, 5, 15, 20, 20, 15, 5, -30},
    {-30, 0, 15, 20, 20, 15, 0, -30},
    {-30, 5, 10, 15, 15, 10, 5, -30},
    {-40, -20, 0, 5, 5, 0, -20, -40},
    {-50, -40, -30, -30, -30, -30, -40, -50}
};

const int Engine::bishopPST[8][8] = {
    {-20, -10, -10, -10, -10, -10, -10, -20},
    {-10,   0,   0,   0,   0,   0,   0, -10},
    {-10,   0,   5,  10,  10,   5,   0, -10},
    {-10,   5,   5,  10,  10,   5,   5, -10},
    {-10,   0,  10,  10,  10,  10,   0, -10},
    {-10,  10,  10,  10,  10,  10,  10, -10},
    {-10,   5,   0,   0,   0,   0,   5, -10},
    {-20, -10, -10, -10, -10, -10, -10, -20}
};

const int Engine::rookPST[8][8] = {
    { 0,  0,  0,  0,  0,  0,  0,  0},
    { 5, 10, 10, 10, 10, 10, 10,  5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    {-5,  0,  0,  0,  0,  0,  0, -5},
    { 0,  0,  0,  5,  5,  0,  0,  0}
};

const int Engine::queenPST[8][8] = {
    {-20, -10, -10,  -5,  -5, -10, -10, -20},
    {-10,   0,   0,   0,   0,   0,   0, -10},
    {-10,   0,   5,   5,   5,   5,   0, -10},
    { -5,   0,   5,   5,   5,   5,   0,  -5},
    {  0,   0,   5,   5,   5,   5,   0,  -5},
    {-10,   5,   5,   5,   5,   5,   0, -10},
    {-10,   0,   5,   0,   0,   0,   0, -10},
    {-20, -10, -10,  -5,  -5, -10, -10, -20}
};

const int Engine::kingMiddlegamePST[8][8] = {
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-30, -40, -40, -50, -50, -40, -40, -30},
    {-20, -30, -30, -40, -40, -30, -30, -20},
    {-10, -20, -20, -20, -20, -20, -20, -10},
    { 20,  20,   0,   0,   0,   0,  20,  20},
    { 20,  30,  10,   0,   0,  10,  30,  20}
};

const int Engine::kingEndgamePST[8][8] = {
    {-50, -40, -30, -20, -20, -30, -40, -50},
    {-30, -20, -10,   0,   0, -10, -20, -30},
    {-30, -10,  20,  30,  30,  20, -10, -30},
    {-30, -10,  30,  40,  40,  30, -10, -30},
    {-30, -10,  30,  40,  40,  30, -10, -30},
    {-30, -10,  20,  30,  30,  20, -10, -30},
    {-30, -30,   0,   0,   0,   0, -30, -30},
    {-50, -30, -30, -30, -30, -30, -30, -50}
};

Engine::Engine(Board* b, Moves* m) : board(b), moves(m) {}

int Engine::getPSTValue(char piece, int row, int col, bool isWhite) {
    char type = tolower(piece);
    int pstRow = isWhite ? row : 7 - row;

    switch (type) {
        case 'p': return pawnPST[pstRow][col];
        case 'n': return knightPST[pstRow][col];
        case 'b': return bishopPST[pstRow][col];
        case 'r': return rookPST[pstRow][col];
        case 'q': return queenPST[pstRow][col];
        case 'k':
            return isEndgame() ? kingEndgamePST[pstRow][col] : kingMiddlegamePST[pstRow][col];
        default: return 0;
    }
}

int Engine::evaluatePawnStructure() {
    int score = 0;

    for (int col = 0; col < 8; col++) {
        int whitePawnsInCol = 0;
        int blackPawnsInCol = 0;

        // Count pawns in this column
        for (int row = 0; row < 8; row++) {
            char piece = board->getPiece(row, col);
            if (piece == 'P') whitePawnsInCol++;
            if (piece == 'p') blackPawnsInCol++;
        }

        // Doubled pawns penalty
        if (whitePawnsInCol > 1) score -= 30 * (whitePawnsInCol - 1);
        if (blackPawnsInCol > 1) score += 30 * (blackPawnsInCol - 1);
    }

    // Check for isolated and passed pawns
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = board->getPiece(row, col);
            if (piece != 'P' && piece != 'p') continue;

            bool isWhite = (piece == 'P');
            bool isolated = true;
            bool passed = true;

            // Check adjacent files for friendly pawns (isolated pawn check)
            for (int adjCol = col - 1; adjCol <= col + 1; adjCol += 2) {
                if (adjCol < 0 || adjCol >= 8) continue;

                for (int checkRow = 0; checkRow < 8; checkRow++) {
                    char adjPiece = board->getPiece(checkRow, adjCol);
                    if (isWhite && adjPiece == 'P') isolated = false;
                    if (!isWhite && adjPiece == 'p') isolated = false;
                }
            }

            // Check for passed pawns (no enemy pawns blocking or on adjacent files ahead)
            if (isWhite) {
                // Check ahead for white pawns (decreasing row numbers)
                for (int checkRow = row - 1; checkRow >= 0; checkRow--) {
                    // Check same file
                    if (board->getPiece(checkRow, col) == 'p') {
                        passed = false;
                        break;
                    }
                    // Check adjacent files
                    if (col > 0 && board->getPiece(checkRow, col - 1) == 'p') passed = false;
                    if (col < 7 && board->getPiece(checkRow, col + 1) == 'p') passed = false;
                }
            } else {
                // Check ahead for black pawns (increasing row numbers)
                for (int checkRow = row + 1; checkRow < 8; checkRow++) {
                    // Check same file
                    if (board->getPiece(checkRow, col) == 'P') {
                        passed = false;
                        break;
                    }
                    // Check adjacent files
                    if (col > 0 && board->getPiece(checkRow, col - 1) == 'P') passed = false;
                    if (col < 7 && board->getPiece(checkRow, col + 1) == 'P') passed = false;
                }
            }

            // Apply penalties/bonuses
            if (isolated) {
                score += isWhite ? -20 : 20;
            }

            if (passed) {
                int passedBonus = 50;
                // Increase bonus based on how far advanced the pawn is
                if (isWhite) {
                    passedBonus += (7 - row) * 10; // Closer to promotion = higher bonus
                } else {
                    passedBonus += row * 10;
                }
                score += isWhite ? passedBonus : -passedBonus;
            }
        }
    }

    return score;
}

bool Engine::isEndgame() {
    int totalMaterial = 0;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = board->getPiece(row, col);
            if (piece == '.') continue;

            char type = tolower(piece);
            switch (type) {
                case 'p': totalMaterial += 100; break;
                case 'n': totalMaterial += 320; break;
                case 'b': totalMaterial += 330; break;
                case 'r': totalMaterial += 500; break;
                case 'q': totalMaterial += 900; break;
            }
        }
    }

    return totalMaterial < 1500;
}

int Engine::evaluateKingSafety() {
    int score = 0;
    bool endgame = isEndgame();

    // Find kings
    int whiteKingRow = -1, whiteKingCol = -1;
    int blackKingRow = -1, blackKingCol = -1;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = board->getPiece(row, col);
            if (piece == 'K') {
                whiteKingRow = row;
                whiteKingCol = col;
            } else if (piece == 'k') {
                blackKingRow = row;
                blackKingCol = col;
            }
        }
    }

    // Evaluate white king safety
    if (whiteKingRow != -1) {
        // Check if castled (kingside: g1, queenside: c1)
        if (whiteKingRow == 7 && (whiteKingCol == 6 || whiteKingCol == 2)) {
            // Verify rook is in castled position
            if (whiteKingCol == 6 && board->getPiece(7, 5) == 'R') {
                score += 30; // Kingside castle
            } else if (whiteKingCol == 2 && board->getPiece(7, 3) == 'R') {
                score += 30; // Queenside castle
            }
        }

        // Check if king is in center (d or e file)
        if (whiteKingCol == 3 || whiteKingCol == 4) {
            if (endgame) {
                score += 20; // Central king is good in endgame
            } else {
                score -= 40; // Central king is bad in middlegame
            }
        }

        // Check pawn shield (only in middlegame)
        if (!endgame && whiteKingRow == 7) {
            // Check for kingside castle pawn shield
            if (whiteKingCol >= 5) {
                if (board->getPiece(6, 5) != 'P') score -= 15;
                if (board->getPiece(6, 6) != 'P') score -= 15;
                if (board->getPiece(6, 7) != 'P') score -= 15;
            }
            // Check for queenside castle pawn shield
            else if (whiteKingCol <= 3) {
                if (board->getPiece(6, 0) != 'P') score -= 15;
                if (board->getPiece(6, 1) != 'P') score -= 15;
                if (board->getPiece(6, 2) != 'P') score -= 15;
            }
        }
    }

    // Evaluate black king safety
    if (blackKingRow != -1) {
        // Check if castled (kingside: g8, queenside: c8)
        if (blackKingRow == 0 && (blackKingCol == 6 || blackKingCol == 2)) {
            // Verify rook is in castled position
            if (blackKingCol == 6 && board->getPiece(0, 5) == 'r') {
                score -= 30; // Kingside castle
            } else if (blackKingCol == 2 && board->getPiece(0, 3) == 'r') {
                score -= 30; // Queenside castle
            }
        }

        // Check if king is in center (d or e file)
        if (blackKingCol == 3 || blackKingCol == 4) {
            if (endgame) {
                score -= 20; // Central king is good in endgame
            } else {
                score += 40; // Central king is bad in middlegame (penalty for black)
            }
        }

        // Check pawn shield (only in middlegame)
        if (!endgame && blackKingRow == 0) {
            // Check for kingside castle pawn shield
            if (blackKingCol >= 5) {
                if (board->getPiece(1, 5) != 'p') score += 15;
                if (board->getPiece(1, 6) != 'p') score += 15;
                if (board->getPiece(1, 7) != 'p') score += 15;
            }
            // Check for queenside castle pawn shield
            else if (blackKingCol <= 3) {
                if (board->getPiece(1, 0) != 'p') score += 15;
                if (board->getPiece(1, 1) != 'p') score += 15;
                if (board->getPiece(1, 2) != 'p') score += 15;
            }
        }
    }

    return score;
}

int Engine::evaluateCenterControl() {
    int score = 0;
    bool endgame = isEndgame();

    // Center squares: d4(4,3), d5(3,3), e4(4,4), e5(3,4)
    int centerSquares[4][2] = {
        {4, 3}, // d4
        {3, 3}, // d5
        {4, 4}, // e4
        {3, 4}  // e5
    };

    // Reduce center control importance in endgame
    double centerMultiplier = endgame ? 0.5 : 1.0;

    for (int i = 0; i < 4; i++) {
        int row = centerSquares[i][0];
        int col = centerSquares[i][1];
        char piece = board->getPiece(row, col);

        // Evaluate occupation (pieces physically on central squares)
        if (piece != '.') {
            bool isWhite = isupper(piece);
            char type = tolower(piece);
            int occupationBonus = 0;

            // Different pieces get different bonuses for central occupation
            switch (type) {
                case 'p': occupationBonus = 25; break;  // Pawns are great in center
                case 'n': occupationBonus = 30; break;  // Knights excel in center
                case 'b': occupationBonus = 15; break;  // Bishops benefit from center
                case 'r': occupationBonus = 10; break;  // Rooks less dependent on center
                case 'q': occupationBonus = 10; break;  // Queen is mobile anywhere
                case 'k':
                    // King in center: bad in middlegame, good in endgame
                    occupationBonus = endgame ? 15 : -20;
                    break;
            }

            occupationBonus = (int)(occupationBonus * centerMultiplier);
            score += isWhite ? occupationBonus : -occupationBonus;
        }

        // Evaluate control (pieces attacking central squares)
        bool whiteControls = moves->isSquareAttacked(row, col, true);
        bool blackControls = moves->isSquareAttacked(row, col, false);

        // Control bonus (smaller than occupation bonus)
        int controlBonus = (int)(10 * centerMultiplier);

        if (whiteControls) score += controlBonus;
        if (blackControls) score -= controlBonus;
    }

    // Extended center (c3-c6, f3-f6) - smaller bonuses
    int extendedCenter[12][2] = {
        {5, 2}, {4, 2}, {3, 2}, {2, 2},  // c-file (c3-c6)
        {5, 5}, {4, 5}, {3, 5}, {2, 5},  // f-file (f3-f6)
        {5, 3}, {5, 4},                   // d3, e3
        {2, 3}, {2, 4}                    // d6, e6
    };

    for (int i = 0; i < 12; i++) {
        int row = extendedCenter[i][0];
        int col = extendedCenter[i][1];
        char piece = board->getPiece(row, col);

        // Small bonus for occupying extended center
        if (piece != '.') {
            bool isWhite = isupper(piece);
            char type = tolower(piece);
            int bonus = 0;

            switch (type) {
                case 'p': bonus = 8; break;
                case 'n': bonus = 10; break;
                case 'b': bonus = 5; break;
                default: bonus = 3; break;
            }

            bonus = (int)(bonus * centerMultiplier);
            score += isWhite ? bonus : -bonus;
        }
    }

    return score;
}

int Engine::countPseudoLegalMoves(int row, int col, char piece) {
    int moveCount = 0;
    bool isWhite = isupper(piece);
    char type = tolower(piece);

    // Helper lambda to check if a square is valid and either empty or contains enemy piece
    auto canMoveTo = [&](int r, int c) -> bool {
        if (r < 0 || r >= 8 || c < 0 || c >= 8) return false;
        char target = board->getPiece(r, c);
        if (target == '.') return true;
        return isupper(target) != isWhite; // Can capture opposite color
    };

    // Pawn moves (simplified - doesn't check en passant for performance)
    if (type == 'p') {
        int direction = isWhite ? -1 : 1;
        int startRank = isWhite ? 6 : 1;

        // Forward move
        if (board->getPiece(row + direction, col) == '.') {
            moveCount++;
            // Double push from start
            if (row == startRank && board->getPiece(row + 2 * direction, col) == '.') {
                moveCount++;
            }
        }

        // Captures
        if (col > 0) {
            char target = board->getPiece(row + direction, col - 1);
            if (target != '.' && isupper(target) != isWhite) moveCount++;
        }
        if (col < 7) {
            char target = board->getPiece(row + direction, col + 1);
            if (target != '.' && isupper(target) != isWhite) moveCount++;
        }
    }
    // Knight moves
    else if (type == 'n') {
        int knightMoves[8][2] = {{-2,-1}, {-2,1}, {-1,-2}, {-1,2}, {1,-2}, {1,2}, {2,-1}, {2,1}};
        for (int i = 0; i < 8; i++) {
            if (canMoveTo(row + knightMoves[i][0], col + knightMoves[i][1])) {
                moveCount++;
            }
        }
    }
    // Bishop moves
    else if (type == 'b') {
        int directions[4][2] = {{-1,-1}, {-1,1}, {1,-1}, {1,1}};
        for (int d = 0; d < 4; d++) {
            for (int dist = 1; dist < 8; dist++) {
                int r = row + directions[d][0] * dist;
                int c = col + directions[d][1] * dist;
                if (r < 0 || r >= 8 || c < 0 || c >= 8) break;

                char target = board->getPiece(r, c);
                if (target == '.') {
                    moveCount++;
                } else {
                    if (isupper(target) != isWhite) moveCount++; // Can capture
                    break; // Blocked
                }
            }
        }
    }
    // Rook moves
    else if (type == 'r') {
        int directions[4][2] = {{-1,0}, {1,0}, {0,-1}, {0,1}};
        for (int d = 0; d < 4; d++) {
            for (int dist = 1; dist < 8; dist++) {
                int r = row + directions[d][0] * dist;
                int c = col + directions[d][1] * dist;
                if (r < 0 || r >= 8 || c < 0 || c >= 8) break;

                char target = board->getPiece(r, c);
                if (target == '.') {
                    moveCount++;
                } else {
                    if (isupper(target) != isWhite) moveCount++; // Can capture
                    break; // Blocked
                }
            }
        }
    }
    // Queen moves (combination of rook and bishop)
    else if (type == 'q') {
        int directions[8][2] = {{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}};
        for (int d = 0; d < 8; d++) {
            for (int dist = 1; dist < 8; dist++) {
                int r = row + directions[d][0] * dist;
                int c = col + directions[d][1] * dist;
                if (r < 0 || r >= 8 || c < 0 || c >= 8) break;

                char target = board->getPiece(r, c);
                if (target == '.') {
                    moveCount++;
                } else {
                    if (isupper(target) != isWhite) moveCount++; // Can capture
                    break; // Blocked
                }
            }
        }
    }
    // King moves (doesn't check for castling or safety for performance)
    else if (type == 'k') {
        int kingMoves[8][2] = {{-1,-1}, {-1,0}, {-1,1}, {0,-1}, {0,1}, {1,-1}, {1,0}, {1,1}};
        for (int i = 0; i < 8; i++) {
            if (canMoveTo(row + kingMoves[i][0], col + kingMoves[i][1])) {
                moveCount++;
            }
        }
    }

    return moveCount;
}

int Engine::evaluateMobility() {
    int whiteMobility = 0;
    int blackMobility = 0;
    bool endgame = isEndgame();

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = board->getPiece(row, col);
            if (piece == '.') continue;

            bool isWhite = isupper(piece);
            char type = tolower(piece);
            int moveCount = countPseudoLegalMoves(row, col, piece);

            // Weight mobility differently for each piece type
            int mobilityValue = 0;
            switch (type) {
                case 'p':
                    // Pawns: low weight (1 point per move)
                    mobilityValue = moveCount * 1;
                    break;
                case 'n':
                    // Knights: high weight - trapped knights are terrible
                    // 0-2 moves: very bad, 3-5 moves: okay, 6-8 moves: excellent
                    mobilityValue = moveCount * 4;
                    if (moveCount <= 2) mobilityValue -= 15; // Penalty for trapped knight
                    break;
                case 'b':
                    // Bishops: high weight - blocked bishops are bad
                    mobilityValue = moveCount * 3;
                    if (moveCount <= 3) mobilityValue -= 10; // Penalty for blocked bishop
                    break;
                case 'r':
                    // Rooks: moderate-high weight, especially in endgame
                    mobilityValue = moveCount * (endgame ? 3 : 2);
                    break;
                case 'q':
                    // Queen: lower weight (already very mobile by nature)
                    mobilityValue = moveCount * 1;
                    break;
                case 'k':
                    // King: more important in endgame
                    mobilityValue = moveCount * (endgame ? 3 : 1);
                    break;
            }

            if (isWhite) {
                whiteMobility += mobilityValue;
            } else {
                blackMobility += mobilityValue;
            }
        }
    }

    // Return the mobility advantage (positive favors white)
    return whiteMobility - blackMobility;
}

int Engine::evaluateDevelopment() {
    int score = 0;

    // Only evaluate development in opening/middlegame
    if (isEndgame()) {
        return 0;
    }

    // Check WHITE development
    // Knights should be developed (not on b1 or g1)
    if (board->getPiece(7, 1) == 'N') score -= 10;  // Knight still on b1
    if (board->getPiece(7, 6) == 'N') score -= 10;  // Knight still on g1

    // Bishops should be developed (not on c1 or f1)
    if (board->getPiece(7, 2) == 'B') score -= 10;  // Bishop still on c1
    if (board->getPiece(7, 5) == 'B') score -= 10;  // Bishop still on f1

    // Queen should not be developed too early (penalize if moved from d1 before knights/bishops)
    bool whiteQueenMoved = (board->getPiece(7, 3) != 'Q');
    bool whiteMinorsStillHome = (board->getPiece(7, 1) == 'N' || board->getPiece(7, 6) == 'N' ||
                                  board->getPiece(7, 2) == 'B' || board->getPiece(7, 5) == 'B');

    if (whiteQueenMoved && whiteMinorsStillHome) {
        score -= 20;  // Penalty for early queen development
    }

    // Rooks should be connected or on open files (bonus if not on starting squares)
    bool whiteRookA1Moved = (board->getPiece(7, 0) != 'R');
    bool whiteRookH1Moved = (board->getPiece(7, 7) != 'R');

    // Small bonus for activating rooks
    if (whiteRookA1Moved) score += 5;
    if (whiteRookH1Moved) score += 5;

    // Bonus for castling (king not on e1)
    if (board->getPiece(7, 4) != 'K') {
        score += 15;  // King has moved (likely castled)
    }

    // Check BLACK development
    // Knights should be developed (not on b8 or g8)
    if (board->getPiece(0, 1) == 'n') score += 10;  // Knight still on b8
    if (board->getPiece(0, 6) == 'n') score += 10;  // Knight still on g8

    // Bishops should be developed (not on c8 or f8)
    if (board->getPiece(0, 2) == 'b') score += 10;  // Bishop still on c8
    if (board->getPiece(0, 5) == 'b') score += 10;  // Bishop still on f8

    // Queen should not be developed too early
    bool blackQueenMoved = (board->getPiece(0, 3) != 'q');
    bool blackMinorsStillHome = (board->getPiece(0, 1) == 'n' || board->getPiece(0, 6) == 'n' ||
                                  board->getPiece(0, 2) == 'b' || board->getPiece(0, 5) == 'b');

    if (blackQueenMoved && blackMinorsStillHome) {
        score += 20;  // Penalty for early queen development
    }

    // Rooks activation
    bool blackRookA8Moved = (board->getPiece(0, 0) != 'r');
    bool blackRookH8Moved = (board->getPiece(0, 7) != 'r');

    if (blackRookA8Moved) score -= 5;
    if (blackRookH8Moved) score -= 5;

    // Bonus for castling
    if (board->getPiece(0, 4) != 'k') {
        score -= 15;  // King has moved (likely castled)
    }

    return score;
}

int Engine::evaluate() {
    int score = 0;

    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            char piece = board->getPiece(row, col);
            if (piece == '.') continue;

            int value = 0;
            char type = tolower(piece);

            switch (type) {
                case 'p': value = 100; break;
                case 'n': value = 320; break;
                case 'b': value = 330; break;
                case 'r': value = 500; break;
                case 'q': value = 900; break;
                case 'k': value = 20000; break;
            }

            bool isWhite = isupper(piece);
            int pstValue = getPSTValue(piece, row, col, isWhite);

            if (isWhite) {
                score += value + pstValue;
            } else {
                score -= value + pstValue;
            }
        }
    }

    // Add pawn structure evaluation
    score += evaluatePawnStructure();

    // Add king safety evaluation
    score += evaluateKingSafety();

    // Add center control evaluation
    score += evaluateCenterControl();

    // Add mobility evaluation
    score += evaluateMobility();

    // Add development evaluation
    score += evaluateDevelopment();

    return score;
}

int Engine::minimax(int depth, int alpha, int beta, bool maximizing) {
    if (depth == 0) {
        return evaluate();
    }

    std::vector<std::string> legalMoves = moves->generateLegalMoves();

    if (legalMoves.empty()) {
        // No moves available (game over)
        return maximizing ? -999999 : 999999;
    }

    if (maximizing) {
        int maxEval = std::numeric_limits<int>::min();
        for (const std::string& move : legalMoves) {
            // Make move
            MoveInfo info = moves->makeMoveWithInfo(move);

            int eval = minimax(depth - 1, alpha, beta, false);

            // Undo move
            moves->unmakeMove(move, info);

            maxEval = std::max(maxEval, eval);
            alpha = std::max(alpha, eval);
            if (beta <= alpha) break;
        }
        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        for (const std::string& move : legalMoves) {
            // Make move
            MoveInfo info = moves->makeMoveWithInfo(move);

            int eval = minimax(depth - 1, alpha, beta, true);

            // Undo move
            moves->unmakeMove(move, info);

            minEval = std::min(minEval, eval);
            beta = std::min(beta, eval);
            if (beta <= alpha) break;
        }
        return minEval;
    }
}

std::string Engine::getBestMove() {
    std::vector<std::string> legalMoves = moves->generateLegalMoves();

    if (legalMoves.empty()) {
        return "";
    }

    std::string bestMove = legalMoves[0];
    int bestScore = board->isWhiteToMove() ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

    for (const std::string& move : legalMoves) {
        // Make move
        MoveInfo info = moves->makeMoveWithInfo(move);

        int score = minimax(3, std::numeric_limits<int>::min(), std::numeric_limits<int>::max(), !board->isWhiteToMove());

        // Undo move
        moves->unmakeMove(move, info);

        if (board->isWhiteToMove()) {
            if (score > bestScore) {
                bestScore = score;
                bestMove = move;
            }
        } else {
            if (score < bestScore) {
                bestScore = score;
                bestMove = move;
            }
        }
    }

    return bestMove;
}
