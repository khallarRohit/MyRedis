#include "redislist.h"

namespace MyRedis{

    RedisList::RedisList() {
        head = std::make_shared<ListBlock>();
        tail = head;
    }

    DataType RedisList::getType() const{ 
        return DataType::LIST; 
    }

    size_t RedisList::RedisList::size() const {
        return list_size.load(std::memory_order_relaxed);
    }

    int RedisList::lpush(const std::vector<std::string>& elements) {
        for (const auto& el : elements) {
            while(true){
                bool pushed = false;

                // --- FAST PATH ---
                {
                    std::shared_lock<std::shared_mutex> listLock(listMutex);
                    
                    std::unique_lock<std::shared_mutex> blockLock(head->blockMutex);
                    
                    if (head->elements.size() < BLOCK_CAPACITY) {
                        head->elements.push_front(el);
                        pushed = true;
                    }
                }

                if (pushed) {
                    list_size.fetch_add(1, std::memory_order_relaxed);
                    break; 
                }

                // --- SLOW PATH (Block is full, allocate new head) ---
                {
                    std::unique_lock<std::shared_mutex> listLock(listMutex);

                    if (head->elements.size() == BLOCK_CAPACITY) {
                        auto newBlock = std::make_shared<ListBlock>();
                        newBlock->elements.push_front(el); 
                        
                        newBlock->next = head;
                        head->prev = newBlock;
                        head = newBlock;
                        
                        list_size.fetch_add(1, std::memory_order_relaxed);
                        break; 
                    }
                }
            }
        }
        return list_size.load(std::memory_order_relaxed);
    }
    
    int RedisList::rpush(const std::vector<std::string>& elements) {
        for (const auto& el : elements) {
            while(true){
                bool pushed = false;

                // --- FAST PATH ---
                {
                    std::shared_lock<std::shared_mutex> listLock(listMutex);
                    
                    std::unique_lock<std::shared_mutex> blockLock(tail->blockMutex);
                    
                    if (tail->elements.size() < BLOCK_CAPACITY) {
                        tail->elements.push_back(el);
                        pushed = true;
                    }
                }

                if (pushed) {
                    list_size.fetch_add(1, std::memory_order_relaxed);
                    break; 
                }

                // --- SLOW PATH (Block is full, allocate new head) ---
                {
                    std::unique_lock<std::shared_mutex> listLock(listMutex);

                    if (tail->elements.size() == BLOCK_CAPACITY) {
                        auto newBlock = std::make_shared<ListBlock>();
                        newBlock->elements.push_back(el); 
                        
                        tail->next = newBlock;
                        newBlock->prev = tail;
                        tail = newBlock;
                        
                        list_size.fetch_add(1, std::memory_order_relaxed);
                        break; 
                    }
                }
            }
        }
        return list_size.load(std::memory_order_relaxed);
    }

    std::optional<std::string> RedisList::lpop() {
        while(true){
            if (list_size.load(std::memory_order_relaxed) == 0) return std::nullopt;

            bool popped = false;
            bool block_became_empty = false;

            std::string popped_val;

            // --- FAST PATH ---
            {
                std::shared_lock<std::shared_mutex> listLock(listMutex);

                std::unique_lock<std::shared_mutex> blockLock(head->blockMutex);

                if (!head->elements.empty()) {
                    popped_val = std::move(head->elements.front());
                    head->elements.pop_front();
                    popped = true;

                    if (head->elements.empty() && head != tail) {
                        block_became_empty = true;
                    }
                }

            }

            if (popped) {
                list_size.fetch_sub(1, std::memory_order_relaxed);
                
                // --- SLOW PATH (Cleanup empty block) ---
                if (block_became_empty) {
                    std::unique_lock<std::shared_mutex> listLock(listMutex);

                    if (head->elements.empty() && head != tail) {
                        head = head->next;
                        head->prev.reset();
                    }
                }
                return popped_val;
            }
        }
    }

    std::optional<std::string> RedisList::rpop() {
        while(true){
            if (list_size.load(std::memory_order_relaxed) == 0) return std::nullopt;

            bool popped = false;
            bool block_became_empty = false;

            std::string popped_val;

            // --- FAST PATH ---
            {
                std::shared_lock<std::shared_mutex> listLock(listMutex);

                std::unique_lock<std::shared_mutex> blockLock(tail->blockMutex);

                if (!tail->elements.empty()) {
                    popped_val = std::move(tail->elements.back());
                    tail->elements.pop_back();
                    popped = true;

                    if (tail->elements.empty() && head != tail) {
                        block_became_empty = true;
                    }
                }

            }

            if (popped) {
                list_size.fetch_sub(1, std::memory_order_relaxed);
                
                // --- SLOW PATH (Cleanup empty block) ---
                if (block_became_empty) {
                    std::unique_lock<std::shared_mutex> listLock(listMutex);

                    if (tail->elements.empty() && head != tail) {
                        auto prevBlock = tail->prev.lock(); // Convert weak_ptr to shared_ptr
                        if (prevBlock) {
                            tail = prevBlock;
                            tail->next.reset();
                        }
                    }
                }
                return popped_val;
            }
        }
    }

    std::vector<std::string> RedisList::lrange(int start, int stop) const {
        int len = list_size.load(std::memory_order_relaxed);
        if (len == 0) return {};

        // Convert negative indices
        if (start < 0) start = len + start;
        if (stop < 0) stop = len + stop;

        // Clamp bounds
        if (start < 0) start = 0;
        if (start > stop || start >= len) return {};
        if (stop >= len) stop = len - 1;

        std::vector<std::string> result;
        result.reserve(stop - start + 1);

        int current_global_index = 0;

        std::shared_lock<std::shared_mutex> listLock(listMutex);

        std::shared_ptr<ListBlock> current_block = head;

        while (current_block != nullptr && current_global_index <= stop){
            std::shared_lock<std::shared_mutex> blockLock(current_block->blockMutex);

            int block_size = current_block->elements.size();

            if (current_global_index + block_size > start) {

                int local_start = std::max(0, start - current_global_index);
                int local_end = std::min(block_size - 1, stop - current_global_index);

                for (int i = local_start; i <= local_end; ++i) {
                    result.push_back(current_block->elements[i]);
                }
            }

            current_global_index += block_size;
            blockLock.unlock();

            current_block = current_block->next;
        }

        return result;
    }
}