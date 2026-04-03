#include "redisstring.h"

namespace MyRedis{

    RedisString::RedisString(const std::string& val)
    :value(val){};

    DataType RedisString::getType() const{
        return DataType::STRING;
    }
    
    std::string RedisString::get() const{
        return value;
    }
    
    void RedisString::set(const std::string& val){
        value = val;
    }

    std::string RedisString::substr(int start, int stop) const {
        int len = value.length();
        if (len == 0) return "";

        // Convert negative indices
        if (start < 0) start = len + start;
        if (stop < 0) stop = len + stop;

        // Clamp out-of-bounds indices
        if (start < 0) start = 0;
        if (start > stop || start >= len) return "";
        if (stop >= len) stop = len - 1;

        return value.substr(start, stop - start + 1);
    }

}