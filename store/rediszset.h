#pragma once
#include "object.h"
#include "hashmap.h" // The ConcurrentHashMap
#include "map.h"     // The Red-Black Tree
#include <string>
#include <vector>
#include <optional>
#include <memory>
#include <mutex>
#include <shared_mutex>

namespace MyRedis {

    struct ZSetKey {
        double score;
        std::string member;

        bool operator<(const ZSetKey& other) const;
        bool operator<=(const ZSetKey& other) const;
        bool operator==(const ZSetKey& other) const;
    };

    class RedisZSet : public RedisObject {
    private:
        HashMap<std::string, double> memberScores; 
        
        Map<ZSetKey, bool> orderedTree;

        mutable std::shared_mutex zsetMutex;

    public:
        RedisZSet() = default;
        ~RedisZSet() override = default;

        DataType getType() const override;

        int zadd(double score, const std::string& member);
        std::optional<double> zscore(const std::string& member) const;
        std::vector<std::string> zrange(int start, int stop) const;
        size_t zcard() const;
        size_t zcount(double min, double max) const;
        std::optional<int> zrank(const std::string& member) const;
    };
}