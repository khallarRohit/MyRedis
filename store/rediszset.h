#pragma once
#include "object.h"
#include "hashmap.h"
#include "map.h"
#include <string>
#include <vector>
#include <optional>

namespace MyRedis {

    struct ZSetKey {
        double score;
        std::string member;

        // How the Red-Black tree knows how to sort them
        bool operator<(const ZSetKey& other) const;

        // How the Red-Black tree knows they are exactly the same
        bool operator==(const ZSetKey& other) const;
    };

    class RedisZSet : public RedisObject {
    private:
        // O(1) lookups: "What is Alice's score?"
        HashMap<std::string, double> memberScores; 
        
        // O(log N) sorting: The tree holding the ordered data
        Map<ZSetKey, bool> orderedTree;

    public:
        RedisZSet() = default;
        ~RedisZSet() override = default;

        DataType getType() const override;

        int zadd(double score, const std::string& member);
        std::optional<double> zscore(const std::string& member) const;
        std::vector<std::string> zrange(int start, int stop);
        size_t zcard() const;
        size_t zcount(double min, double max);
        std::optional<int> zrank(const std::string& member);
    };
}