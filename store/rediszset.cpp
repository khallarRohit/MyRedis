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

    bool ZSetKey::operator==(const ZSetKey& other) const {
        return score == other.score && member == other.member;
    }


    DataType RedisZSet::getType() const { return DataType::ZSET; }

    int RedisZSet::zadd(double score, const std::string& member) {
        std::unique_lock<std::shared_mutex> lock(zsetMutex);
        
        auto existingScoreOpt = memberScores.find(member);
        
        if (existingScoreOpt.has_value()) {
            double existingScore = existingScoreOpt.value();
            
            if (existingScore == score) return 0;

            orderedTree.erase({existingScore, member});
        }

        memberScores.insert(member, score);
        orderedTree.insert({score, member}, true);
        
        return existingScoreOpt.has_value() ? 0 : 1;
    }

    std::optional<double> RedisZSet::zscore(const std::string& member) const {
        return memberScores.find(member); 
    }

    size_t RedisZSet::zcard() const {
        return memberScores.size();
    }

    std::vector<std::string> RedisZSet::zrange(int start, int stop) const {
        std::shared_lock<std::shared_mutex> lock(zsetMutex);
        
        std::vector<ZSetKey> sortedKeys = orderedTree.getSortedKeys();
        int len = sortedKeys.size();
        if (len == 0) return {};

        if (start < 0) start = len + start;
        if (stop < 0) stop = len + stop;

        if (start < 0) start = 0;
        if (start > stop || start >= len) return {};
        if (stop >= len) stop = len - 1;

        std::vector<std::string> result;
        result.reserve(stop - start + 1);
        for (int i = start; i <= stop; i++) {
            result.push_back(sortedKeys[i].member);
        }
        return result;
    }

    size_t RedisZSet::zcount(double min, double max) const {
        std::shared_lock<std::shared_mutex> lock(zsetMutex);
        
        std::vector<ZSetKey> sortedKeys = orderedTree.getSortedKeys();
        size_t count = 0;
        for (const auto& key : sortedKeys) {
            if (key.score >= min && key.score <= max) {
                count++;
            }
        }
        return count;
    }

    std::optional<int> RedisZSet::zrank(const std::string& member) const {
        if (!memberScores.find(member).has_value()) {
            return std::nullopt;
        }

        std::shared_lock<std::shared_mutex> lock(zsetMutex);
        
        std::vector<ZSetKey> sortedKeys = orderedTree.getSortedKeys();
        for (size_t i = 0; i < sortedKeys.size(); i++) {
            if (sortedKeys[i].member == member) {
                return static_cast<int>(i);
            }
        }
        return std::nullopt;
    }
}