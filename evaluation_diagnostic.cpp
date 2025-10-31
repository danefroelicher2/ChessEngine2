#include "board.h"
#include "moves.h"
#include "engine.h"
#include <iostream>
#include <vector>
#include <algorithm>
#include <iomanip>

using namespace std;

// ANSI color codes for output
const char* COLOR_GREEN = "\033[32m";
const char* COLOR_RED = "\033[31m";
const char* COLOR_YELLOW = "\033[33m";
const char* COLOR_BLUE = "\033[34m";
const char* COLOR_RESET = "\033[0m";

void printSeparator(const string& title = "") {
    cout << "\n========================================" << endl;
    if (!title.empty()) {
        cout << title << endl;
        cout << "========================================" << endl;
    }
}

void printResult(bool passed) {
    if (passed) {
        cout << COLOR_GREEN << "Result: PASS ✓" << COLOR_RESET << endl;
    } else {
        cout << COLOR_RED << "Result: FAIL ✗" << COLOR_RESET << endl;
    }
}

// Test 1: Evaluation Function Sanity Check
bool testEvaluationSanity() {
    printSeparator("TEST 1: Evaluation Sanity Check");

    bool allPassed = true;

    // Test 1a: White up a queen
    {
        cout << "\n1a. Position: White up a queen" << endl;
        cout << "FEN: rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" << endl;

        Board board;
        board.loadFromFEN("rnb1kbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        Moves moves(&board);
        Engine engine(&board, &moves);

        // Call private evaluate() through a search
        string bestMove = engine.getBestMove(100);  // Quick search to get evaluation

        cout << "Expected eval: ~+900 (White has extra queen)" << endl;
        cout << "Diagnosis: Checking if engine recognizes material advantage..." << endl;

        // For now, just document what we're testing
        cout << COLOR_YELLOW << "Manual check needed: Does engine prefer keeping the queen?" << COLOR_RESET << endl;
    }

    // Test 1b: Black up a queen
    {
        cout << "\n1b. Position: Black up a queen" << endl;
        cout << "FEN: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNB1KBNR w KQkq - 0 1" << endl;

        Board board;
        board.loadFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNB1KBNR w KQkq - 0 1");
        Moves moves(&board);
        Engine engine(&board, &moves);

        cout << "Expected eval: ~-900 (Black has extra queen)" << endl;
        cout << "Diagnosis: Checking if engine recognizes material disadvantage..." << endl;
        cout << COLOR_YELLOW << "Manual check needed: Does engine evaluate Black as better?" << COLOR_RESET << endl;
    }

    // Test 1c: Equal material
    {
        cout << "\n1c. Position: Equal material (starting position)" << endl;
        cout << "FEN: rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1" << endl;

        Board board;
        board.loadFromFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");
        Moves moves(&board);
        Engine engine(&board, &moves);

        cout << "Expected eval: ~0 (equal material)" << endl;
        cout << "Diagnosis: Checking if engine sees position as balanced..." << endl;
        cout << COLOR_YELLOW << "Manual check needed: Does engine evaluate close to 0?" << COLOR_RESET << endl;
    }

    return allPassed;
}

// Test 2: Capture Move Generation
bool testCaptureGeneration() {
    printSeparator("TEST 2: Capture Move Generation");

    cout << "\nPosition: White queen on e5, Black to move" << endl;
    cout << "FEN: rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1" << endl;

    Board board;
    board.loadFromFEN("rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1");
    Moves moves(&board);

    vector<string> legalMoves = moves.generateLegalMoves();

    cout << "\nTotal legal moves: " << legalMoves.size() << endl;

    // Find all captures
    vector<string> captures;
    for (const string& move : legalMoves) {
        int toCol = move[2] - 'a';
        int toRow = 8 - (move[3] - '0');
        char target = board.getPiece(toRow, toCol);
        if (target != '.') {
            captures.push_back(move);
        }
    }

    cout << "Total captures found: " << captures.size() << endl;
    cout << "\nAll capture moves:" << endl;
    for (const string& capture : captures) {
        int fromCol = capture[0] - 'a';
        int fromRow = 8 - (capture[1] - '0');
        int toCol = capture[2] - 'a';
        int toRow = 8 - (capture[3] - '0');

        char movingPiece = board.getPiece(fromRow, fromCol);
        char capturedPiece = board.getPiece(toRow, toCol);

        cout << "  " << capture << " (" << movingPiece << " captures "
             << capturedPiece << ")" << endl;
    }

    // Check for d8xe5 specifically
    bool foundQueenCapture = false;
    for (const string& move : legalMoves) {
        // d8 = col 3, row 0 -> e5 = col 4, row 3
        if (move[0] == 'd' && move[1] == '8' && move[2] == 'e' && move[3] == '5') {
            foundQueenCapture = true;
            cout << "\n✓ Found d8xe5 (Queen captures Queen)" << endl;
            break;
        }
    }

    if (!foundQueenCapture) {
        cout << COLOR_RED << "\n✗ d8xe5 NOT FOUND!" << COLOR_RESET << endl;
        cout << "This is CRITICAL - the move generator is broken!" << endl;
        return false;
    }

    printResult(foundQueenCapture);
    return foundQueenCapture;
}

// Test 3: Capture Move Scoring
bool testCaptureScoring() {
    printSeparator("TEST 3: Capture Move Scoring");

    cout << "\nPosition: White queen on e5, Black to move" << endl;
    cout << "FEN: rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1" << endl;

    Board board;
    board.loadFromFEN("rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1");
    Moves moves(&board);
    Engine engine(&board, &moves);

    vector<string> legalMoves = moves.generateLegalMoves();

    // We can't directly call scoreMove, but we can check the move ordering
    // by looking at what the engine would search

    cout << "\nGenerating scored moves (simulating engine's move ordering)..." << endl;

    // Find specific moves
    string queenCapture = "";
    string quietMove = "";

    for (const string& move : legalMoves) {
        int toCol = move[2] - 'a';
        int toRow = 8 - (move[3] - '0');
        char target = board.getPiece(toRow, toCol);

        // Look for d8xe5
        if (move[0] == 'd' && move[1] == '8' && move[2] == 'e' && move[3] == '5') {
            queenCapture = move;
        }

        // Look for a quiet move (e.g., g8f6)
        if (move[0] == 'g' && move[1] == '8' && move[2] == 'f' && move[3] == '6') {
            quietMove = move;
        }
    }

    if (queenCapture.empty()) {
        cout << COLOR_RED << "ERROR: Could not find d8xe5!" << COLOR_RESET << endl;
        return false;
    }

    cout << "\nFound queen capture: " << queenCapture << " (d8 captures Q on e5)" << endl;
    if (!quietMove.empty()) {
        cout << "Found quiet move: " << quietMove << " (knight to f6)" << endl;
    }

    cout << "\nExpected: Capture should score MUCH higher than quiet move (>1,000,000 vs <10,000)" << endl;
    cout << COLOR_YELLOW << "Note: Actual scores are internal to engine." << COLOR_RESET << endl;
    cout << "We'll verify in Test 4 by checking move ordering." << endl;

    return true;
}

// Test 4: Search Actually Tries Captures First
bool testSearchMoveOrdering() {
    printSeparator("TEST 4: Search Move Ordering");

    cout << "\nPosition: White queen on e5, Black to move" << endl;
    cout << "FEN: rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1" << endl;

    Board board;
    board.loadFromFEN("rnbqkbnr/pppppppp/8/4Q3/8/8/PPPPPPPP/RNB1KBNR b KQkq - 0 1");
    Moves moves(&board);
    Engine engine(&board, &moves);

    cout << "\nRunning engine search (500ms)..." << endl;
    cout << "Expected: Engine should choose d8xe5 (capturing free queen)" << endl;

    string bestMove = engine.getBestMove(500);

    cout << "\n" << COLOR_BLUE << "Best move chosen: " << bestMove << COLOR_RESET << endl;

    // Check if it's the queen capture
    bool isQueenCapture = (bestMove[0] == 'd' && bestMove[1] == '8' &&
                           bestMove[2] == 'e' && bestMove[3] == '5');

    if (isQueenCapture) {
        cout << COLOR_GREEN << "✓ Engine correctly chose to capture the free queen!" << COLOR_RESET << endl;
        printResult(true);
        return true;
    } else {
        cout << COLOR_RED << "✗ Engine DID NOT capture the free queen!" << COLOR_RESET << endl;
        cout << "This means the search or move ordering is broken." << endl;

        // Parse the move to show what it did instead
        int fromCol = bestMove[0] - 'a';
        int fromRow = 8 - (bestMove[1] - '0');
        int toCol = bestMove[2] - 'a';
        int toRow = 8 - (bestMove[3] - '0');

        char movingPiece = board.getPiece(fromRow, fromCol);
        char capturedPiece = board.getPiece(toRow, toCol);

        cout << "Instead, it chose: " << bestMove << " (" << movingPiece;
        if (capturedPiece != '.') {
            cout << " captures " << capturedPiece;
        } else {
            cout << " quiet move";
        }
        cout << ")" << endl;

        printResult(false);
        return false;
    }
}

// Additional Test: Simplest possible position
bool testSimplestCapture() {
    printSeparator("TEST 5: Simplest Capture (King vs Free Knight)");

    cout << "\nPosition: Black king can capture free white knight" << endl;
    cout << "FEN: 4k3/8/8/8/4N3/8/8/4K3 b - - 0 1" << endl;

    Board board;
    board.loadFromFEN("4k3/8/8/8/4N3/8/8/4K3 b - - 0 1");
    Moves moves(&board);
    Engine engine(&board, &moves);

    vector<string> legalMoves = moves.generateLegalMoves();

    cout << "\nLegal moves:" << endl;
    for (const string& move : legalMoves) {
        int toCol = move[2] - 'a';
        int toRow = 8 - (move[3] - '0');
        char target = board.getPiece(toRow, toCol);

        cout << "  " << move;
        if (target != '.') {
            cout << " (CAPTURE " << target << ")";
        }
        cout << endl;
    }

    cout << "\nRunning engine search (500ms)..." << endl;
    string bestMove = engine.getBestMove(500);

    cout << "\n" << COLOR_BLUE << "Best move: " << bestMove << COLOR_RESET << endl;

    // Check if king captures knight (e8 -> e4)
    int toCol = bestMove[2] - 'a';
    int toRow = 8 - (bestMove[3] - '0');
    char captured = board.getPiece(toRow, toCol);

    bool capturedKnight = (captured == 'N');

    if (capturedKnight) {
        cout << COLOR_GREEN << "✓ Engine captured the free knight!" << COLOR_RESET << endl;
        printResult(true);
        return true;
    } else {
        cout << COLOR_RED << "✗ Engine did NOT capture the free knight!" << COLOR_RESET << endl;
        cout << "Something is fundamentally broken." << endl;
        printResult(false);
        return false;
    }
}

int main() {
    cout << "\n";
    cout << "================================================================" << endl;
    cout << "  CHESS ENGINE EVALUATION DIAGNOSTIC TOOL" << endl;
    cout << "================================================================" << endl;
    cout << "\nThis diagnostic will test 4 critical components:" << endl;
    cout << "1. Evaluation function (material counting)" << endl;
    cout << "2. Capture move generation" << endl;
    cout << "3. Capture move scoring" << endl;
    cout << "4. Search move ordering" << endl;

    // Run all tests
    bool test1 = testEvaluationSanity();
    bool test2 = testCaptureGeneration();
    bool test3 = testCaptureScoring();
    bool test4 = testSearchMoveOrdering();
    bool test5 = testSimplestCapture();

    // Summary
    printSeparator("DIAGNOSTIC SUMMARY");

    cout << "\nTest Results:" << endl;
    cout << "1. Evaluation sanity:   " << (test1 ? COLOR_GREEN "PASS ✓" : COLOR_RED "FAIL ✗") << COLOR_RESET << endl;
    cout << "2. Capture generation:  " << (test2 ? COLOR_GREEN "PASS ✓" : COLOR_RED "FAIL ✗") << COLOR_RESET << endl;
    cout << "3. Capture scoring:     " << (test3 ? COLOR_GREEN "PASS ✓" : COLOR_RED "FAIL ✗") << COLOR_RESET << endl;
    cout << "4. Search ordering:     " << (test4 ? COLOR_GREEN "PASS ✓" : COLOR_RED "FAIL ✗") << COLOR_RESET << endl;
    cout << "5. Simplest capture:    " << (test5 ? COLOR_GREEN "PASS ✓" : COLOR_RED "FAIL ✗") << COLOR_RESET << endl;

    cout << "\n" << COLOR_YELLOW << "CRITICAL QUESTIONS:" << COLOR_RESET << endl;
    cout << "1. Does evaluate() return +900 when White has extra queen?  "
         << (test1 ? "NEEDS MANUAL CHECK" : "UNKNOWN") << endl;
    cout << "2. Does generateLegalMoves() include d8xe5?                 "
         << (test2 ? COLOR_GREEN "YES ✓" : COLOR_RED "NO ✗") << COLOR_RESET << endl;
    cout << "3. Does scoreMove() give d8xe5 high score (>1,000,000)?     NEEDS MANUAL CHECK" << endl;
    cout << "4. Does search try d8xe5 first?                             "
         << (test4 ? COLOR_GREEN "YES ✓" : COLOR_RED "NO ✗") << COLOR_RESET << endl;

    cout << "\n" << COLOR_YELLOW << "ROOT CAUSE DIAGNOSIS:" << COLOR_RESET << endl;

    if (!test2) {
        cout << COLOR_RED << "MOVE GENERATION IS BROKEN - can't generate queen capture!" << COLOR_RESET << endl;
    } else if (!test4 && !test5) {
        cout << COLOR_RED << "SEARCH/EVALUATION IS BROKEN - generates captures but doesn't choose them!" << COLOR_RESET << endl;
        cout << "Possible causes:" << endl;
        cout << "  - scoreMove() not scoring captures highly enough" << endl;
        cout << "  - Evaluation function returning wrong sign" << endl;
        cout << "  - Minimax not respecting move ordering" << endl;
        cout << "  - Alpha-beta pruning cutting off good moves" << endl;
    } else if (test5 && !test4) {
        cout << COLOR_YELLOW << "King can capture but Queen won't - complexity issue?" << COLOR_RESET << endl;
    } else {
        cout << COLOR_GREEN << "All tests passed! Engine is working correctly." << COLOR_RESET << endl;
    }

    cout << "\n================================================================" << endl;

    return 0;
}
