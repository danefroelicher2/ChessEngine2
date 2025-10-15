// server.cpp - Simple HTTP server for chess engine API
// Listens on port 10000 and provides /move endpoint for chess move calculations

#include "board.h"
#include "moves.h"
#include "engine.h"
#include <iostream>
#include <string>
#include <sstream>
#include <cstring>
#include <algorithm>

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

// Server configuration
const int PORT = 10000;
const int BUFFER_SIZE = 4096;

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
            // No legal moves available (checkmate or stalemate)
            return "{\"move\":\"\",\"status\":\"error\",\"message\":\"No legal moves available\"}";
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
    std::string body = "{\"status\":\"error\",\"message\":\"Endpoint not found. Use GET /move?fen=... or GET /legal-moves?fen=...\"}";
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

    std::cout << "Server listening on port " << PORT << std::endl;
    std::cout << "Endpoints:" << std::endl;
    std::cout << "  GET /move?fen=<FEN_STRING>" << std::endl;
    std::cout << "  GET /legal-moves?fen=<FEN_STRING>" << std::endl;
    std::cout << "Press Ctrl+C to stop\n"
              << std::endl;

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

    return 0;
}
