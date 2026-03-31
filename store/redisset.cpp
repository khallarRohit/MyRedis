#include "redisset.h"

namespace MyRedis{

    DataType RedisSet::getType() const{ 
        return DataType::SET; 
    }

    // --- REDIS COMMAND: SADD ---
    // Adds multiple members. Returns the number of elements that were added 
    // (does not count elements that already existed).
    int RedisSet::sadd(const std::vector<std::string>& members) {
        int elementsAdded = 0;
        for (const auto& member : members) {
            // Your find() returns false if not found
            if (!internalSet.find(member)) { 
                internalSet.insert(member, true);
                elementsAdded++;
            }
        }
        return elementsAdded;
    }

    // --- REDIS COMMAND: SISMEMBER ---
    // Returns 1 if the member exists, 0 if it does not.
    int RedisSet::sismember(const std::string& member) {
        return internalSet.find(member) ? 1 : 0;
    }

    // --- REDIS COMMAND: SREM ---
    // Removes members. Returns the number of elements successfully removed.
    int RedisSet::srem(const std::vector<std::string>& members) {
        int elementsRemoved = 0;
        for (const auto& member : members) {
            if (internalSet.find(member)) {
                internalSet.erase(member);
                elementsRemoved++;
            }
        }
        return elementsRemoved;
    }
}