#pragma once
#include "object.h"
#include "hashmap.h" 
#include <string>
#include <vector>

namespace MyRedis {

    class RedisSet : public RedisObject {
    private:
        // The Key is the set member, the Value is just a dummy boolean.
        HashMap<std::string, bool> internalMap;

    public:
        RedisSet() = default;
        ~RedisSet() override = default;

        DataType getType() const override;

        int sadd(const std::vector<std::string>& members);
        int srem(const std::vector<std::string>& members);
        size_t scard() const;
        int sismember(const std::string& member) const;
    };

}