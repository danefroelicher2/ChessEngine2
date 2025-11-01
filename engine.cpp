#include "engine.h"
#include <cctype>
#include <limits>
#include <algorithm>
#include <iostream>

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

Engine::Engine(Board* b, Moves* m) : board(b), moves(m) {
    clearMoveOrdering();
    nodesSearched = 0;
    qNodesSearched = 0;
    betaCutoffs = 0;
    firstMoveCutoffs = 0;
    maxQDepthReached = 0;
    totalQDepth = 0;
    qSearches = 0;
    lmrReductions = 0;
    lmrReSearches = 0;
    timeLimit = 5000;  // Default 5 seconds
    pvMove = "";
}

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

// =============================================================================
// Move Ordering Functions
// =============================================================================

void Engine::clearMoveOrdering() {
    // Clear killer moves
    for (int depth = 0; depth < MAX_DEPTH; depth++) {
        killerMoves[depth][0] = "";
        killerMoves[depth][1] = "";
    }

    // Clear history table
    for (int fr = 0; fr < 8; fr++) {
        for (int fc = 0; fc < 8; fc++) {
            for (int tr = 0; tr < 8; tr++) {
                for (int tc = 0; tc < 8; tc++) {
                    historyTable[fr][fc][tr][tc] = 0;
                }
            }
        }
    }
}

int Engine::getPieceValue(char piece) {
    char type = tolower(piece);
    switch (type) {
        case 'p': return 100;
        case 'n': return 320;
        case 'b': return 330;
        case 'r': return 500;
        case 'q': return 900;
        case 'k': return 20000;
        default: return 0;
    }
}

void Engine::parseMove(const std::string& move, int& fromRow, int& fromCol, int& toRow, int& toCol) {
    // Move format: "e2e4" or "e7e8q" (with promotion)
    fromCol = move[0] - 'a';
    fromRow = 8 - (move[1] - '0');
    toCol = move[2] - 'a';
    toRow = 8 - (move[3] - '0');
}

bool Engine::isCapture(const std::string& move) {
    // Extract destination square from move string
    int toCol = move[2] - 'a';
    int toRow = 8 - (move[3] - '0');

    // Check if destination square has a piece (any piece means it's a capture)
    char piece = board->getPiece(toRow, toCol);
    return piece != '.';
}

int Engine::scoreMove(const std::string& move, int depth) {
    int fromRow, fromCol, toRow, toCol;
    parseMove(move, fromRow, fromCol, toRow, toCol);

    char movingPiece = board->getPiece(fromRow, fromCol);
    char capturedPiece = board->getPiece(toRow, toCol);

    // 1. Use SEE for captures to distinguish good from bad captures
    if (capturedPiece != '.') {
        int seeScore = staticExchangeEvaluation(move);

        if (seeScore > 0) {
            // Good capture (wins material)
            // Score high using MVV-LVA, but boosted by SEE value
            int victimValue = getPieceValue(capturedPiece);
            int attackerValue = getPieceValue(movingPiece);
            return 1000000 + (victimValue * 100 - attackerValue) + seeScore;
        } else if (seeScore == 0) {
            // Equal trade
            // Score above killer moves but below winning captures
            int victimValue = getPieceValue(capturedPiece);
            return 850000 + victimValue;
        } else {
            // Bad capture (loses material)
            // Score BELOW killer moves so it's searched late
            // seeScore is negative, so this will be less than 700000
            return 700000 + seeScore;
        }
    }

    // 2. Killer moves (second priority after good/equal captures)
    if (depth >= 0 && depth < MAX_DEPTH) {
        if (move == killerMoves[depth][0]) {
            return 900000;  // First killer
        }
        if (move == killerMoves[depth][1]) {
            return 800000;  // Second killer
        }
    }

    // 3. History heuristic (third priority)
    // Cap history values to prevent overflow
    int historyScore = historyTable[fromRow][fromCol][toRow][toCol];
    if (historyScore > 10000) historyScore = 10000;
    return historyScore;
}

std::vector<ScoredMove> Engine::scoreMoves(const std::vector<std::string>& moves, int depth, TTEntry* ttEntry) {
    std::vector<ScoredMove> scoredMoves;
    scoredMoves.reserve(moves.size());

    // Get best move from transposition table for move ordering
    // Use passed-in ttEntry to avoid duplicate probe
    std::string ttBestMove = "";
    if (ttEntry != nullptr && !ttEntry->bestMove.empty()) {
        ttBestMove = ttEntry->bestMove;
    }

    for (const std::string& move : moves) {
        int score = scoreMove(move, depth);

        // TEMPORARILY DISABLED: Testing if TT bonus prevents captures from being tried
        // Even 1,500,000 is too high - captures score ~1,090,000
        // if (!ttBestMove.empty() && move == ttBestMove) {
        //     score = 1500000;  // Higher than best captures but not overwhelming
        // }

        scoredMoves.push_back(ScoredMove(move, score));
    }

    // Sort moves by score (descending - highest scores first)
    std::sort(scoredMoves.begin(), scoredMoves.end());

    return scoredMoves;
}

