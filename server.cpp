// server.cpp - Simple HTTP server for chess engine API
// Listens on port 10000 and provides /move endpoint for chess move calculations

#include "board.h"
#include "moves.h"
#include "engine.h"
#include "database.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <algorithm>
#include <cstdlib> // for getenv
#include <fstream> // for file I/O
#include <ctime>   // for timestamp

// Platform-specific socket headers
#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
typedef int socklen_t;
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <arpa/inet.h>
#define SOCKET int
#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket close
#endif

// Server configurationn
// Read PORT from environment variable (required for Render deployment)
int getPortFromEnv()
{
    const char *portEnv = std::getenv("PORT");
    if (portEnv != nullptr)
    {
        return std::atoi(portEnv);
    }
    return 10000; // Default fallback for local development
}

const int PORT = getPortFromEnv();
const int BUFFER_SIZE = 4096;

// Global database connection
Database *db = nullptr;

// =============================================================================
// URL Decoding Functions
// =============================================================================

// Convert hex digit to integer (0-15)
int hexToInt(char c)
{
    if (c >= '0' && c <= '9')
        return c - '0';
    if (c >= 'A' && c <= 'F')
        return c - 'A' + 10;
    if (c >= 'a' && c <= 'f')
        return c - 'a' + 10;
    return 0;
}

// Decode URL-encoded string (e.g., %20 -> space, %2F -> /)
std::string urlDecode(const std::string &str)
{
    std::string result;
    result.reserve(str.length());

    for (size_t i = 0; i < str.length(); ++i)
    {
        if (str[i] == '%' && i + 2 < str.length())
        {
            // Convert %XX to character
            int value = hexToInt(str[i + 1]) * 16 + hexToInt(str[i + 2]);
            result += static_cast<char>(value);
            i += 2;
        }
        else if (str[i] == '+')
        {
            // Convert + to space
            result += ' ';
        }
        else
        {
            result += str[i];
        }
    }

    return result;
}

// =============================================================================
// HTTP Request Parsing
// =============================================================================

// Extract query parameter value from URL
// Example: extractQueryParam("GET /move?fen=rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR", "fen")
std::string extractQueryParam(const std::string &request, const std::string &param)
{
    // Find the parameter in the query string
    std::string searchStr = param + "=";
    size_t paramPos = request.find(searchStr);

    if (paramPos == std::string::npos)
    {
        return "";
    }

    // Extract value (until space or & or end of line)
    size_t valueStart = paramPos + searchStr.length();
    size_t valueEnd = request.find_first_of(" &\r\n", valueStart);

    if (valueEnd == std::string::npos)
    {
        valueEnd = request.length();
    }

    std::string encoded = request.substr(valueStart, valueEnd - valueStart);
    return urlDecode(encoded);
}

// Check if request is GET /move
bool isGetMoveRequest(const std::string &request)
{
    return request.find("GET /move") == 0;
}

// Check if request is GET /legal-moves
bool isGetLegalMovesRequest(const std::string &request)
{
    return request.find("GET /legal-moves") == 0;
}

// Check if request is GET /save-opening-move
bool isSaveOpeningMoveRequest(const std::string &request)
{
    return request.find("GET /save-opening-move") == 0;
}

// Check if request is GET /get-opening-move
bool isGetOpeningMoveRequest(const std::string &request)
{
    return request.find("GET /get-opening-move") == 0;
}

// Check if request is GET /delete-opening-move
bool isDeleteOpeningMoveRequest(const std::string &request)
{
    return request.find("GET /delete-opening-move") == 0;
}

// Check if request is GET /diagnostic
bool isDiagnosticRequest(const std::string &request)
{
    return request.find("GET /diagnostic") == 0;
}

// =============================================================================
// Chess Engine Integration
// =============================================================================

