#include "board.h"
#include "moves.h"
#include "engine.h"
#include <iostream>

using namespace std;

int main() {
    cout << "\n==============================================================================" << endl;
    cout << "CRITICAL DIAGNOSTIC: Why Can't Engine See Free Material?" << endl;
    cout << "==============================================================================" << endl;

    // Test 1: Can engine capture free queen?
    cout << "\n--- TEST 1: Free Queen Capture ---" << endl;
    cout << "Position: White queen on e5, Black to move" << endl;
    cout << "FEN: rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1" << endl;

    Board board1;
    board1.loadFromFEN("rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1");
    Moves moves1(&board1);
    Engine engine1(&board1, &moves1);

    // Check move generation
    vector<string> legalMoves = moves1.generateLegalMoves();
    cout << "\nTotal legal moves: " << legalMoves.size() << endl;

    // Look for d8xe5
    bool found = false;
    for (const string& move : legalMoves) {
        if (move[0] == 'd' && move[1] == '8' && move[2] == 'e' && move[3] == '5') {
            cout << "Found d8xe5 in legal moves - GOOD!" << endl;
            found = true;
            break;
        }
    }

    if (!found) {
        cout << "ERROR: d8xe5 NOT in legal moves! Move generation is BROKEN!" << endl;
        return 1;
    }

    // Now let engine choose
    cout << "\nLetting engine choose best move (1000ms search)..." << endl;
    string bestMove = engine1.getBestMove(1000);

    cout << "\nEngine chose: " << bestMove << endl;

    if (bestMove[0] == 'd' && bestMove[1] == '8' && bestMove[2] == 'e' && bestMove[3] == '5') {
        cout << "SUCCESS: Engine captured the free queen!" << endl;
    } else {
        cout << "FAILURE: Engine did NOT capture the free queen!" << endl;
        cout << "This proves the search/evaluation is broken." << endl;
    }

    // Test 2: Simplest possible - King captures free knight
    cout << "\n\n--- TEST 2: King Captures Free Knight ---" << endl;
    cout << "Position: Black king can capture free white knight" << endl;
    cout << "FEN: 4k3/8/8/8/4N3/8/8/4K3 b - - 0 1" << endl;

    Board board2;
    board2.loadFromFEN("4k3/8/8/8/4N3/8/8/4K3 b - - 0 1");
    Moves moves2(&board2);
    Engine engine2(&board2, &moves2);

    cout << "\nLetting engine choose best move (1000ms search)..." << endl;
    string bestMove2 = engine2.getBestMove(1000);

    cout << "\nEngine chose: " << bestMove2 << endl;

    // Check if it captured the knight (knight is on e4)
    int toCol = bestMove2[2] - 'a';
    int toRow = 8 - (bestMove2[3] - '0');
    char captured = board2.getPiece(toRow, toCol);

    if (captured == 'N') {
        cout << "SUCCESS: Engine captured the free knight!" << endl;
    } else {
        cout << "FAILURE: Engine did NOT capture the free knight!" << endl;
        cout << "Even the simplest capture fails!" << endl;
    }

    cout << "\n==============================================================================" << endl;
    cout << "DIAGNOSIS COMPLETE" << endl;
    cout << "==============================================================================" << endl;

    return 0;
}