void Engine::updateKillerMove(const std::string& move, int depth) {
    if (depth < 0 || depth >= MAX_DEPTH) return;

    // Don't store the same move twice
    if (move == killerMoves[depth][0]) return;

    // Shift moves: second -> discarded, first -> second, new -> first
    killerMoves[depth][1] = killerMoves[depth][0];
    killerMoves[depth][0] = move;
}

void Engine::updateHistory(const std::string& move, int depth) {
    int fromRow, fromCol, toRow, toCol;
    parseMove(move, fromRow, fromCol, toRow, toCol);

    // Increment history score (depth squared gives higher weight to deeper moves)
    // This rewards moves that cause cutoffs deeper in the tree
    int increment = depth * depth;
    historyTable[fromRow][fromCol][toRow][toCol] += increment;

    // Cap to prevent overflow
    if (historyTable[fromRow][fromCol][toRow][toCol] > 100000) {
        historyTable[fromRow][fromCol][toRow][toCol] = 100000;
    }
}

// =============================================================================
// Static Exchange Evaluation (SEE) Functions
// =============================================================================

bool Engine::pieceCanAttack(int fromRow, int fromCol, int targetRow, int targetCol, char piece) {
    if (fromRow == targetRow && fromCol == targetCol) return false;

    char type = tolower(piece);
    bool isWhite = isupper(piece);

    // Pawn attacks
    if (type == 'p') {
        int direction = isWhite ? -1 : 1;
        if (targetRow == fromRow + direction && abs(targetCol - fromCol) == 1) {
            return true;
        }
        return false;
    }

    // Knight attacks
    if (type == 'n') {
        int rowDiff = abs(targetRow - fromRow);
        int colDiff = abs(targetCol - fromCol);
        return (rowDiff == 2 && colDiff == 1) || (rowDiff == 1 && colDiff == 2);
    }

    // King attacks
    if (type == 'k') {
        int rowDiff = abs(targetRow - fromRow);
        int colDiff = abs(targetCol - fromCol);
        return rowDiff <= 1 && colDiff <= 1;
    }

    // Bishop attacks (diagonal)
    if (type == 'b' || type == 'q') {
        int rowDiff = abs(targetRow - fromRow);
        int colDiff = abs(targetCol - fromCol);
        if (rowDiff == colDiff && rowDiff > 0) {
            // Check if path is clear
            int rowDir = (targetRow - fromRow) / rowDiff;
            int colDir = (targetCol - fromCol) / colDiff;
            for (int i = 1; i < rowDiff; i++) {
                if (board->getPiece(fromRow + i * rowDir, fromCol + i * colDir) != '.') {
                    return false;  // Path blocked
                }
            }
            return true;
        }
    }

    // Rook attacks (straight)
    if (type == 'r' || type == 'q') {
        if (fromRow == targetRow || fromCol == targetCol) {
            // Check if path is clear
            int rowDir = (targetRow == fromRow) ? 0 : ((targetRow - fromRow) > 0 ? 1 : -1);
            int colDir = (targetCol == fromCol) ? 0 : ((targetCol - fromCol) > 0 ? 1 : -1);
            int steps = (rowDir != 0) ? abs(targetRow - fromRow) : abs(targetCol - fromCol);

            for (int i = 1; i < steps; i++) {
                if (board->getPiece(fromRow + i * rowDir, fromCol + i * colDir) != '.') {
                    return false;  // Path blocked
                }
            }
            return true;
        }
    }

    return false;
}

char Engine::getSmallestAttacker(int targetRow, int targetCol, bool attackerColor,
                                  int& fromRow, int& fromCol,
                                  const std::vector<bool>& usedPieces) {
    char cheapestPiece = '.';
    int cheapestValue = 99999;
    int bestFromRow = -1;
    int bestFromCol = -1;

    // Scan the entire board for attackers
    for (int row = 0; row < 8; row++) {
        for (int col = 0; col < 8; col++) {
            // Check if this piece has already been used in the exchange
            int pieceIndex = row * 8 + col;
            if (usedPieces[pieceIndex]) continue;

            char piece = board->getPiece(row, col);
            if (piece == '.') continue;

            // Check if piece is the correct color
            bool isPieceWhite = isupper(piece);
            if (isPieceWhite != attackerColor) continue;

            // Check if this piece can attack the target square
            if (pieceCanAttack(row, col, targetRow, targetCol, piece)) {
                int value = getPieceValue(piece);
                if (value < cheapestValue) {
                    cheapestValue = value;
                    cheapestPiece = piece;
                    bestFromRow = row;
                    bestFromCol = col;
                }
            }
        }
    }

    fromRow = bestFromRow;
    fromCol = bestFromCol;
    return cheapestPiece;
}

