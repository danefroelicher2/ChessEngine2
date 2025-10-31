#include "board.h"
#include "moves.h"
#include "engine.h"
#include <iostream>
#include <iomanip>

using namespace std;

void printSeparator(const string& title = "") {
    cout << "\n========================================" << endl;
    if (!title.empty()) {
        cout << title << endl;
        cout << "========================================" << endl;
    }
}

void testSEE(const string& testName, const string& fen, const string& move, int expectedScore) {
    cout << "\n" << testName << endl;
    cout << "FEN: " << fen << endl;
    cout << "Move: " << move << endl;

    Board board;
    board.loadFromFEN(fen);
    Moves moves(&board);
    Engine engine(&board, &moves);

    int seeScore = engine.staticExchangeEvaluation(move);
    int moveScore = engine.scoreMove(move, -1);

    cout << "SEE score: " << seeScore << endl;
    cout << "Expected: " << expectedScore << endl;
    cout << "scoreMove() result: " << moveScore << endl;

    bool passed = (seeScore >= expectedScore - 50 && seeScore <= expectedScore + 50);
    cout << "Result: " << (passed ? "PASS" : "FAIL");
    if (!passed) {
        cout << " *** CRITICAL FAILURE ***";
    }
    cout << endl;
}

int main() {
    printSeparator("SEE DIAGNOSTIC - Testing Static Exchange Evaluation");

    cout << "\nThis diagnostic tests if SEE is returning correct values" << endl;
    cout << "for the exact positions that are failing in tactical tests." << endl;

    // TEST 1: Free Queen on e5
    testSEE(
        "TEST 1: Capture Free Queen (d8xe5)",
        "rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1",
        "d8e5",
        900  // Should win queen (900)
    );

    // TEST 2: King Captures Knight
    testSEE(
        "TEST 2: King Captures Knight (e8xe4)",
        "4k3/8/8/8/4N3/8/8/4K3 b - - 0 1",
        "e8e4",
        320  // Should win knight (320)
    );

    // TEST 3: Capture Free Pawn
    testSEE(
        "TEST 3: Capture Free Pawn (e5xe4)",
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPPNPPP/RNBQKB1R b KQkq - 0 1",
        "e5e4",
        100  // Should win pawn (100)
    );

    // TEST 4: Capture Checking Knight
    testSEE(
        "TEST 4: Capture Checking Knight (e8xe6)",
        "4k3/8/4N3/8/8/8/8/4K3 b - - 0 1",
        "e8e6",
        320  // Should win knight (320)
    );

    // TEST 5: Queen takes Pawn (might be defended)
    testSEE(
        "TEST 5: Queen Takes Pawn on e4",
        "rnbqkbnr/pppp1ppp/8/4p3/4P3/8/PPPP1PPP/RNBQKBNR b KQkq - 0 1",
        "d8e4",
        100  // Takes pawn, might be defended but should still be positive or zero
    );

    // TEST 6: Compare capture vs quiet move
    printSeparator("TEST 6: Move Scoring Comparison");

    Board board;
    board.loadFromFEN("rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1");
    Moves moves(&board);
    Engine engine(&board, &moves);

    string captureMove = "d8e5";
    string quietMove = "g8f6";

    int captureScore = engine.scoreMove(captureMove, -1);
    int quietScore = engine.scoreMove(quietMove, -1);

    cout << "\nFEN: rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1" << endl;
    cout << "Capture move (d8xe5) score: " << captureScore << endl;
    cout << "Quiet move (g8f6) score: " << quietScore << endl;
    cout << "Difference: " << (captureScore - quietScore) << endl;
    cout << "\nExpected: Capture should score MUCH higher (>1,000,000 vs <10,000)" << endl;

    bool passed = (captureScore > 1000000 && captureScore > quietScore);
    cout << "Result: " << (passed ? "PASS" : "FAIL");
    if (!passed) {
        cout << " *** CRITICAL FAILURE ***";
    }
    cout << endl;

    // SUMMARY
    printSeparator("DIAGNOSTIC SUMMARY");

    cout << "\nIf SEE tests FAIL:" << endl;
    cout << "  - SEE is returning wrong values (negative when should be positive)" << endl;
    cout << "  - This causes good captures to be scored below killer moves" << endl;
    cout << "  - Engine searches quiet moves before captures" << endl;
    cout << "  - FIX: Debug SEE implementation or disable SEE temporarily" << endl;

    cout << "\nIf TEST 6 FAILS:" << endl;
    cout << "  - scoreMove() is not using SEE correctly" << endl;
    cout << "  - Captures are not being prioritized" << endl;
    cout << "  - FIX: Review scoreMove() logic at engine.cpp:713-758" << endl;

    cout << "\nIf ALL PASS:" << endl;
    cout << "  - SEE and scoreMove are working correctly" << endl;
    cout << "  - Bug is elsewhere (evaluation function, search logic)" << endl;
    cout << "  - Need deeper investigation" << endl;

    printSeparator();

    return 0;
}
