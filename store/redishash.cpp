#include "redishash.h"

namespace MyRedis{
    DataType RedisHash::getType() const { 
        return DataType::HASH; 
    }

    int RedisHash::hset(const std::string& field, const std::string& value) {
        // Check if it's a new field to mimic standard Redis integer replies
        std::string* existing = internalMap.find(field);
        int isNewField = (existing == nullptr) ? 1 : 0;
        
        internalMap.insert(field, value);
        return isNewField;
    }

    std::optional<std::string> RedisHash::hget(const std::string& field) const {
        // Your custom find() returns a pointer, making this very easy!
        std::string* valPtr = internalMap.find(field);
        if (valPtr != nullptr) {
            return *valPtr;
        }
        return std::nullopt;
    }

    size_t RedisHash::hdel(const std::string& field) {
        return internalMap.erase(field);
    }

}