int Engine::staticExchangeEvaluation(const std::string& move) {
    // Parse move
    int fromRow, fromCol, toRow, toCol;
    parseMove(move, fromRow, fromCol, toRow, toCol);

    // Get piece being moved and piece being captured
    char attacker = board->getPiece(fromRow, fromCol);
    char victim = board->getPiece(toRow, toCol);

    if (victim == '.') return 0;  // Not a capture

    // Track which pieces have been used in the exchange
    std::vector<bool> usedPieces(64, false);

    // Initial gain: value of captured piece
    int gain[32];  // Maximum exchange depth
    int depth = 0;
    gain[depth] = getPieceValue(victim);

    // Mark the initial attacker as used
    usedPieces[fromRow * 8 + fromCol] = true;

    // Determine attacking side
    bool attackerIsWhite = isupper(attacker);
    bool currentSide = !attackerIsWhite;  // Defender moves next

    // Current piece on target square (after initial capture)
    char currentPiece = attacker;

    // Simulate exchange sequence
    while (true) {
        depth++;
        if (depth >= 32) break;  // Safety limit

        // Find cheapest piece that can recapture
        int attackerRow, attackerCol;
        char nextAttacker = getSmallestAttacker(toRow, toCol, currentSide,
                                                 attackerRow, attackerCol,
                                                 usedPieces);

        if (nextAttacker == '.') break;  // No more attackers

        // This attacker captures current piece
        gain[depth] = getPieceValue(currentPiece) - gain[depth - 1];

        // Mark this attacker as used
        usedPieces[attackerRow * 8 + attackerCol] = true;

        // Update for next iteration
        currentPiece = nextAttacker;
        currentSide = !currentSide;

        // Prune if the moving side is winning enough already
        if (std::max(-gain[depth-1], gain[depth]) < 0) break;
    }

    // Minimax the gain array (work backwards)
    while (--depth) {
        gain[depth-1] = -std::max(-gain[depth-1], gain[depth]);
    }

    return gain[0];
}

// =============================================================================
// Quiescence Search Functions
// =============================================================================

std::vector<std::string> Engine::generateTacticalMoves() {
    // Generate all legal moves first
    std::vector<std::string> allMoves = moves->generateLegalMoves();
    std::vector<std::string> tacticalMoves;
    tacticalMoves.reserve(allMoves.size() / 4);  // Estimate: ~25% of moves are captures

    for (const std::string& move : allMoves) {
        // Parse destination square
        int toCol = move[2] - 'a';
        int toRow = 8 - (move[3] - '0');

        // Check if destination square has an enemy piece (capture)
        char target = board->getPiece(toRow, toCol);
        if (target != '.') {
            tacticalMoves.push_back(move);
        }

        // Note: We're only including captures for now
        // Can add checks later if needed for stronger play
    }

    return tacticalMoves;
}

int Engine::quiescence(int alpha, int beta, int qDepth) {
    qNodesSearched++;

    // Check if time is up - return stand-pat if so
    if (isTimeUp()) {
        int eval = evaluate();
        // Quiescence uses negamax style - score must be from current player's perspective
        // evaluate() returns from White's perspective, so negate if Black to move
        if (!board->isWhiteToMove()) {
            eval = -eval;
        }
        return eval;
    }

    // Hard depth limit: Prevent quiescence from thrashing in complex positions
    // Reduced from 10 to 6 to fix performance bottleneck (223 → 5000+ nodes/sec)
    const int MAX_Q_DEPTH = 6;
    if (qDepth >= MAX_Q_DEPTH) {
        int eval = evaluate();
        // Quiescence uses negamax style - score must be from current player's perspective
        // evaluate() returns from White's perspective, so negate if Black to move
        if (!board->isWhiteToMove()) {
            eval = -eval;
        }
        return eval;
    }

    // Track statistics
    qSearches++;
    totalQDepth += qDepth;
    if (qDepth > maxQDepthReached) {
        maxQDepthReached = qDepth;
    }

    // Stand pat: Evaluate the current position without making any move
    // This allows us to "stop" if the position is already good enough
    int standPat = evaluate();

    // CRITICAL FIX: Quiescence uses negamax style - score must be from current player's perspective
    // evaluate() returns from White's perspective, so negate if Black to move
    if (!board->isWhiteToMove()) {
        standPat = -standPat;
    }

    // Beta cutoff: If standing pat is already better than beta,
    // the opponent won't let us reach this position
    if (standPat >= beta) {
        return beta;
    }

    // Update alpha if standing pat is better than current alpha
    // This raises the bar for what we're looking for
    if (standPat > alpha) {
        alpha = standPat;
    }

    // Generate only tactical moves (captures)
    std::vector<std::string> tacticalMoves = generateTacticalMoves();

    // If no tactical moves, return stand-pat evaluation (position is quiet)
    if (tacticalMoves.empty()) {
        return alpha;
    }

    // Score and sort tactical moves using MVV-LVA from Phase 2.1
    std::vector<ScoredMove> scoredMoves = scoreMoves(tacticalMoves, -1);  // depth=-1 for quiescence

    // Search each tactical move
    for (const ScoredMove& scoredMove : scoredMoves) {
        const std::string& move = scoredMove.move;

        // Delta pruning: Skip captures that can't possibly improve alpha
        // ONLY use delta pruning when we're not desperate (not far behind)
        // When losing, even "bad" captures might lead to counterplay

        // Parse move to get captured piece value
        int toCol = move[2] - 'a';
        int toRow = 8 - (move[3] - '0');
        char capturedPiece = board->getPiece(toRow, toCol);
        int capturedValue = getPieceValue(capturedPiece);

        // Only apply delta pruning when not too far behind
        // If standPat < alpha - 200, we're desperate and need to search ALL captures
        const int DESPERATION_THRESHOLD = 200;  // If losing by 2+ pawns, search everything
        const int DELTA_MARGIN = 200;           // Normal delta margin

        if (standPat >= alpha - DESPERATION_THRESHOLD) {
            // Not desperate - apply delta pruning
            if (standPat + capturedValue + DELTA_MARGIN < alpha) {
                continue;  // Futile capture - can't improve position enough
            }
        }
        // If standPat < alpha - DESPERATION_THRESHOLD, we're desperate
        // Search ALL captures (no pruning) - we need counterplay!

        // Make the tactical move
        MoveInfo info = moves->makeMoveWithInfo(move);

        // Recursively search this position
        // Negamax style: negate the score and swap alpha/beta
        int score = -quiescence(-beta, -alpha, qDepth + 1);

        // Unmake the move
        moves->unmakeMove(move, info);

        // Beta cutoff: This move is too good, opponent won't allow it
        if (score >= beta) {
            return beta;
        }

        // Update alpha if we found a better move
        if (score > alpha) {
            alpha = score;
        }
    }

    // Return the best score found (alpha)
    return alpha;
}

