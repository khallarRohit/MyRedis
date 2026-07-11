#include "redisset.h"

namespace MyRedis {

    DataType RedisSet::getType() const { 
        return DataType::SET; 
    }

    int RedisSet::sadd(const std::vector<std::string>& members) {
        int added = 0;
        for (const auto& member : members) {
            // ZERO REDUNDANCY: We attempt the insert in a single pass.
            // If another thread adds it a microsecond before us, 'inserted' will safely be false.
            auto [existing_opt, inserted] = internalMap.insert(member, true);
            
            if (inserted) {
                added++;
            }
        }
        return added;
    }

    int RedisSet::srem(const std::vector<std::string>& members) {
        int removed = 0;
        for (const auto& member : members) {
            // map.erase() handles the bucket-level unique_lock internally
            removed += internalMap.erase(member);
        }
        return removed;
    }

    size_t RedisSet::scard() const {
        // Atomic read from the map
        return internalMap.size(); 
    }

    int RedisSet::sismember(const std::string& member) const {
        // Safe lock-free read using the bucket's shared_lock
        return internalMap.find(member).has_value() ? 1 : 0;
    }
}