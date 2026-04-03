#include "redisset.h"

namespace MyRedis{

    DataType RedisSet::getType() const { 
        return DataType::SET; 
    }

    int RedisSet::sadd(const std::vector<std::string>& members) {
        int added = 0;
        for (const auto& member : members) {
            if (internalMap.find(member) == nullptr) {
                internalMap.insert(member, true);
                added++;
            }
        }
        return added;
    }

    int RedisSet::srem(const std::vector<std::string>& members) {
        int removed = 0;
        for (const auto& member : members) {
            removed += internalMap.erase(member);
        }
        return removed;
    }

    size_t RedisSet::scard() const {
        return internalMap.size(); 
    }

    int RedisSet::sismember(const std::string& member) const {
        return (internalMap.find(member) != nullptr) ? 1 : 0;
    }
}