// =============================================================================
// Time Management Functions
// =============================================================================

bool Engine::isTimeUp() {
    auto now = std::chrono::steady_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
        now - searchStartTime
    ).count();
    return elapsed >= timeLimit;
}

// =============================================================================
// Evaluation Functions
// =============================================================================

int Engine::evaluate() {
    int score = 0;
    int materialScore = 0;
    int pstScore = 0;

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
                materialScore += value;
                pstScore += pstValue;
            } else {
                score -= value + pstValue;
                materialScore -= value;
                pstScore -= pstValue;
            }
        }
    }

    // DIAGNOSTIC: Track score components
    int kingSafetyScore = 0;
    int pawnStructureScore = 0;

    // CONDITIONAL: Positional evaluation functions
    #if FAST_EVAL_MODE == 0
        // Full evaluation (better positional play, slower search)
        pawnStructureScore = evaluatePawnStructure();
        kingSafetyScore = evaluateKingSafety();
        score += pawnStructureScore;
        score += kingSafetyScore;
        score += evaluateCenterControl();
        score += evaluateMobility();        // VERY EXPENSIVE
        score += evaluateDevelopment();
    #else
        // Fast evaluation (tactical focus, deeper search)
        pawnStructureScore = evaluatePawnStructure();
        kingSafetyScore = evaluateKingSafety();
        score += pawnStructureScore;   // Keep - relatively fast
        score += kingSafetyScore;      // Keep - important for king safety
        // DISABLED for speed testing:
        // score += evaluateCenterControl();  // Skip - moderate cost
        // score += evaluateMobility();       // Skip - VERY expensive
        // score += evaluateDevelopment();    // Skip - opening only
    #endif

    // DIAGNOSTIC LOGGING: Show breakdown of evaluation
    static int evalCount = 0;
    evalCount++;
    if (evalCount <= 20) {  // Only log first 20 evaluations to avoid spam
        std::cout << "[EVAL-DEBUG #" << evalCount << "] Total: " << score
                  << " = Material: " << materialScore
                  << " + PST: " << pstScore
                  << " + PawnStruct: " << pawnStructureScore
                  << " + KingSafety: " << kingSafetyScore << std::endl;
    }

    return score;
}

