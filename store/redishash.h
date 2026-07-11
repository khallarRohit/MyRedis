#pragma once
#include "object.h"
#include "hashmap.h" // custom hash map
#include <string>
#include <optional>
#include <shared_mutex>
#include <atomic>
#include <memory>

namespace MyRedis{

    class RedisHash : public RedisObject{
    private:
        struct AtomicValue {
            std::atomic<std::shared_ptr<const std::string>> ptr;
            
            explicit AtomicValue(const std::string& val) {
                ptr.store(std::make_shared<const std::string>(val), std::memory_order_relaxed);
            }
        };

        HashMap<std::string, std::shared_ptr<AtomicValue>> internalMap;

        mutable std::shared_mutex hashMutex;
    public:
        RedisHash() = default;
        ~RedisHash() override = default;

        DataType getType() const override;

        int hset(const std::string& field, const std::string& value);
        std::optional<std::string> hget(const std::string& field) const;
        size_t hdel(const std::string& field);
        int hexists(const std::string& field) const;
        size_t hlen() const;
        std::optional<std::string> hgetdel(const std::string& field);
    };


}