// Process chess move request: parse FEN, calculate best move
std::string processMoveRequest(const std::string &fenString)
{
    try
    {
        // Validate FEN string is not empty
        if (fenString.empty())
        {
            return "{\"move\":\"\",\"status\":\"error\",\"message\":\"Missing FEN parameter\"}";
        }

        std::cout << "Processing FEN: " << fenString << std::endl;

        // Initialize board and load FEN position
        Board board;
        board.loadFromFEN(fenString);

        // Create Moves object for move generation
        Moves moves(&board);

        // Generate all legal moves for current position
        std::vector<std::string> legalMoves = moves.generateLegalMoves();

        if (legalMoves.empty())
        {
            // No legal moves - check if checkmate or stalemate
            if (moves.isCheckmate())
            {
                // Determine winner (the side NOT to move wins)
                std::string winner = board.isWhiteToMove() ? "black" : "white";
                return "{\"move\":\"\",\"status\":\"checkmate\",\"winner\":\"" + winner + "\"}";
            }
            else if (moves.isStalemate())
            {
                return "{\"move\":\"\",\"status\":\"stalemate\",\"message\":\"Draw by stalemate\"}";
            }
            else
            {
                // Shouldn't reach here, but handle as error
                return "{\"move\":\"\",\"status\":\"error\",\"message\":\"No legal moves available\"}";
            }
        }

        // Create Engine object and find best move
        Engine engine(&board, &moves);
        std::string bestMove = engine.getBestMove();

        std::cout << "Best move: " << bestMove << std::endl;

        // Return success response
        return "{\"move\":\"" + bestMove + "\",\"status\":\"ok\"}";
    }
    catch (const std::exception &e)
    {
        // Handle any unexpected errors
        std::string errorMsg = e.what();
        // Escape quotes in error message
        size_t pos = 0;
        while ((pos = errorMsg.find("\"", pos)) != std::string::npos)
        {
            errorMsg.replace(pos, 1, "\\\"");
            pos += 2;
        }
        return "{\"move\":\"\",\"status\":\"error\",\"message\":\"" + errorMsg + "\"}";
    }
    catch (...)
    {
        return "{\"move\":\"\",\"status\":\"error\",\"message\":\"Unknown error occurred\"}";
    }
}

// Process legal moves request: parse FEN, return all legal moves
std::string processLegalMovesRequest(const std::string &fenString)
{
    try
    {
        // Validate FEN string is not empty
        if (fenString.empty())
        {
            return "{\"moves\":[],\"status\":\"error\",\"message\":\"Missing FEN parameter\"}";
        }

        std::cout << "Processing legal moves for FEN: " << fenString << std::endl;

        // Initialize board and load FEN position
        Board board;
        board.loadFromFEN(fenString);

        // Create Moves object for move generation
        Moves moves(&board);

        // Generate all legal moves for current position
        std::vector<std::string> legalMoves = moves.generateLegalMoves();

        // Build JSON array of moves
        std::ostringstream jsonMoves;
        jsonMoves << "[";
        for (size_t i = 0; i < legalMoves.size(); ++i)
        {
            jsonMoves << "\"" << legalMoves[i] << "\"";
            if (i < legalMoves.size() - 1)
            {
                jsonMoves << ",";
            }
        }
        jsonMoves << "]";

        std::cout << "Legal moves count: " << legalMoves.size() << std::endl;

        // Return success response
        return "{\"moves\":" + jsonMoves.str() + ",\"status\":\"ok\"}";
    }
    catch (const std::exception &e)
    {
        // Handle any unexpected errors
        std::string errorMsg = e.what();
        // Escape quotes in error message
        size_t pos = 0;
        while ((pos = errorMsg.find("\"", pos)) != std::string::npos)
        {
            errorMsg.replace(pos, 1, "\\\"");
            pos += 2;
        }
        return "{\"moves\":[],\"status\":\"error\",\"message\":\"" + errorMsg + "\"}";
    }
    catch (...)
    {
        return "{\"moves\":[],\"status\":\"error\",\"message\":\"Unknown error occurred\"}";
    }
}

// =============================================================================
// Opening Book Management
// =============================================================================