int Engine::minimax(int depth, int alpha, int beta, bool maximizing) {
    nodesSearched++;

    // Store original alpha for TT bound type determination
    int originalAlpha = alpha;

    // ============================================
    // TEMPORARY DIAGNOSTIC: TT PROBE DISABLED
    // Testing if TT is causing tactical failures
    // Date: 2025-10-31
    // Hypothesis: TT returning wrong cached evaluations
    // ============================================

    // Probe transposition table
    uint64_t posHash = board->getHash();
    TTEntry* ttEntry = nullptr;  // DIAGNOSTIC: Force TT miss to test hypothesis
    // TTEntry* ttEntry = transpositionTable.probe(posHash);  // TEMPORARILY DISABLED

    if (ttEntry != nullptr && ttEntry->depth >= depth) {
        // We've searched this position to sufficient depth before
        if (ttEntry->bound == EXACT) {
            return ttEntry->score;  // Exact score - use it!
        }
        if (ttEntry->bound == LOWER_BOUND && ttEntry->score >= beta) {
            return beta;  // Beta cutoff
        }
        if (ttEntry->bound == UPPER_BOUND && ttEntry->score <= alpha) {
            return alpha;  // Alpha not improved
        }
    }

    if (depth == 0) {
        // Instead of immediately returning static evaluation,
        // call quiescence search to avoid horizon effect
        return quiescence(alpha, beta, 0);
    }

    std::vector<std::string> legalMoves = moves->generateLegalMoves();

    if (legalMoves.empty()) {
        const int CHECKMATE_SCORE = 999999;
        return maximizing ? -CHECKMATE_SCORE : CHECKMATE_SCORE;
    }

    // Score and sort moves for better move ordering
    // Pass ttEntry to avoid duplicate TT probe
    std::vector<ScoredMove> scoredMoves = scoreMoves(legalMoves, depth, ttEntry);

    if (maximizing) {
        int maxEval = std::numeric_limits<int>::min();
        int moveIndex = 0;
        std::string bestMoveFound = "";

        // DIAGNOSTIC: Log move ordering at root level
        if (depth >= 3) {
            std::cout << "[SEARCH-DEBUG] Depth " << depth << " (maximizing): Searching "
                      << scoredMoves.size() << " moves" << std::endl;
            for (int i = 0; i < std::min(5, (int)scoredMoves.size()); i++) {
                std::cout << "  Move " << (i+1) << ": " << scoredMoves[i].move
                          << " (ordering score: " << scoredMoves[i].score << ")" << std::endl;
            }
        }

        for (const ScoredMove& scoredMove : scoredMoves) {
            const std::string& move = scoredMove.move;

            // Check time periodically (every 2000 nodes)
            if (nodesSearched % 2000 == 0 && isTimeUp()) {
                return maxEval > std::numeric_limits<int>::min() ? maxEval : 0;
            }

            // DIAGNOSTIC: Log move about to be tried
            if (depth >= 3 && moveIndex < 5) {
                std::cout << "[SEARCH-DEBUG] Depth " << depth << " trying move #" << (moveIndex+1)
                          << ": " << move << " (ordering score: " << scoredMove.score << ")" << std::endl;
            }

            // Make move
            MoveInfo info = moves->makeMoveWithInfo(move);

            // Late Move Reduction (LMR)
            int reduction = 0;
            if (moveIndex >= 4 && depth >= 3 && !isCapture(move)) {
                reduction = 2;  // Search 2 ply shallower for late non-captures
                lmrReductions++;
            }

            // Search with reduced depth
            int childEval = minimax(depth - 1 - reduction, alpha, beta, false);
            int eval = -childEval;  // Convert child perspective to current player

            // If reduced move beats alpha, re-search at full depth
            if (reduction > 0 && eval > alpha) {
                lmrReSearches++;
                childEval = minimax(depth - 1, alpha, beta, false);
                eval = -childEval;
            }

            // Undo move
            moves->unmakeMove(move, info);

            // DIAGNOSTIC: Log evaluation result
            if (depth >= 3 && moveIndex < 5) {
                std::cout << "[SEARCH-DEBUG] Depth " << depth << " move " << move
                          << " returned eval: " << eval << std::endl;
            }

            if (eval > maxEval) {
                maxEval = eval;
                bestMoveFound = move;
            }

            alpha = std::max(alpha, eval);
            if (beta <= alpha) {
                // Beta cutoff
                betaCutoffs++;
                if (moveIndex == 0) {
                    firstMoveCutoffs++;
                }

                // Update move ordering heuristics for non-capture moves
                // Check if this was a capture by looking at the MoveInfo
                if (info.capturedPiece == '.') {
                    updateKillerMove(move, depth);
                    updateHistory(move, depth);
                }

                break;
            }

            moveIndex++;
        }

        // Store result in transposition table
        // IMPORTANT: Store ALL depths - iterative deepening needs shallow searches stored!
        // Depth 1 results are used by depth 2, depth 2 by depth 3, etc.
        BoundType bound;
        if (maxEval <= originalAlpha) {
            bound = UPPER_BOUND;  // Failed low
        } else if (maxEval >= beta) {
            bound = LOWER_BOUND;  // Failed high (beta cutoff)
        } else {
            bound = EXACT;  // PV node
        }
        transpositionTable.store(posHash, depth, maxEval, bestMoveFound, bound);

        return maxEval;
    } else {
        int minEval = std::numeric_limits<int>::max();
        int moveIndex = 0;
        std::string bestMoveFound = "";

        // DIAGNOSTIC: Log move ordering at root level
        if (depth >= 3) {
            std::cout << "[SEARCH-DEBUG] Depth " << depth << " (minimizing): Searching "
                      << scoredMoves.size() << " moves" << std::endl;
            for (int i = 0; i < std::min(5, (int)scoredMoves.size()); i++) {
                std::cout << "  Move " << (i+1) << ": " << scoredMoves[i].move
                          << " (ordering score: " << scoredMoves[i].score << ")" << std::endl;
            }
        }

        for (const ScoredMove& scoredMove : scoredMoves) {
            const std::string& move = scoredMove.move;

            // Check time periodically (every 2000 nodes)
            if (nodesSearched % 2000 == 0 && isTimeUp()) {
                return minEval < std::numeric_limits<int>::max() ? minEval : 0;
            }

            // DIAGNOSTIC: Log move about to be tried
            if (depth >= 3 && moveIndex < 5) {
                std::cout << "[SEARCH-DEBUG] Depth " << depth << " trying move #" << (moveIndex+1)
                          << ": " << move << " (ordering score: " << scoredMove.score << ")" << std::endl;
            }

            // Make move
            MoveInfo info = moves->makeMoveWithInfo(move);

            // Late Move Reduction (LMR)
            int reduction = 0;
            if (moveIndex >= 4 && depth >= 3 && !isCapture(move)) {
                reduction = 2;  // Search 2 ply shallower for late non-captures
                lmrReductions++;
            }

            // Search with reduced depth
            int childEval = minimax(depth - 1 - reduction, alpha, beta, true);
            int eval = -childEval;  // Convert child perspective to current player

            // If reduced move beats alpha (from minimizer's perspective: eval < beta)
            if (reduction > 0 && eval < beta) {
                lmrReSearches++;
                childEval = minimax(depth - 1, alpha, beta, true);
                eval = -childEval;
            }

            // Undo move
            moves->unmakeMove(move, info);

            // DIAGNOSTIC: Log evaluation result
            if (depth >= 3 && moveIndex < 5) {
                std::cout << "[SEARCH-DEBUG] Depth " << depth << " move " << move
                          << " returned eval: " << eval << std::endl;
            }

            if (eval < minEval) {
                minEval = eval;
                bestMoveFound = move;
            }

            beta = std::min(beta, eval);
            if (beta <= alpha) {
                // Beta cutoff
                betaCutoffs++;
                if (moveIndex == 0) {
                    firstMoveCutoffs++;
                }

                // Update move ordering heuristics for non-capture moves
                // Check if this was a capture by looking at the MoveInfo
                if (info.capturedPiece == '.') {
                    updateKillerMove(move, depth);
                    updateHistory(move, depth);
                }

                break;
            }

            moveIndex++;
        }

        // Store result in transposition table
        // IMPORTANT: Store ALL depths - iterative deepening needs shallow searches stored!
        // Depth 1 results are used by depth 2, depth 2 by depth 3, etc.
        BoundType bound;
        if (minEval <= originalAlpha) {
            bound = UPPER_BOUND;  // Failed low
        } else if (minEval >= beta) {
            bound = LOWER_BOUND;  // Failed high (beta cutoff)
        } else {
            bound = EXACT;  // PV node
        }
        transpositionTable.store(posHash, depth, minEval, bestMoveFound, bound);

        return minEval;
    }
}

