#pragma once
#include "object.h"
#include "hashmap.h" // Your custom hash map
#include <string>
#include <optional>

namespace MyRedis{

    class RedisHash : public RedisObject{
    private:
        HashMap<std::string, std::string> internalMap;
    public:
        RedisHash() = default;
        ~RedisHash() override = default;

        DataType getType() const override;

        int hset(const std::string& field, const std::string& value);
        std::optional<std::string> hget(const std::string& field) const;
        size_t hdel(const std::string& field);
        int hexists(const std::string& field) const;
        size_t hlen() const;
        std::optional<std::string> hgetdel(const std::string& field);
    };


}