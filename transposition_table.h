#ifndef TRANSPOSITION_TABLE_H
#define TRANSPOSITION_TABLE_H

#include <cstdint>
#include <vector>
#include <string>

// Bound type for transposition table entries
enum BoundType {
    EXACT,       // Exact score (PV node)
    LOWER_BOUND, // Beta cutoff (score >= beta)
    UPPER_BOUND  // Alpha not improved (score <= alpha)
};

// Transposition table entry structure
struct TTEntry {
    uint64_t hash;        // Full hash for verification
    int depth;            // Search depth of this entry
    int score;            // Evaluation score
    std::string bestMove; // Best move found
    BoundType bound;      // Type of bound

    TTEntry() : hash(0), depth(-1), score(0), bestMove(""), bound(EXACT) {}
};

// Transposition table class
class TranspositionTable {
private:
    static const int TABLE_SIZE = 1048576;  // 1 million entries (~64MB)
    std::vector<TTEntry> table;

    // Statistics
    long long hits;
    long long misses;
    long long collisions;
    long long stores;

public:
    TranspositionTable() : table(TABLE_SIZE), hits(0), misses(0), collisions(0), stores(0) {}

    // Store an entry in the transposition table
    void store(uint64_t hash, int depth, int score,
               const std::string& bestMove, BoundType bound) {
        int index = hash % TABLE_SIZE;
        TTEntry& entry = table[index];

        stores++;

        // Replacement scheme: replace if:
        // 1. Empty slot (depth == -1)
        // 2. Same position (hash match)
        // 3. Deeper or equal depth search
        if (entry.depth == -1 || entry.hash == hash || depth >= entry.depth) {
            // Track collisions (replacing a different position)
            if (entry.depth != -1 && entry.hash != hash) {
                collisions++;
            }

            entry.hash = hash;
            entry.depth = depth;
            entry.score = score;
            entry.bestMove = bestMove;
            entry.bound = bound;
        }
    }

    // Probe the transposition table
    TTEntry* probe(uint64_t hash) {
        int index = hash % TABLE_SIZE;
        TTEntry& entry = table[index];

        if (entry.hash == hash) {
            hits++;
            return &entry;
        }

        misses++;
        return nullptr;
    }

    // Clear the transposition table
    void clear() {
        table.assign(TABLE_SIZE, TTEntry());
        hits = 0;
        misses = 0;
        collisions = 0;
        stores = 0;
    }

    // Get statistics
    long long getHits() const { return hits; }
    long long getMisses() const { return misses; }
    long long getCollisions() const { return collisions; }
    long long getStores() const { return stores; }

    double getHitRate() const {
        long long total = hits + misses;
        return total > 0 ? (100.0 * hits) / total : 0.0;
    }

    int getUsedEntries() const {
        int used = 0;
        for (const TTEntry& entry : table) {
            if (entry.depth != -1) used++;
        }
        return used;
    }

    double getTableFillRate() const {
        return (100.0 * getUsedEntries()) / TABLE_SIZE;
    }
};

#endif
