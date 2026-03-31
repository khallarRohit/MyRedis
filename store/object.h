#pragma once
#include <string>
#include <chrono>
#include <variant>

namespace MyRedis{

    enum class DataType {
        STRING,
        HASH,
        SET,
        ZSET,
        LIST
    };

    class RedisObject{
    protected:
        bool hasExpiry{false};
        std::chrono::time_point<std::chrono::system_clock> expiresAt;
    
    public:
        virtual ~RedisObject() = default;

        virtual DataType getType() const = 0;

        void setExpiry(int milliseconds);

        bool isExpired() const;
    };
}