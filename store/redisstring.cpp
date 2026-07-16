#include "redisstring.h"

namespace MyRedis{

    RedisString::RedisString(const std::string& val){
        value_ptr.store(std::make_shared<const std::string>(val), std::memory_order_relaxed);
    }

    DataType RedisString::getType() const{
        return DataType::STRING;
    }
    
    std::string RedisString::get() const{
        std::shared_ptr<const std::string> current_ptr = value_ptr.load(std::memory_order_acquire);

        return *current_ptr;
    }
    
    void RedisString::set(const std::string& val){
        std::shared_ptr<const std::string> new_ptr = std::make_shared<const std::string>(val);

        value_ptr.store(new_ptr, std::memory_order_release);
    }

    std::string RedisString::substr(int start, int stop) const {
        auto current_ptr = value_ptr.load(std::memory_order_acquire);
        const std::string& val = *current_ptr;

        int len = val.length();
        if (len == 0) return "";

        if (start < 0) start = len + start;
        if (stop < 0) stop = len + stop;

        if (start < 0) start = 0;
        if (start > stop || start >= len) return "";
        if (stop >= len) stop = len - 1;

        return val.substr(start, stop - start + 1);
    }

}