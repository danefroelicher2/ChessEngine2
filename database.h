#ifndef DATABASE_H
#define DATABASE_H

#include <string>
#include <libpq-fe.h>

class Database {
public:
    Database(const std::string& connectionString);
    ~Database();

    // Connection management
    bool connect();
    void disconnect();

    // Opening book operations
    bool saveOpeningMove(const std::string& opening, const std::string& fen, const std::string& move);
    std::string getOpeningMove(const std::string& opening, const std::string& fen);
    bool deleteOpeningMove(const std::string& opening, const std::string& fen);
    int getPositionsLearned(const std::string& opening);

private:
    std::string connectionString_;
    PGconn* conn_;

    std::string escapeString(const std::string& str);
};

#endif