// Process save opening move request
std::string processSaveOpeningMoveRequest(const std::string &opening, const std::string &fen, const std::string &move)
{
    try
    {
        // Validate parameters
        if (opening.empty() || fen.empty() || move.empty())
        {
            return "{\"status\":\"error\",\"message\":\"Missing required parameters\"}";
        }

        std::cout << "Saving to database: " << opening << " | FEN: " << fen << " | Move: " << move << std::endl;

        if (!db)
        {
            return "{\"status\":\"error\",\"message\":\"Database not connected\"}";
        }

        // Save to database
        if (db->saveOpeningMove(opening, fen, move))
        {
            int count = db->getPositionsLearned(opening);
            std::cout << "Saved successfully! Total positions: " << count << std::endl;
            return "{\"status\":\"ok\",\"positions_learned\":" + std::to_string(count) + "}";
        }
        else
        {
            return "{\"status\":\"error\",\"message\":\"Failed to save to database\"}";
        }
    }
    catch (const std::exception &e)
    {
        return "{\"status\":\"error\",\"message\":\"" + std::string(e.what()) + "\"}";
    }
}

// Process get opening move request
std::string processGetOpeningMoveRequest(const std::string &opening, const std::string &fen)
{
    try
    {
        // Validate parameters
        if (opening.empty() || fen.empty())
        {
            return "{\"status\":\"error\",\"message\":\"Missing required parameters\"}";
        }

        std::cout << "Looking up in database: " << opening << " | FEN: " << fen << std::endl;

        if (!db)
        {
            return "{\"status\":\"error\",\"message\":\"Database not connected\"}";
        }

        // Get from database
        std::string move = db->getOpeningMove(opening, fen);

        if (!move.empty())
        {
            std::cout << "Found move: " << move << std::endl;
            return "{\"status\":\"ok\",\"move\":\"" + move + "\",\"found\":true}";
        }
        else
        {
            std::cout << "Position not found in database" << std::endl;
            return "{\"status\":\"ok\",\"move\":\"\",\"found\":false}";
        }
    }
    catch (const std::exception &e)
    {
        return "{\"status\":\"error\",\"message\":\"" + std::string(e.what()) + "\"}";
    }
}

// Process delete opening move request
std::string processDeleteOpeningMoveRequest(const std::string &opening, const std::string &fen)
{
    try
    {
        // Validate parameters
        if (opening.empty() || fen.empty())
        {
            return "{\"status\":\"error\",\"message\":\"Missing required parameters\"}";
        }

        std::cout << "Deleting from database: " << opening << " | FEN: " << fen << std::endl;

        if (!db)
        {
            return "{\"status\":\"error\",\"message\":\"Database not connected\"}";
        }

        // Delete from database
        if (db->deleteOpeningMove(opening, fen))
        {
            int count = db->getPositionsLearned(opening);
            std::cout << "Deleted successfully! Remaining positions: " << count << std::endl;
            return "{\"status\":\"ok\",\"message\":\"Position deleted\",\"positions_learned\":" + std::to_string(count) + "}";
        }
        else
        {
            return "{\"status\":\"error\",\"message\":\"Failed to delete from database\"}";
        }
    }
    catch (const std::exception &e)
    {
        return "{\"status\":\"error\",\"message\":\"" + std::string(e.what()) + "\"}";
    }
}

// =============================================================================
// HTTP Response Generation
// =============================================================================

// Build complete HTTP response with headers and JSON body
std::string buildHttpResponse(const std::string &jsonBody)
{
    std::ostringstream response;

    // HTTP status line
    response << "HTTP/1.1 200 OK\r\n";

    // Headers
    response << "Content-Type: application/json\r\n";
    response << "Content-Length: " << jsonBody.length() << "\r\n";
    response << "Access-Control-Allow-Origin: *\r\n"; // Allow CORS
    response << "Connection: close\r\n";
    response << "\r\n";

    // Body
    response << jsonBody;

    return response.str();
}

// Build 404 Not Found response
std::string build404Response()
{
    std::string body = "{\"status\":\"error\",\"message\":\"Endpoint not found. Available endpoints: GET /move?fen=..., GET /legal-moves?fen=..., GET /diagnostic, GET /save-opening-move?opening=...&fen=...&move=..., GET /get-opening-move?opening=...&fen=..., GET /delete-opening-move?opening=...&fen=...\"}";
    std::ostringstream response;

    response << "HTTP/1.1 404 Not Found\r\n";
    response << "Content-Type: application/json\r\n";
    response << "Content-Length: " << body.length() << "\r\n";
    response << "Connection: close\r\n";
    response << "\r\n";
    response << body;

    return response.str();
}

