#include "redishash.h"

namespace MyRedis{
    DataType RedisHash::getType() const { 
        return DataType::HASH; 
    }

    int RedisHash::hset(const std::string& field, const std::string& value) {
        auto new_val = std::make_shared<AtomicValue>(value);
        
        auto [existing_opt, inserted] = internalMap.insert(field, new_val);
        
        if (!inserted && existing_opt.has_value()) {
            auto new_str_ptr = std::make_shared<const std::string>(value);
            existing_opt.value()->ptr.store(new_str_ptr, std::memory_order_release);
            return 0;
        }
        
        return 1;
    }

    std::optional<std::string> RedisHash::hget(const std::string& field) const {
        auto valOpt = internalMap.find(field);
        if (valOpt.has_value()) {
            auto str_ptr = valOpt.value()->ptr.load(std::memory_order_acquire);
            return *str_ptr;
        }
        return std::nullopt;
    }

    size_t RedisHash::hdel(const std::string& field) {
        return internalMap.erase(field);
    }

    int RedisHash::hexists(const std::string& field) const {
        return internalMap.find(field).has_value() ? 1 : 0;
    }

    size_t RedisHash::hlen() const {
        return internalMap.size();
    }

    std::optional<std::string> RedisHash::hgetdel(const std::string& field) {
        auto valOpt = internalMap.find(field);
        if (valOpt.has_value()) {
            auto str_ptr = valOpt.value()->ptr.load(std::memory_order_acquire);
            std::string valueCopy = *str_ptr; 
            
            internalMap.erase(field);
            return valueCopy;
        }
        return std::nullopt;
    }
}