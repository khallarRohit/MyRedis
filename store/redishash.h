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

        DataType getType() const override { 
            return DataType::HASH; 
        }

        int hset(const std::string& field, const std::string& value) {
            // Check if it's a new field to mimic standard Redis integer replies
            std::string* existing = internalMap.find(field);
            int isNewField = (existing == nullptr) ? 1 : 0;
            
            internalMap.insert(field, value);
            return isNewField;
        }

        std::optional<std::string> hget(const std::string& field) const {
            // Your custom find() returns a pointer, making this very easy!
            std::string* valPtr = internalMap.find(field);
            if (valPtr != nullptr) {
                return *valPtr;
            }
            return std::nullopt;
        }

        size_t hdel(const std::string& field) {
            return internalMap.erase(field);
        }

    };


}