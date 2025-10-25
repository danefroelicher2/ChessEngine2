#include "database.h"
#include <iostream>
#include <sstream>

Database::Database(const std::string& connectionString)
    : connectionString_(connectionString), conn_(nullptr) {
}

Database::~Database() {
    disconnect();
}

bool Database::connect() {
    // Establish connection
    conn_ = PQconnectdb(connectionString_.c_str());

    // Check connection status
    if (PQstatus(conn_) != CONNECTION_OK) {
        std::cerr << "Connection to database failed: " << PQerrorMessage(conn_) << std::endl;
        PQfinish(conn_);
        conn_ = nullptr;
        return false;
    }

    // Create table if not exists
    std::stringstream createTableQuery;
    createTableQuery << "CREATE TABLE IF NOT EXISTS opening_book ("
                     << "id SERIAL PRIMARY KEY, "
                     << "opening_name VARCHAR(100) NOT NULL, "
                     << "fen TEXT NOT NULL, "
                     << "best_move VARCHAR(10) NOT NULL, "
                     << "timestamp TIMESTAMP DEFAULT CURRENT_TIMESTAMP, "
                     << "UNIQUE(opening_name, fen)"
                     << ");";

    PGresult* res = PQexec(conn_, createTableQuery.str().c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Failed to create table: " << PQerrorMessage(conn_) << std::endl;
        PQclear(res);
        return false;
    }
    PQclear(res);

    // Create index
    std::string createIndexQuery = "CREATE INDEX IF NOT EXISTS idx_opening_fen ON opening_book(opening_name, fen);";
    res = PQexec(conn_, createIndexQuery.c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Failed to create index: " << PQerrorMessage(conn_) << std::endl;
        PQclear(res);
        return false;
    }
    PQclear(res);

    return true;
}

void Database::disconnect() {
    if (conn_ != nullptr) {
        PQfinish(conn_);
        conn_ = nullptr;
    }
}

std::string Database::escapeString(const std::string& str) {
    if (conn_ == nullptr) {
        return "''";
    }

    char* escaped = PQescapeLiteral(conn_, str.c_str(), str.length());
    if (escaped == nullptr) {
        std::cerr << "Failed to escape string: " << PQerrorMessage(conn_) << std::endl;
        return "''";
    }

    std::string result(escaped);
    PQfreemem(escaped);
    return result;
}

bool Database::saveOpeningMove(const std::string& opening, const std::string& fen, const std::string& move) {
    if (conn_ == nullptr) {
        std::cerr << "Database not connected" << std::endl;
        return false;
    }

    std::stringstream query;
    query << "INSERT INTO opening_book (opening_name, fen, best_move) VALUES ("
          << escapeString(opening) << ", "
          << escapeString(fen) << ", "
          << escapeString(move)
          << ") ON CONFLICT (opening_name, fen) "
          << "DO UPDATE SET best_move = " << escapeString(move)
          << ", timestamp = CURRENT_TIMESTAMP;";

    PGresult* res = PQexec(conn_, query.str().c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Failed to save opening move: " << PQerrorMessage(conn_) << std::endl;
        PQclear(res);
        return false;
    }
    PQclear(res);

    return true;
}

std::string Database::getOpeningMove(const std::string& opening, const std::string& fen) {
    if (conn_ == nullptr) {
        std::cerr << "Database not connected" << std::endl;
        return "";
    }

    std::stringstream query;
    query << "SELECT best_move FROM opening_book WHERE opening_name = "
          << escapeString(opening) << " AND fen = " << escapeString(fen) << ";";

    PGresult* res = PQexec(conn_, query.str().c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Failed to get opening move: " << PQerrorMessage(conn_) << std::endl;
        PQclear(res);
        return "";
    }

    std::string move = "";
    if (PQntuples(res) > 0) {
        move = PQgetvalue(res, 0, 0);
    }

    PQclear(res);
    return move;
}

bool Database::deleteOpeningMove(const std::string& opening, const std::string& fen) {
    if (conn_ == nullptr) {
        std::cerr << "Database not connected" << std::endl;
        return false;
    }

    std::stringstream query;
    query << "DELETE FROM opening_book WHERE opening_name = "
          << escapeString(opening) << " AND fen = " << escapeString(fen) << ";";

    PGresult* res = PQexec(conn_, query.str().c_str());
    if (PQresultStatus(res) != PGRES_COMMAND_OK) {
        std::cerr << "Failed to delete opening move: " << PQerrorMessage(conn_) << std::endl;
        PQclear(res);
        return false;
    }
    PQclear(res);

    return true;
}

int Database::getPositionsLearned(const std::string& opening) {
    if (conn_ == nullptr) {
        std::cerr << "Database not connected" << std::endl;
        return 0;
    }

    std::stringstream query;
    query << "SELECT COUNT(*) FROM opening_book WHERE opening_name = "
          << escapeString(opening) << ";";

    PGresult* res = PQexec(conn_, query.str().c_str());
    if (PQresultStatus(res) != PGRES_TUPLES_OK) {
        std::cerr << "Failed to get position count: " << PQerrorMessage(conn_) << std::endl;
        PQclear(res);
        return 0;
    }

    int count = 0;
    if (PQntuples(res) > 0) {
        count = std::atoi(PQgetvalue(res, 0, 0));
    }

    PQclear(res);
    return count;
}
