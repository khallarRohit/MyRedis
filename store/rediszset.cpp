#include "rediszset.h" 

namespace MyRedis{
    bool ZSetKey::operator<(const ZSetKey& other) const {
        if (score != other.score) return score < other.score;
        return member < other.member; 
    }

    bool ZSetKey::operator<=(const ZSetKey& other) const {
        if (score != other.score) return score < other.score;
        return member <= other.member; 
    }

    // How the Red-Black tree knows they are exactly the same
    bool ZSetKey::operator==(const ZSetKey& other) const {
        return score == other.score && member == other.member;
    }


    DataType RedisZSet::getType() const { return DataType::ZSET; }

    // --- COMMAND: ZADD ---
    // Adds or updates a member. Returns 1 if new, 0 if updated.
    int RedisZSet::zadd(double score, const std::string& member) {
        double* existingScore = memberScores.find(member);
        
        if (existingScore != nullptr) {
            // Member already exists. If score is the same, do nothing.
            if (*existingScore == score) return 0;

            // Score changed! Remove old entry from the Tree
            orderedTree.erase({*existingScore, member});
        }

        // Insert new data into both structures
        memberScores.insert(member, score);
        orderedTree.insert({score, member}, true);
        
        return (existingScore == nullptr) ? 1 : 0;
    }

    // --- COMMAND: ZSCORE ---
    std::optional<double> RedisZSet::zscore(const std::string& member) const {
        double* score = memberScores.find(member);
        if (score != nullptr) return *score;
        return std::nullopt;
    }

    // --- COMMAND: ZRANGE ---
    std::vector<std::string> RedisZSet::zrange(int start, int stop) {
        // Get all keys perfectly sorted from your Red-Black tree
        std::vector<ZSetKey> sortedKeys = orderedTree.getSortedKeys();
        int len = sortedKeys.size();
        if (len == 0) return {};

        // Handle negative indices (e.g., -1 is the last element)
        if (start < 0) start = len + start;
        if (stop < 0) stop = len + stop;

        if (start < 0) start = 0;
        if (start > stop || start >= len) return {};
        if (stop >= len) stop = len - 1;

        std::vector<std::string> result;
        for (int i = start; i <= stop; i++) {
            result.push_back(sortedKeys[i].member);
        }
        return result;
    }

    size_t RedisZSet::zcard() const {
        return memberScores.size();
    }

    size_t RedisZSet::zcount(double min, double max) {
        std::vector<ZSetKey> sortedKeys = orderedTree.getSortedKeys();
        size_t count = 0;
        for (const auto& key : sortedKeys) {
            if (key.score >= min && key.score <= max) {
                count++;
            }
        }
        return count;
    }

    std::optional<int> RedisZSet::zrank(const std::string& member) {
        // O(1) check to see if it even exists before traversing the tree
        double* scorePtr = memberScores.find(member);
        if (scorePtr == nullptr) return std::nullopt;

        std::vector<ZSetKey> sortedKeys = orderedTree.getSortedKeys();
        for (size_t i = 0; i < sortedKeys.size(); i++) {
            if (sortedKeys[i].member == member) {
                return static_cast<int>(i);
            }
        }
        return std::nullopt;
    }

}
        
