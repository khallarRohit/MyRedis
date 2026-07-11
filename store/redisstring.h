#pragma once
#include "object.h"
#include <atomic>
#include <memory>

namespace MyRedis{

    class RedisString : public RedisObject{
    private:
        std::atomic<std::shared_ptr<const std::string>> value_ptr;
    public:
        RedisString(const std::string& val);
        ~RedisString() override = default;

        DataType getType() const override;

        std::string get() const;
        void set(const std::string& val);
        std::string substr(int start, int stop) const;
    };

}