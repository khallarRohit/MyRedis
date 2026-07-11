#pragma once
#include "object.h"
#include <string>
#include <vector>
#include <deque>
#include <shared_mutex>
#include <mutex>
#include <memory>
#include <atomic>
#include <optional>

namespace MyRedis{

    class RedisList : public RedisObject{
    private:
        static constexpr size_t BLOCK_CAPACITY = 512;

        struct ListBlock {
            std::deque<std::string> elements;
            mutable std::shared_mutex blockMutex;
            
            std::shared_ptr<ListBlock> next{nullptr};
            std::weak_ptr<ListBlock> prev; 
            
            ListBlock() = default;
        };

        std::shared_ptr<ListBlock> head;
        std::shared_ptr<ListBlock> tail;
        
        // Protects structural changes (adding/removing blocks)
        mutable std::shared_mutex listMutex; 
        
        // Atomic counter for O(1) LLEN commands
        std::atomic<size_t> list_size{0};

    public:
        RedisList() = default;
        ~RedisList() override = default;

        DataType getType() const override;
        size_t size() const;

        int lpush(const std::vector<std::string>& elements);
        int rpush(const std::vector<std::string>& elements);
        std::optional<std::string> lpop();
        std::optional<std::string> rpop();

        std::vector<std::string> lrange(int start, int stop) const;
    };

}