std::string Engine::searchAtDepth(int depth, int& outScore) {
    std::vector<std::string> legalMoves = moves->generateLegalMoves();

    if (legalMoves.empty()) {
        outScore = 0;
        return "";
    }

    // Score and sort moves, prioritizing PV move from previous iteration
    std::vector<ScoredMove> scoredMoves;
    scoredMoves.reserve(legalMoves.size());

    for (const std::string& move : legalMoves) {
        int score = scoreMove(move, depth);

        // TEMPORARILY DISABLED: Testing if PV bonus prevents captures from being tried
        // Even 1,500,000 is too high - captures score ~1,090,000
        // if (!pvMove.empty() && move == pvMove) {
        //     score = 1500000;  // Higher than best captures but not overwhelming
        // }

        scoredMoves.push_back(ScoredMove(move, score));
    }

    std::sort(scoredMoves.begin(), scoredMoves.end());

    // DIAGNOSTIC: Log root level move ordering
    std::cout << "\n[ROOT-SEARCH] Depth " << depth << ": Searching "
              << scoredMoves.size() << " root moves" << std::endl;
    std::cout << "[MINIMAX-DEBUG] Depth " << depth << " root search starting" << std::endl;
    for (int i = 0; i < std::min(5, (int)scoredMoves.size()); i++) {
        std::cout << "  Root move " << (i+1) << ": " << scoredMoves[i].move
                  << " (ordering score: " << scoredMoves[i].score << ")" << std::endl;
    }

    std::string bestMove = scoredMoves[0].move;
    int bestScore = board->isWhiteToMove() ? std::numeric_limits<int>::min() : std::numeric_limits<int>::max();

    int moveNum = 0;
    for (const ScoredMove& scoredMove : scoredMoves) {
        const std::string& move = scoredMove.move;

        // Check if time is up before searching this move
        if (isTimeUp()) {
            break;  // Return best move found so far
        }

        // DIAGNOSTIC: Log BEFORE trying the move
        std::cout << "\n[ROOT-SEARCH-DEBUG] ========================================" << std::endl;
        std::cout << "[ROOT-SEARCH-DEBUG] Trying move #" << (moveNum+1) << ": " << move << std::endl;
        std::cout << "[ROOT-SEARCH-DEBUG] Ordering score: " << scoredMove.score << std::endl;
        std::cout << "[MINIMAX-DEBUG] Trying " << move << " (order score: " << scoredMove.score << ")" << std::endl;

        // Make move
        MoveInfo info = moves->makeMoveWithInfo(move);

        int minimaxScore = minimax(depth - 1, std::numeric_limits<int>::min(),
                           std::numeric_limits<int>::max(), !board->isWhiteToMove());

        // Undo move
        moves->unmakeMove(move, info);

        // DIAGNOSTIC: Log AFTER getting minimax result
        std::cout << "[ROOT-SEARCH-DEBUG] Move " << move << " returned MINIMAX score: " << minimaxScore << std::endl;
        std::cout << "[MINIMAX-DEBUG] " << move << " returned minimax score: " << minimaxScore << std::endl;

        // Update best move if this is better
        int oldBestScore = bestScore;
        bool isNewBest = false;

        if (board->isWhiteToMove()) {
            if (minimaxScore > bestScore) {
                bestScore = minimaxScore;
                bestMove = move;
                isNewBest = true;
            }
        } else {
            if (minimaxScore < bestScore) {
                bestScore = minimaxScore;
                bestMove = move;
                isNewBest = true;
            }
        }

        // DIAGNOSTIC: Log whether this became the new best
        if (isNewBest) {
            std::cout << "[ROOT-SEARCH-DEBUG] *** NEW BEST MOVE: " << move << " ***" << std::endl;
            std::cout << "[ROOT-SEARCH-DEBUG] Minimax score: " << minimaxScore
                      << " (previous best: " << oldBestScore << ")" << std::endl;
            std::cout << "[MINIMAX-DEBUG] *** NEW BEST: " << move << " (score: " << minimaxScore << ") ***" << std::endl;
        } else {
            std::cout << "[ROOT-SEARCH-DEBUG] Move NOT chosen" << std::endl;
            std::cout << "[ROOT-SEARCH-DEBUG] Minimax score " << minimaxScore
                      << " vs current best " << bestScore << std::endl;
            if (board->isWhiteToMove()) {
                std::cout << "[ROOT-SEARCH-DEBUG] (White wants higher score)" << std::endl;
            } else {
                std::cout << "[ROOT-SEARCH-DEBUG] (Black wants lower score)" << std::endl;
            }
        }
        std::cout << "[ROOT-SEARCH-DEBUG] ========================================\n" << std::endl;

        moveNum++;
    }

    // DIAGNOSTIC: Log final choice
    std::cout << "[ROOT-SEARCH] Final choice at depth " << depth << ": "
              << bestMove << " (score: " << bestScore << ")\n" << std::endl;

    outScore = bestScore;
    return bestMove;
}

