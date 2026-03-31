#pragma once
#include "object.h"
#include <string>
#include <vector>
#include <deque>
#include <optional>

namespace MyRedis{

    class RedisList : public RedisObject{
    private:
        std::deque<std::string> internalList;

    public:
        RedisList() = default;
        ~RedisList() override = default;

        DataType getType() const override;

        size_t size() const;

        int lpush(const std::vector<std::string>& elements);
        
        int rpush(const std::vector<std::string>& elements);
        std::optional<std::string> lpop();
        std::optional<std::string> rpop();

        std::vector<std::string> lrange(int start, int stop) const;
    };

}