// =============================================================================
// Socket Initialization
// =============================================================================

// Initialize Windows sockets (no-op on Linux)
bool initializeSockets()
{
#ifdef _WIN32
    WSADATA wsaData;
    int result = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (result != 0)
    {
        std::cerr << "WSAStartup failed: " << result << std::endl;
        return false;
    }
#endif
    return true;
}

// Cleanup Windows sockets (no-op on Linux)
void cleanupSockets()
{
#ifdef _WIN32
    WSACleanup();
#endif
}

// =============================================================================
// Board State Diagnostics
// =============================================================================

std::string runBoardDiagnostics()
{
    std::stringstream result;
    int passCount = 0;
    int failCount = 0;

    result << "=== BOARD STATE DIAGNOSTICS ===\n\n";

    // TEST 1: Move/Unmove Consistency
    result << "TEST 1: Move/Unmove Consistency\n";
    try
    {
        Board board;
        Moves moves(&board);
        bool test1Pass = true;

        // Test 10 moves from starting position
        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        for (int i = 0; i < std::min(10, (int)legalMoves.size()); i++)
        {
            // Save state
            std::string boardBefore = board.toString();
            bool turnBefore = board.isWhiteToMove();
            int epRowBefore = board.getEnPassantRow();
            int epColBefore = board.getEnPassantCol();

            // Make and unmake move
            MoveInfo info = moves.makeMoveWithInfo(legalMoves[i]);
            moves.unmakeMove(legalMoves[i], info);

            // Check state
            std::string boardAfter = board.toString();
            bool turnAfter = board.isWhiteToMove();
            int epRowAfter = board.getEnPassantRow();
            int epColAfter = board.getEnPassantCol();

            if (boardBefore != boardAfter || turnBefore != turnAfter ||
                epRowBefore != epRowAfter || epColBefore != epColAfter)
            {
                result << "FAIL - Move " << legalMoves[i] << " corrupted state\n";
                test1Pass = false;
                break;
            }
        }

        if (test1Pass)
        {
            result << "Status: PASS\n";
            passCount++;
        }
        else
        {
            result << "Status: FAIL\n";
            failCount++;
        }
    }
    catch (const std::exception &e)
    {
        result << "Status: EXCEPTION - " << e.what() << "\n";
        failCount++;
    }
    result << "---\n\n";

    // TEST 2: Hash Consistency
    result << "TEST 2: Hash Consistency\n";
    try
    {
        Board board;
        Moves moves(&board);
        bool test2Pass = true;

        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        for (int i = 0; i < std::min(10, (int)legalMoves.size()); i++)
        {
            uint64_t hashBefore = board.getHash();

            MoveInfo info = moves.makeMoveWithInfo(legalMoves[i]);
            moves.unmakeMove(legalMoves[i], info);

            uint64_t hashAfter = board.getHash();
            if (hashBefore != hashAfter)
            {
                result << "FAIL - Move " << legalMoves[i] << " hash mismatch\n";
                result << "Hash before: 0x" << std::hex << hashBefore << std::dec << "\n";
                result << "Hash after:  0x" << std::hex << hashAfter << std::dec << "\n";
                test2Pass = false;
                break;
            }
        }

        if (test2Pass)
        {
            result << "Status: PASS\n";
            passCount++;
        }
        else
        {
            result << "Status: FAIL\n";
            failCount++;
        }
    }
    catch (const std::exception &e)
    {
        result << "Status: EXCEPTION - " << e.what() << "\n";
        failCount++;
    }
    result << "---\n\n";

    // TEST 3: King Always Findable
    result << "TEST 3: King Always Findable\n";
    try
    {
        Board board;
        bool test3Pass = true;

        // Count kings in starting position
        int whiteKings = 0, blackKings = 0;
        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                char piece = board.getPiece(row, col);
                if (piece == 'K')
                    whiteKings++;
                if (piece == 'k')
                    blackKings++;
            }
        }

        if (whiteKings != 1 || blackKings != 1)
        {
            result << "FAIL - Found " << whiteKings << " white kings, "
                   << blackKings << " black kings\n";
            test3Pass = false;
        }

        if (test3Pass)
        {
            result << "Status: PASS\n";
            passCount++;
        }
        else
        {
            result << "Status: FAIL\n";
            failCount++;
        }
    }
    catch (const std::exception &e)
    {
        result << "Status: EXCEPTION - " << e.what() << "\n";
        failCount++;
    }
    result << "---\n\n";

    // TEST 4: King Location After Move/Unmove
    result << "TEST 4: King Location After Move/Unmove\n";
    try
    {
        Board board;
        Moves moves(&board);
        bool test4Pass = true;

        // Find initial king positions
        int wkRow = -1, wkCol = -1, bkRow = -1, bkCol = -1;
        for (int row = 0; row < 8; row++)
        {
            for (int col = 0; col < 8; col++)
            {
                char piece = board.getPiece(row, col);
                if (piece == 'K')
                {
                    wkRow = row;
                    wkCol = col;
                }
                if (piece == 'k')
                {
                    bkRow = row;
                    bkCol = col;
                }
            }
        }

        std::vector<std::string> legalMoves = moves.generateLegalMoves();
        for (int i = 0; i < std::min(10, (int)legalMoves.size()); i++)
        {
            MoveInfo info = moves.makeMoveWithInfo(legalMoves[i]);
            moves.unmakeMove(legalMoves[i], info);

            // Find kings again
            int wkRow2 = -1, wkCol2 = -1, bkRow2 = -1, bkCol2 = -1;
            for (int row = 0; row < 8; row++)
            {
                for (int col = 0; col < 8; col++)
                {
                    char piece = board.getPiece(row, col);
                    if (piece == 'K')
                    {
                        wkRow2 = row;
                        wkCol2 = col;
                    }
                    if (piece == 'k')
                    {
                        bkRow2 = row;
                        bkCol2 = col;
                    }
                }
            }

            if (wkRow != wkRow2 || wkCol != wkCol2 ||
                bkRow != bkRow2 || bkCol != bkCol2)
            {
                result << "FAIL - King positions not restored after move "
                       << legalMoves[i] << "\n";
                result << "White king: (" << wkRow << "," << wkCol << ") -> ("
                       << wkRow2 << "," << wkCol2 << ")\n";
                result << "Black king: (" << bkRow << "," << bkCol << ") -> ("
                       << bkRow2 << "," << bkCol2 << ")\n";
                test4Pass = false;
                break;
            }
        }

        if (test4Pass)
        {
            result << "Status: PASS\n";
            passCount++;
        }
        else
        {
            result << "Status: FAIL\n";
            failCount++;
        }
    }
    catch (const std::exception &e)
    {
        result << "Status: EXCEPTION - " << e.what() << "\n";
        failCount++;
    }
    result << "---\n\n";

    // TEST 5: Deep Search State (4-ply)
    result << "TEST 5: Deep Search State (4-ply)\n";
    try
    {
        Board board;
        Moves moves(&board);
        bool test5Pass = true;

        uint64_t originalHash = board.getHash();
        std::string originalBoard = board.toString();

        // Make 4 moves deep
        std::vector<std::string> movesPlayed;
        std::vector<MoveInfo> moveInfos;

        for (int depth = 0; depth < 4; depth++)
        {
            std::vector<std::string> legalMoves = moves.generateLegalMoves();
            if (legalMoves.empty())
                break;

            std::string move = legalMoves[0];
            MoveInfo info = moves.makeMoveWithInfo(move);
            movesPlayed.push_back(move);
            moveInfos.push_back(info);
        }

        // Unmake all 4 moves
        for (int i = movesPlayed.size() - 1; i >= 0; i--)
        {
            moves.unmakeMove(movesPlayed[i], moveInfos[i]);
        }

        uint64_t finalHash = board.getHash();
        std::string finalBoard = board.toString();

        if (originalHash != finalHash || originalBoard != finalBoard)
        {
            result << "FAIL - State corrupted after 4-ply make/unmake\n";
            result << "Original hash: 0x" << std::hex << originalHash << std::dec << "\n";
            result << "Final hash:    0x" << std::hex << finalHash << std::dec << "\n";
            test5Pass = false;
        }

        if (test5Pass)
        {
            result << "Status: PASS\n";
            passCount++;
        }
        else
        {
            result << "Status: FAIL\n";
            failCount++;
        }
    }
    catch (const std::exception &e)
    {
        result << "Status: EXCEPTION - " << e.what() << "\n";
        failCount++;
    }
    result << "---\n\n";

    // Summary
    result << "=== DIAGNOSTIC SUMMARY ===\n";
    result << "Tests passed: " << passCount << "/5\n";
    result << "Tests failed: " << failCount << "/5\n\n";

    if (failCount > 0)
    {
        result << "ROOT CAUSE ANALYSIS:\n";
        result << "Board state integrity issues detected. ";
        result << "This explains both TT low hit rate and NMP segfaults.\n";
        result << "\nLikely causes:\n";
        result << "- Hash not properly saved/restored in unmakeMove()\n";
        result << "- Board state (pieces/castling/en passant) corruption\n";
        result << "- King position tracking failure\n";
    }
    else
    {
        result << "ROOT CAUSE ANALYSIS:\n";
        result << "All tests PASSED. Board state is solid.\n";
        result << "TT and NMP issues must be caused by something else:\n";
        result << "- toggleTurn() not updating hash correctly for NMP\n";
        result << "- TT hash collision or replacement strategy\n";
        result << "- Search logic error (not a board state issue)\n";
    }

    return result.str();
}

