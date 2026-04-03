#include "redishash.h"

namespace MyRedis{
    DataType RedisHash::getType() const { 
        return DataType::HASH; 
    }

    int RedisHash::hset(const std::string& field, const std::string& value) {
        std::string* existing = internalMap.find(field);
        int isNewField = (existing == nullptr) ? 1 : 0;
        
        internalMap.insert(field, value);
        return isNewField;
    }

    std::optional<std::string> RedisHash::hget(const std::string& field) const {
        std::string* valPtr = internalMap.find(field);
        if (valPtr != nullptr) {
            return *valPtr;
        }
        return std::nullopt;
    }

    size_t RedisHash::hdel(const std::string& field) {
        return internalMap.erase(field);
    }

    int RedisHash::hexists(const std::string& field) const {
        return (internalMap.find(field) != nullptr) ? 1 : 0;
    }

    size_t RedisHash::hlen() const {
        return internalMap.size();
    }

    std::optional<std::string> RedisHash::hgetdel(const std::string& field) {
        std::string* valPtr = internalMap.find(field);
        if (valPtr != nullptr) {
            std::string valueCopy = *valPtr; 
            internalMap.erase(field);
            return valueCopy;
        }
        return std::nullopt;
    }

}