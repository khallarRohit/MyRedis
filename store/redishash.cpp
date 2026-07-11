#include "redishash.h"

namespace MyRedis{
    DataType RedisHash::getType() const { 
        return DataType::HASH; 
    }

    int RedisHash::hset(const std::string& field, const std::string& value) {
        // Create the new atomic wrapper right away
        auto new_val = std::make_shared<AtomicValue>(value);
        
        // Single traversal! If it exists, the map returns the existing shared_ptr safely.
        auto [existing_opt, inserted] = internalMap.insert(field, new_val);
        
        if (!inserted && existing_opt.has_value()) {
            // --- FAST PATH OVERRIDE ---
            // The key already existed. The map did NOT overwrite the shared_ptr (which would be unsafe).
            // We use the returned shared_ptr to perform the lock-free RCU update natively.
            auto new_str_ptr = std::make_shared<const std::string>(value);
            existing_opt.value()->ptr.store(new_str_ptr, std::memory_order_release);
            return 0; // 0 indicates an existing field was updated
        }
        
        return 1; // 1 indicates a brand new field was inserted
    }

    std::optional<std::string> RedisHash::hget(const std::string& field) const {
        auto valOpt = internalMap.find(field);
        if (valOpt.has_value()) {
            // Safely read the string pointer out of the atomic wrapper
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
            // Extract the string before the node gets deleted
            auto str_ptr = valOpt.value()->ptr.load(std::memory_order_acquire);
            std::string valueCopy = *str_ptr; 
            
            internalMap.erase(field);
            return valueCopy;
        }
        return std::nullopt;
    }
}