#include "redislist.h"

namespace MyRedis{

    DataType RedisList::getType() const{ 
        return DataType::LIST; 
    }

    size_t RedisList::RedisList::size() const {
        return internalList.size();
    }

    int RedisList::lpush(const std::vector<std::string>& elements) {
        for (const auto& el : elements) {
            internalList.push_front(el);
        }
        return internalList.size();
    }
    
    int RedisList::rpush(const std::vector<std::string>& elements) {
        for (const auto& el : elements) {
            internalList.push_back(el);
        }
        return internalList.size();
    }

    std::optional<std::string> RedisList::lpop() {
        if (internalList.empty()) return std::nullopt;
        
        std::string val = internalList.front();
        internalList.pop_front();
        return val;
    }

    std::optional<std::string> RedisList::rpop() {
        if (internalList.empty()) return std::nullopt;
        
        std::string val = internalList.back();
        internalList.pop_back();
        return val;
    }

    std::vector<std::string> RedisList::lrange(int start, int stop) const {
        int len = internalList.size();
        if (len == 0) return {};

        // Convert negative indices
        if (start < 0) start = len + start;
        if (stop < 0) stop = len + stop;

        // Clamp bounds
        if (start < 0) start = 0;
        if (start > stop || start >= len) return {};
        if (stop >= len) stop = len - 1;

        std::vector<std::string> result;
        for (int i = start; i <= stop; i++) {
            result.push_back(internalList[i]);
        }
        return result;
    }
}