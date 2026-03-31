#include "redisstring.h"

namespace MyRedis{

    RedisString::RedisString(const std::string& val)
    :value(val){};

    DataType RedisString::getType() const{
        return DataType::STRING;
    }
    
    std::string RedisString::getValue() const{
        return value;
    }
    
    void RedisString::setValue(const std::string& val){
        value = val;
    }

}