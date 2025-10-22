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

// Server configuration
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

// Get current timestamp in ISO 8601 format
std::string getCurrentTimestamp()
{
    time_t now = time(nullptr);
    char buffer[32];
    strftime(buffer, sizeof(buffer), "%Y-%m-%dT%H:%M:%SZ", gmtime(&now));
    return std::string(buffer);
}

// Escape special characters in JSON strings
std::string escapeJsonString(const std::string &str)
{
    std::string escaped;
    for (char c : str)
    {
        if (c == '"')
            escaped += "\\\"";
        else if (c == '\\')
            escaped += "\\\\";
        else if (c == '\n')
            escaped += "\\n";
        else if (c == '\r')
            escaped += "\\r";
        else if (c == '\t')
            escaped += "\\t";
        else
            escaped += c;
    }
    return escaped;
}

// Simple function to find and extract a JSON string value
std::string extractJsonStringValue(const std::string &json, const std::string &key)
{
    std::string searchPattern = "\"" + key + "\":\"";
    size_t startPos = json.find(searchPattern);
    if (startPos == std::string::npos)
        return "";

    startPos += searchPattern.length();
    size_t endPos = json.find("\"", startPos);
    if (endPos == std::string::npos)
        return "";

    return json.substr(startPos, endPos - startPos);
}

// Simple function to find and extract a JSON number value
int extractJsonNumberValue(const std::string &json, const std::string &key)
{
    std::string searchPattern = "\"" + key + "\":";
    size_t startPos = json.find(searchPattern);
    if (startPos == std::string::npos)
        return 0;

    startPos += searchPattern.length();
    // Skip whitespace
    while (startPos < json.length() && (json[startPos] == ' ' || json[startPos] == '\t'))
        startPos++;

    size_t endPos = startPos;
    while (endPos < json.length() && (json[endPos] >= '0' && json[endPos] <= '9'))
        endPos++;

    if (endPos == startPos)
        return 0;

    return std::atoi(json.substr(startPos, endPos - startPos).c_str());
}