// =============================================================================
// Main Server Loop
// =============================================================================

int main()
{
    std::cout << "Chess Engine HTTP Server" << std::endl;
    std::cout << "=========================" << std::endl;

    // Initialize socket library (Windows only)
    if (!initializeSockets())
    {
        return 1;
    }

    // Create TCP socket
    SOCKET serverSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (serverSocket == INVALID_SOCKET)
    {
        std::cerr << "Failed to create socket" << std::endl;
        cleanupSockets();
        return 1;
    }

    // Allow socket reuse (helpful during development)
    int opt = 1;
    setsockopt(serverSocket, SOL_SOCKET, SO_REUSEADDR, (const char *)&opt, sizeof(opt));

    // Configure server address
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY; // Listen on all interfaces
    serverAddr.sin_port = htons(PORT);

    // Bind socket to port
    if (bind(serverSocket, (sockaddr *)&serverAddr, sizeof(serverAddr)) == SOCKET_ERROR)
    {
        std::cerr << "Failed to bind to port " << PORT << std::endl;
        closesocket(serverSocket);
        cleanupSockets();
        return 1;
    }

    // Start listening for connections (backlog of 5)
    if (listen(serverSocket, 5) == SOCKET_ERROR)
    {
        std::cerr << "Failed to listen on socket" << std::endl;
        closesocket(serverSocket);
        cleanupSockets();
        return 1;
    }

    // Startup logging for Render deployment detection
    std::cout << "Server listening on 0.0.0.0:" << PORT << std::endl;
    std::cout << "Ready to accept connections" << std::endl;
    std::cout << "Endpoints:" << std::endl;
    std::cout << "  GET /move?fen=<FEN_STRING>" << std::endl;
    std::cout << "  GET /legal-moves?fen=<FEN_STRING>" << std::endl;
    std::cout << "Press Ctrl+C to stop\n"
              << std::endl;
    std::flush(std::cout); // Force output to appear immediately

    // Initialize database connection
    std::cout << "Connecting to database..." << std::endl;
    const char *dbUrl = std::getenv("DATABASE_URL");
    if (!dbUrl)
    {
        std::cerr << "ERROR: DATABASE_URL environment variable not set!" << std::endl;
        std::cerr << "Please set DATABASE_URL in Render environment settings." << std::endl;
        closesocket(serverSocket);
        cleanupSockets();
        return 1;
    }

    db = new Database(dbUrl);
    if (!db->connect())
    {
        std::cerr << "Failed to connect to database. Exiting." << std::endl;
        delete db;
        closesocket(serverSocket);
        cleanupSockets();
        return 1;
    }
    std::cout << "Database connected successfully!" << std::endl;

    // Main server loop - handle requests one at a time
    while (true)
    {
        // Accept incoming connection
        sockaddr_in clientAddr;
        socklen_t clientAddrLen = sizeof(clientAddr);
        SOCKET clientSocket = accept(serverSocket, (sockaddr *)&clientAddr, &clientAddrLen);

        if (clientSocket == INVALID_SOCKET)
        {
            std::cerr << "Failed to accept connection" << std::endl;
            continue;
        }

        // Log client connection
        char clientIP[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &(clientAddr.sin_addr), clientIP, INET_ADDRSTRLEN);
        std::cout << "\n[" << clientIP << "] Connection received" << std::endl;

        // Read HTTP request
        char buffer[BUFFER_SIZE];
        memset(buffer, 0, BUFFER_SIZE);
        int bytesReceived = recv(clientSocket, buffer, BUFFER_SIZE - 1, 0);

        if (bytesReceived > 0)
        {
            std::string request(buffer, bytesReceived);
            std::cout << "[" << clientIP << "] " << request.substr(0, request.find("\r\n")) << std::endl;

            std::string response;

            // Route request
            if (isGetMoveRequest(request))
            {
                // Extract FEN parameter and process move request
                std::string fenString = extractQueryParam(request, "fen");
                std::string jsonResponse = processMoveRequest(fenString);
                response = buildHttpResponse(jsonResponse);
            }
            else if (isGetLegalMovesRequest(request))
            {
                // Extract FEN parameter and process legal moves request
                std::string fenString = extractQueryParam(request, "fen");
                std::string jsonResponse = processLegalMovesRequest(fenString);
                response = buildHttpResponse(jsonResponse);
            }
            else if (isSaveOpeningMoveRequest(request))
            {
                // Extract parameters and process save opening move request
                std::string opening = extractQueryParam(request, "opening");
                std::string fenString = extractQueryParam(request, "fen");
                std::string move = extractQueryParam(request, "move");
                std::string jsonResponse = processSaveOpeningMoveRequest(opening, fenString, move);
                response = buildHttpResponse(jsonResponse);
            }
            else if (isGetOpeningMoveRequest(request))
            {
                // Extract parameters and process get opening move request
                std::string opening = extractQueryParam(request, "opening");
                std::string fenString = extractQueryParam(request, "fen");
                std::string jsonResponse = processGetOpeningMoveRequest(opening, fenString);
                response = buildHttpResponse(jsonResponse);
            }
            else if (isDeleteOpeningMoveRequest(request))
            {
                // Extract parameters and process delete opening move request
                std::string opening = extractQueryParam(request, "opening");
                std::string fenString = extractQueryParam(request, "fen");
                std::string jsonResponse = processDeleteOpeningMoveRequest(opening, fenString);
                response = buildHttpResponse(jsonResponse);
            }
            else if (isDiagnosticRequest(request))
            {
                // Run board state diagnostics
                std::string diagnosticResults = runBoardDiagnostics();

                std::ostringstream diagnosticResponse;
                diagnosticResponse << "HTTP/1.1 200 OK\r\n";
                diagnosticResponse << "Content-Type: text/plain\r\n";
                diagnosticResponse << "Access-Control-Allow-Origin: *\r\n";
                diagnosticResponse << "Content-Length: " << diagnosticResults.length() << "\r\n";
                diagnosticResponse << "\r\n";
                diagnosticResponse << diagnosticResults;

                response = diagnosticResponse.str();
                std::cout << "[" << clientIP << "] Diagnostic completed" << std::endl;
            }
            else
            {
                // Unknown endpoint
                response = build404Response();
            }

            // Send response to client
            send(clientSocket, response.c_str(), response.length(), 0);
        }

        // Close connection
        closesocket(clientSocket);
        std::cout << "[" << clientIP << "] Connection closed" << std::endl;
    }

    // Cleanup (never reached in this implementation)
    closesocket(serverSocket);
    cleanupSockets();

    // Cleanup database connection
    if (db)
    {
        db->disconnect();
        delete db;
    }

    return 0;
}
