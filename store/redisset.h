#pragma once
#include "object.h"
#include "map.h" // Your custom Red-Black Tree
#include <string>
#include <vector>

namespace MyRedis {

    class RedisSet : public RedisObject {
    private:
        // The Key is the set member, the Value is just a dummy boolean.
        Map<std::string, bool> internalSet;

    public:
        RedisSet() = default;
        ~RedisSet() override = default;

        // Identifies this object safely to the global keyspace
        DataType getType() const override;

        // --- REDIS COMMAND: SADD ---
        // Adds multiple members. Returns the number of elements that were added 
        // (does not count elements that already existed).
        int sadd(const std::vector<std::string>& members);

        // --- REDIS COMMAND: SISMEMBER ---
        // Returns 1 if the member exists, 0 if it does not.
        int sismember(const std::string& member);
        
        // --- REDIS COMMAND: SREM ---
        // Removes members. Returns the number of elements successfully removed.
        int srem(const std::vector<std::string>& members);
    };

}