// Process save opening move request
std::string processSaveOpeningMoveRequest(const std::string &opening, const std::string &fen, const std::string &move)
{
    try
    {
        // Validate parameters
        if (opening.empty() || fen.empty() || move.empty())
        {
            return "{\"status\":\"error\",\"message\":\"Missing required parameters: opening, fen, move\"}";
        }

        std::cout << "Saving opening move: " << opening << " | FEN: " << fen << " | Move: " << move << std::endl;

        // Build file path
        std::string filename = "openings/" + opening + ".json";

        // Read existing file
        std::ifstream inFile(filename);
        if (!inFile.is_open())
        {
            return "{\"status\":\"error\",\"message\":\"Opening file not found: " + filename + "\"}";
        }

        // Read entire file into string
        std::string jsonContent((std::istreambuf_iterator<char>(inFile)),
                                std::istreambuf_iterator<char>());
        inFile.close();

        // Extract current positions count
        int currentCount = extractJsonNumberValue(jsonContent, "positions_learned");

        // Find the "positions" object and check if this FEN already exists
        size_t positionsStart = jsonContent.find("\"positions\":");
        if (positionsStart == std::string::npos)
        {
            return "{\"status\":\"error\",\"message\":\"Invalid JSON structure: missing positions object\"}";
        }

        // Check if this FEN already exists
        std::string escapedFen = escapeJsonString(fen);
        std::string fenPattern = "\"" + escapedFen + "\"";
        bool fenExists = (jsonContent.find(fenPattern) != std::string::npos);

        // Get current timestamp
        std::string timestamp = getCurrentTimestamp();

        // Build the new position entry
        std::ostringstream newEntry;
        newEntry << "    \"" << escapedFen << "\": {\n";
        newEntry << "      \"best_move\": \"" << escapeJsonString(move) << "\",\n";
        newEntry << "      \"timestamp\": \"" << timestamp << "\"\n";
        newEntry << "    }";

        // Find where to insert/update
        size_t positionsObjectStart = jsonContent.find("{", positionsStart + 12);
        if (positionsObjectStart == std::string::npos)
        {
            return "{\"status\":\"error\",\"message\":\"Invalid JSON structure\"}";
        }

        size_t positionsObjectEnd = jsonContent.find("}", positionsObjectStart);
        std::string positionsContent = jsonContent.substr(positionsObjectStart + 1, positionsObjectEnd - positionsObjectStart - 1);

        // Build new positions content
        std::string newPositionsContent;
        if (positionsContent.find_first_not_of(" \t\n\r") == std::string::npos)
        {
            // Empty positions object
            newPositionsContent = "\n" + newEntry.str() + "\n  ";
        }
        else
        {
            // Has existing positions
            // Remove the old entry if it exists
            if (fenExists)
            {
                size_t oldEntryStart = jsonContent.find(fenPattern);
                if (oldEntryStart != std::string::npos)
                {
                    // Find the start of this entry (backtrack to find the key start)
                    size_t entryKeyStart = jsonContent.rfind("\"", oldEntryStart - 1);
                    size_t entryStart = jsonContent.rfind("\n", entryKeyStart);

                    // Find the end of this entry (forward to find the closing brace)
                    size_t entryEnd = jsonContent.find("}", oldEntryStart);
                    entryEnd = jsonContent.find_first_of(",\n", entryEnd);

                    if (entryStart != std::string::npos && entryEnd != std::string::npos)
                    {
                        // Check if there's a comma after
                        bool hasCommaAfter = (jsonContent[entryEnd] == ',');

                        // Remove old entry
                        std::string before = jsonContent.substr(0, entryStart + 1);
                        std::string after = jsonContent.substr(entryEnd + (hasCommaAfter ? 1 : 0));

                        jsonContent = before + after;
                    }
                }
            }

            // Re-find positions after potential modification
            positionsStart = jsonContent.find("\"positions\":");
            positionsObjectStart = jsonContent.find("{", positionsStart + 12);
            positionsObjectEnd = jsonContent.find("}", positionsObjectStart);
            positionsContent = jsonContent.substr(positionsObjectStart + 1, positionsObjectEnd - positionsObjectStart - 1);

            // Add new entry
            if (positionsContent.find_first_not_of(" \t\n\r") == std::string::npos)
            {
                newPositionsContent = "\n" + newEntry.str() + "\n  ";
            }
            else
            {
                newPositionsContent = positionsContent + ",\n" + newEntry.str() + "\n  ";
            }
        }

        // Rebuild JSON with new positions
        std::string beforePositions = jsonContent.substr(0, positionsObjectStart + 1);
        std::string afterPositions = jsonContent.substr(positionsObjectEnd);

        // Update count if new position
        int newCount = fenExists ? currentCount : currentCount + 1;

        // Rebuild the entire JSON
        std::ostringstream finalJson;
        finalJson << "{\n";
        finalJson << "  \"opening_name\": \"" << extractJsonStringValue(jsonContent, "opening_name") << "\",\n";
        finalJson << "  \"side\": \"" << extractJsonStringValue(jsonContent, "side") << "\",\n";
        finalJson << "  \"description\": \"" << extractJsonStringValue(jsonContent, "description") << "\",\n";
        finalJson << "  \"positions\": {" << newPositionsContent << "},\n";
        finalJson << "  \"stats\": {\n";
        finalJson << "    \"positions_learned\": " << newCount << ",\n";
        finalJson << "    \"last_updated\": \"" << timestamp << "\",\n";
        finalJson << "    \"deepest_line\": " << extractJsonNumberValue(jsonContent, "deepest_line") << "\n";
        finalJson << "  }\n";
        finalJson << "}\n";

        // Write back to file
        std::ofstream outFile(filename);
        if (!outFile.is_open())
        {
            return "{\"status\":\"error\",\"message\":\"Failed to write to file: " + filename + "\"}";
        }

        outFile << finalJson.str();
        outFile.close();

        std::cout << "Saved move for FEN: " << fen << " → move: " << move << std::endl;

        // Return success response
        return "{\"status\":\"ok\",\"positions_learned\":" + std::to_string(newCount) + "}";
    }
    catch (const std::exception &e)
    {
        std::string errorMsg = e.what();
        // Escape quotes in error message
        size_t pos = 0;
        while ((pos = errorMsg.find("\"", pos)) != std::string::npos)
        {
            errorMsg.replace(pos, 1, "\\\"");
            pos += 2;
        }
        return "{\"status\":\"error\",\"message\":\"" + errorMsg + "\"}";
    }
    catch (...)
    {
        return "{\"status\":\"error\",\"message\":\"Unknown error occurred while saving opening move\"}";
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
            return "{\"status\":\"error\",\"message\":\"Missing required parameters: opening, fen\"}";
        }

        std::cout << "Looking up FEN: " << fen << " in opening: " << opening << std::endl;

        // Build file path
        std::string filename = "openings/" + opening + ".json";

        // Read existing file
        std::ifstream inFile(filename);
        if (!inFile.is_open())
        {
            std::cout << "  → not found (file doesn't exist)" << std::endl;
            return "{\"status\":\"ok\",\"move\":\"\",\"found\":false}";
        }

        // Read entire file into string
        std::string jsonContent((std::istreambuf_iterator<char>(inFile)),
                                std::istreambuf_iterator<char>());
        inFile.close();

        // Escape the FEN for searching in JSON
        std::string escapedFen = escapeJsonString(fen);
        std::string fenPattern = "\"" + escapedFen + "\"";

        // Search for the FEN in the positions object
        size_t fenPos = jsonContent.find(fenPattern);
        if (fenPos == std::string::npos)
        {
            std::cout << "  → not found" << std::endl;
            return "{\"status\":\"ok\",\"move\":\"\",\"found\":false}";
        }

        // Find the "best_move" value for this FEN
        // Look for "best_move":"..." after the FEN position
        size_t bestMoveStart = jsonContent.find("\"best_move\":", fenPos);
        if (bestMoveStart == std::string::npos)
        {
            std::cout << "  → not found (no best_move field)" << std::endl;
            return "{\"status\":\"ok\",\"move\":\"\",\"found\":false}";
        }

        // Find the opening quote of the move value
        size_t moveValueStart = jsonContent.find("\"", bestMoveStart + 12);
        if (moveValueStart == std::string::npos)
        {
            std::cout << "  → not found (invalid JSON)" << std::endl;
            return "{\"status\":\"ok\",\"move\":\"\",\"found\":false}";
        }

        // Find the closing quote
        size_t moveValueEnd = jsonContent.find("\"", moveValueStart + 1);
        if (moveValueEnd == std::string::npos)
        {
            std::cout << "  → not found (invalid JSON)" << std::endl;
            return "{\"status\":\"ok\",\"move\":\"\",\"found\":false}";
        }

        // Extract the move
        std::string move = jsonContent.substr(moveValueStart + 1, moveValueEnd - moveValueStart - 1);

        // Verify this best_move belongs to the FEN we're looking for
        // (make sure we didn't find a best_move from a different position)
        // Check that the best_move is within a reasonable distance from the FEN
        if (bestMoveStart - fenPos > 200)
        {
            // Too far away, probably a different entry
            std::cout << "  → not found (best_move too far from FEN)" << std::endl;
            return "{\"status\":\"ok\",\"move\":\"\",\"found\":false}";
        }

        std::cout << "  → found: " << move << std::endl;

        // Return success response with move
        return "{\"status\":\"ok\",\"move\":\"" + escapeJsonString(move) + "\",\"found\":true}";
    }
    catch (const std::exception &e)
    {
        std::string errorMsg = e.what();
        // Escape quotes in error message
        size_t pos = 0;
        while ((pos = errorMsg.find("\"", pos)) != std::string::npos)
        {
            errorMsg.replace(pos, 1, "\\\"");
            pos += 2;
        }
        return "{\"status\":\"error\",\"message\":\"" + errorMsg + "\"}";
    }
    catch (...)
    {
        return "{\"status\":\"error\",\"message\":\"Unknown error occurred while retrieving opening move\"}";
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
            return "{\"status\":\"error\",\"message\":\"Missing required parameters: opening, fen\"}";
        }

        std::cout << "=== DELETE REQUEST ===" << std::endl;
        std::cout << "DEBUG: Raw FEN from request: " << fen << std::endl;
        std::cout << "DEBUG: Opening: " << opening << std::endl;

        // Build file path
        std::string filename = "openings/" + opening + ".json";
        std::cout << "DEBUG: File path: " << filename << std::endl;

        // Read existing file
        std::ifstream inFile(filename);
        if (!inFile.is_open())
        {
            std::cout << "ERROR: Could not open file: " << filename << std::endl;
            return "{\"status\":\"error\",\"message\":\"Opening file not found: " + filename + "\"}";
        }

        std::string jsonContent((std::istreambuf_iterator<char>(inFile)),
                                std::istreambuf_iterator<char>());
        inFile.close();
        std::cout << "DEBUG: File read successfully, size: " << jsonContent.length() << " bytes" << std::endl;

        // Escape the FEN for searching
        std::string escapedFen = escapeJsonString(fen);
        std::string fenPattern = "\"" + escapedFen + "\"";
        std::cout << "DEBUG: Escaped FEN for search: " << escapedFen << std::endl;
        std::cout << "DEBUG: FEN pattern to find: " << fenPattern << std::endl;

        // Find the FEN in the JSON
        size_t fenPos = jsonContent.find(fenPattern);
        std::cout << "DEBUG: Search result position: " << fenPos << std::endl;

        if (fenPos == std::string::npos)
        {
            std::cout << "DEBUG: FEN NOT FOUND in JSON!" << std::endl;
            std::cout << "DEBUG: JSON content (first 500 chars): " << jsonContent.substr(0, 500) << std::endl;
            std::cout << "  → Position not in book (nothing to delete)" << std::endl;
            return "{\"status\":\"ok\",\"message\":\"Position was not in book\"}";
        }

        std::cout << "DEBUG: FEN FOUND at position: " << fenPos << std::endl;

        // Find the entry boundaries
        // Backtrack to find the start of this line (find the opening quote before the FEN)
        size_t lineStart = jsonContent.rfind("\n", fenPos);
        if (lineStart == std::string::npos)
            lineStart = 0;

        // Skip any leading whitespace on this line
        while (lineStart < fenPos && (jsonContent[lineStart] == '\n' || jsonContent[lineStart] == ' ' || jsonContent[lineStart] == '\t'))
        {
            lineStart++;
        }

        // Forward to find the end of this entry (closing brace)
        size_t entryEnd = jsonContent.find("}", fenPos);
        if (entryEnd == std::string::npos)
        {
            return "{\"status\":\"error\",\"message\":\"Malformed JSON: could not find entry end\"}";
        }

        // Check if there's a comma after the closing brace
        size_t checkPos = entryEnd + 1;
        bool hasCommaAfter = false;

        while (checkPos < jsonContent.length() && (jsonContent[checkPos] == ' ' || jsonContent[checkPos] == '\t' || jsonContent[checkPos] == '\n'))
        {
            checkPos++;
        }

        if (checkPos < jsonContent.length() && jsonContent[checkPos] == ',')
        {
            hasCommaAfter = true;
            entryEnd = checkPos; // Include the comma in deletion
        }

        // If no comma after, check if there's a comma before (last entry in object)
        if (!hasCommaAfter)
        {
            size_t checkBefore = lineStart;
            while (checkBefore > 0 && (jsonContent[checkBefore - 1] == ' ' || jsonContent[checkBefore - 1] == '\t' || jsonContent[checkBefore - 1] == '\n'))
            {
                checkBefore--;
            }

            if (checkBefore > 0 && jsonContent[checkBefore - 1] == ',')
            {
                lineStart = checkBefore - 1; // Include preceding comma
            }
        }

        // Delete the entry (from lineStart to entryEnd inclusive)
        std::cout << "DEBUG: Entry starts at: " << lineStart << std::endl;
        std::cout << "DEBUG: Entry ends at: " << entryEnd << std::endl;
        std::cout << "DEBUG: Has comma after: " << hasCommaAfter << std::endl;
        std::cout << "DEBUG: Deleting range: [" << lineStart << " to " << entryEnd << "] (length: " << (entryEnd - lineStart + 1) << ")" << std::endl;
        std::cout << "DEBUG: Content to delete: " << jsonContent.substr(lineStart, std::min((size_t)200, entryEnd - lineStart + 1)) << std::endl;

        jsonContent.erase(lineStart, entryEnd - lineStart + 1);
        std::cout << "DEBUG: Entry erased from JSON string" << std::endl;

        // Update positions_learned count (decrement by 1)
        int currentCount = extractJsonNumberValue(jsonContent, "positions_learned");
        if (currentCount > 0)
            currentCount--;

        // Replace the count in the JSON
        std::string countPattern = "\"positions_learned\":";
        size_t countPos = jsonContent.find(countPattern);

        if (countPos != std::string::npos)
        {
            size_t valueStart = countPos + countPattern.length();

            // Skip whitespace
            while (valueStart < jsonContent.length() && (jsonContent[valueStart] == ' ' || jsonContent[valueStart] == '\t'))
                valueStart++;

            size_t valueEnd = valueStart;

            // Find end of number
            while (valueEnd < jsonContent.length() && jsonContent[valueEnd] >= '0' && jsonContent[valueEnd] <= '9')
                valueEnd++;

            // Replace the old count with new count
            jsonContent.replace(valueStart, valueEnd - valueStart, std::to_string(currentCount));
        }

        // Update last_updated timestamp
        std::string timestamp = getCurrentTimestamp();
        std::string lastUpdatedPattern = "\"last_updated\":\"";
        size_t lastUpdatedPos = jsonContent.find(lastUpdatedPattern);

        if (lastUpdatedPos != std::string::npos)
        {
            size_t timestampStart = lastUpdatedPos + lastUpdatedPattern.length();
            size_t timestampEnd = jsonContent.find("\"", timestampStart);

            if (timestampEnd != std::string::npos)
            {
                jsonContent.replace(timestampStart, timestampEnd - timestampStart, timestamp);
            }
        }

        // Write back to file
        std::cout << "DEBUG: Writing modified JSON back to file..." << std::endl;
        std::ofstream outFile(filename);
        if (!outFile.is_open())
        {
            std::cout << "ERROR: Could not open file for writing: " << filename << std::endl;
            return "{\"status\":\"error\",\"message\":\"Could not write to file\"}";
        }

        outFile << jsonContent;
        outFile.close();
        std::cout << "DEBUG: File written successfully" << std::endl;

        // Verify deletion
        std::cout << "DEBUG: Verifying deletion..." << std::endl;
        std::ifstream verifyFile(filename);
        std::string verifyContent((std::istreambuf_iterator<char>(verifyFile)),
                                   std::istreambuf_iterator<char>());
        verifyFile.close();

        if (verifyContent.find(escapedFen) != std::string::npos)
        {
            std::cout << "ERROR: FEN still exists after deletion!" << std::endl;
            std::cout << "VERIFICATION FAILED!" << std::endl;
        }
        else
        {
            std::cout << "VERIFIED: FEN successfully deleted from file" << std::endl;
        }

        std::cout << "  → Position deleted! Positions remaining: " << currentCount << std::endl;
        std::cout << "=== DELETE COMPLETE ===" << std::endl;

        return "{\"status\":\"ok\",\"message\":\"Position deleted\",\"positions_learned\":" + std::to_string(currentCount) + "}";
    }
    catch (const std::exception &e)
    {
        return "{\"status\":\"error\",\"message\":\"Exception: " + std::string(e.what()) + "\"}";
    }
    catch (...)
    {
        return "{\"status\":\"error\",\"message\":\"Unknown error occurred while deleting position\"}";
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
    std::string body = "{\"status\":\"error\",\"message\":\"Endpoint not found. Available endpoints: GET /move?fen=..., GET /legal-moves?fen=..., GET /save-opening-move?opening=...&fen=...&move=..., GET /get-opening-move?opening=...&fen=..., GET /delete-opening-move?opening=...&fen=...\"}";
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

    // Startup logging for Render deployment detection
    std::cout << "Server listening on 0.0.0.0:" << PORT << std::endl;
    std::cout << "Ready to accept connections" << std::endl;
    std::cout << "Endpoints:" << std::endl;
    std::cout << "  GET /move?fen=<FEN_STRING>" << std::endl;
    std::cout << "  GET /legal-moves?fen=<FEN_STRING>" << std::endl;
    std::cout << "Press Ctrl+C to stop\n"
              << std::endl;
    std::flush(std::cout); // Force output to appear immediately

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
