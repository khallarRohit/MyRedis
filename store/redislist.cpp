#include "redislist.h"
#include <algorithm>

namespace MyRedis {

    RedisList::RedisList() {
        head = std::make_shared<ListBlock>();
        tail = head;
    }

    DataType RedisList::getType() const { 
        return DataType::LIST; 
    }

    size_t RedisList::size() const {
        return list_size.load(std::memory_order_relaxed);
    }

    int RedisList::lpush(const std::vector<std::string>& elements) {
        for (const auto& el : elements) {
            while (true) {
                bool need_new_block = false;
                
                {
                    std::unique_lock<std::shared_mutex> listLock(listMutex);
                    std::unique_lock<std::shared_mutex> blockLock(head->blockMutex);
                    
                    if (head->elements.size() < BLOCK_CAPACITY) {
                        head->elements.push_front(el);
                        break; 
                    }
                    need_new_block = true;
                }

                if (need_new_block) {
                    std::unique_lock<std::shared_mutex> listLock(listMutex);
                    if (head->elements.size() == BLOCK_CAPACITY) {
                        auto newBlock = std::make_shared<ListBlock>();
                        newBlock->elements.push_front(el);
                        
                        newBlock->next = head;
                        head->prev = newBlock;
                        head = newBlock;
                        break;
                    }
                }
            }
            list_size.fetch_add(1, std::memory_order_relaxed);
        }
        return list_size.load(std::memory_order_relaxed);
    }
    
    int RedisList::rpush(const std::vector<std::string>& elements) {
        for (const auto& el : elements) {
            while (true) {
                bool need_new_block = false;
                
                {
                    std::unique_lock<std::shared_mutex> listLock(listMutex);
                    std::unique_lock<std::shared_mutex> blockLock(tail->blockMutex);
                    
                    if (tail->elements.size() < BLOCK_CAPACITY) {
                        tail->elements.push_back(el);
                        break;
                    }
                    need_new_block = true;
                }

                if (need_new_block) {
                    std::unique_lock<std::shared_mutex> listLock(listMutex);
                    if (tail->elements.size() == BLOCK_CAPACITY) {
                        auto newBlock = std::make_shared<ListBlock>();
                        newBlock->elements.push_back(el);
                        
                        tail->next = newBlock;
                        newBlock->prev = tail;
                        tail = newBlock;
                        break;
                    }
                }
            }
            list_size.fetch_add(1, std::memory_order_relaxed);
        }
        return list_size.load(std::memory_order_relaxed);
    }

    std::optional<std::string> RedisList::lpop() {
        if (list_size.load(std::memory_order_relaxed) == 0) return std::nullopt;

        std::string popped_val;
        {
            std::unique_lock<std::shared_mutex> listLock(listMutex);
            
            if (list_size.load(std::memory_order_relaxed) == 0 || !head) {
                return std::nullopt;
            }

            std::unique_lock<std::shared_mutex> blockLock(head->blockMutex);
            
            if (head->elements.empty()) {
                return std::nullopt; 
            }

            popped_val = std::move(head->elements.front());
            head->elements.pop_front();

            // Deallocate block if empty, provided it isn't the last block left
            if (head->elements.empty() && head != tail) {
                head = head->next;
                if (head) {
                    head->prev.reset();
                }
            }
        }

        list_size.fetch_sub(1, std::memory_order_relaxed);
        return popped_val;
    }

    std::optional<std::string> RedisList::rpop() {
        if (list_size.load(std::memory_order_relaxed) == 0) return std::nullopt;

        std::string popped_val;
        {
            std::unique_lock<std::shared_mutex> listLock(listMutex);
            
            if (list_size.load(std::memory_order_relaxed) == 0 || !tail) {
                return std::nullopt;
            }

            std::unique_lock<std::shared_mutex> blockLock(tail->blockMutex);
            
            if (tail->elements.empty()) {
                return std::nullopt;
            }

            popped_val = std::move(tail->elements.back());
            tail->elements.pop_back();

            if (tail->elements.empty() && head != tail) {
                auto prevBlock = tail->prev.lock();
                if (prevBlock) {
                    tail = prevBlock;
                    tail->next.reset();
                }
            }
        }

        list_size.fetch_sub(1, std::memory_order_relaxed);
        return popped_val;
    }

    std::vector<std::string> RedisList::lrange(int start, int stop) const {
        int len = list_size.load(std::memory_order_relaxed);
        if (len == 0) return {};

        if (start < 0) start = len + start;
        if (stop < 0) stop = len + stop;

        if (start < 0) start = 0;
        if (start > stop || start >= len) return {};
        if (stop >= len) stop = len - 1;

        std::vector<std::string> result;
        result.reserve(stop - start + 1);

        std::shared_lock<std::shared_mutex> listLock(listMutex);

        std::shared_ptr<ListBlock> current_block = head;
        int current_global_index = 0;

        while (current_block != nullptr && current_global_index <= stop) {
            int block_size = static_cast<int>(current_block->elements.size());

            if (current_global_index + block_size > start) {
                int local_start = std::max(0, start - current_global_index);
                int local_end = std::min(block_size - 1, stop - current_global_index);

                for (int i = local_start; i <= local_end; ++i) {
                    result.push_back(current_block->elements[i]);
                }
            }

            current_global_index += block_size;
            current_block = current_block->next;
        }

        return result;
    }
}