std::string Engine::getBestMove(int timeLimitMs) {
    // Set time limit and start timer
    timeLimit = timeLimitMs;
    searchStartTime = std::chrono::steady_clock::now();

    // Reset statistics for this search
    nodesSearched = 0;
    qNodesSearched = 0;
    betaCutoffs = 0;
    firstMoveCutoffs = 0;
    maxQDepthReached = 0;
    totalQDepth = 0;
    qSearches = 0;
    lmrReductions = 0;
    lmrReSearches = 0;

    // Clear history table to prevent stale data from previous positions
    // Killer moves are NOT cleared - they're depth-specific and update quickly
    for (int fr = 0; fr < 8; fr++) {
        for (int fc = 0; fc < 8; fc++) {
            for (int tr = 0; tr < 8; tr++) {
                for (int tc = 0; tc < 8; tc++) {
                    historyTable[fr][fc][tr][tc] = 0;
                }
            }
        }
    }

    std::vector<std::string> legalMoves = moves->generateLegalMoves();

    // DIAGNOSTIC: Log ALL moves generated to check if captures are missing
    std::cout << "\n[MOVE-GEN-DEBUG] Total legal moves generated: " << legalMoves.size() << std::endl;
    std::cout << "[MOVE-GEN-DEBUG] All moves:" << std::endl;
    for (size_t i = 0; i < legalMoves.size() && i < 20; i++) {
        std::cout << "[MOVE-GEN-DEBUG] " << legalMoves[i] << std::endl;
    }

    if (legalMoves.empty()) {
        return "";
    }

    // Initialize with first legal move
    std::string bestMove = legalMoves[0];
    int bestScore = 0;
    pvMove = "";  // Clear PV move from previous search

    std::cout << "\n=== Iterative Deepening Search ===" << std::endl;
    #if FAST_EVAL_MODE == 1
        std::cout << "FAST_EVAL_MODE: Enabled (tactical focus)" << std::endl;
    #else
        std::cout << "FAST_EVAL_MODE: Disabled (full evaluation)" << std::endl;
    #endif
    std::cout << "Time limit: " << timeLimitMs << "ms" << std::endl;

    // Iterative deepening loop
    for (int depth = 1; depth <= MAX_SEARCH_DEPTH; depth++) {
        // Check if we have time to start this depth
        auto now = std::chrono::steady_clock::now();
        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(
            now - searchStartTime
        ).count();

        // If we've used 80% of time, don't start new depth
        if (elapsed >= timeLimit * 0.8) {
            std::cout << "Time budget exceeded, stopping at depth " << (depth - 1) << std::endl;
            break;
        }

        // Store nodes before this depth
        long long nodesBeforeDepth = nodesSearched + qNodesSearched;
        auto depthStartTime = std::chrono::steady_clock::now();

        // Search at this depth
        int scoreThisDepth = 0;
        std::string moveThisDepth = searchAtDepth(depth, scoreThisDepth);

        // Check if search completed (didn't run out of time)
        if (!isTimeUp() && !moveThisDepth.empty()) {
            // Update best move from this completed depth
            bestMove = moveThisDepth;
            bestScore = scoreThisDepth;
            pvMove = moveThisDepth;  // Store for next iteration

            // Calculate statistics for this depth
            auto depthEndTime = std::chrono::steady_clock::now();
            auto depthTime = std::chrono::duration_cast<std::chrono::milliseconds>(
                depthEndTime - depthStartTime
            ).count();
            long long nodesThisDepth = (nodesSearched + qNodesSearched) - nodesBeforeDepth;

            // Output depth information
            std::cout << "Depth " << depth << ": " << moveThisDepth
                      << " (score: " << scoreThisDepth << ")"
                      << " [" << depthTime << "ms, "
                      << nodesThisDepth << " nodes]" << std::endl;
        } else {
            // Time ran out during this depth - use previous depth result
            std::cout << "Time expired during depth " << depth
                      << ", using depth " << (depth - 1) << " result" << std::endl;
            break;
        }
    }

    // Calculate total search time
    auto searchEndTime = std::chrono::steady_clock::now();
    auto totalTime = std::chrono::duration_cast<std::chrono::milliseconds>(
        searchEndTime - searchStartTime
    ).count();

    // Output final statistics
    std::cout << "\n=== Search Statistics ===" << std::endl;
    std::cout << "Best move: " << bestMove << " (score: " << bestScore << ")" << std::endl;
    std::cout << "Total time: " << totalTime << "ms" << std::endl;
    std::cout << "Regular nodes: " << nodesSearched << std::endl;
    std::cout << "Quiescence nodes: " << qNodesSearched << std::endl;
    std::cout << "Total nodes: " << (nodesSearched + qNodesSearched) << std::endl;
    std::cout << "Nodes/sec: " << (totalTime > 0 ? (nodesSearched + qNodesSearched) * 1000 / totalTime : 0) << std::endl;
    std::cout << "Beta cutoffs: " << betaCutoffs << std::endl;
    std::cout << "First move cutoffs: " << firstMoveCutoffs << std::endl;
    if (betaCutoffs > 0) {
        double percentage = (100.0 * firstMoveCutoffs) / betaCutoffs;
        std::cout << "First move cutoff rate: " << percentage << "%" << std::endl;
    }
    std::cout << "Quiescence searches: " << qSearches << std::endl;
    std::cout << "Max Q-depth: " << maxQDepthReached << std::endl;
    if (qSearches > 0) {
        double avgQDepth = (double)totalQDepth / qSearches;
        std::cout << "Avg Q-depth: " << avgQDepth << std::endl;
    }

    // Late Move Reduction statistics
    std::cout << "\n=== Late Move Reduction Statistics ===" << std::endl;
    std::cout << "LMR reductions: " << lmrReductions << std::endl;
    std::cout << "LMR re-searches: " << lmrReSearches << std::endl;
    if (lmrReductions > 0) {
        double reSearchRate = (100.0 * lmrReSearches) / lmrReductions;
        std::cout << "LMR re-search rate: " << reSearchRate << "%" << std::endl;
    }

    // Transposition table statistics
    std::cout << "\n=== Transposition Table Statistics ===" << std::endl;
    std::cout << "TT hits: " << transpositionTable.getHits() << std::endl;
    std::cout << "TT misses: " << transpositionTable.getMisses() << std::endl;
    std::cout << "TT hit rate: " << transpositionTable.getHitRate() << "%" << std::endl;
    std::cout << "TT stores: " << transpositionTable.getStores() << std::endl;
    std::cout << "TT collisions: " << transpositionTable.getCollisions() << std::endl;

    std::cout << "==========================\n" << std::endl;

    return bestMove;
}
