#pragma once
#include "object.h"
#include "redisstring.h"
#include "redishash.h"
#include "redislist.h"
#include "redisset.h"
#include <unordered_map> 
#include <shared_mutex>
#include <optional>
#include <mutex>
#include <memory>
#include <thread>
#include <atomic>

namespace MyRedis{

    class RedisDatabase{
    private:
        std::unordered_map<std::string, std::shared_ptr<RedisObject>> keyspace;
        mutable std::shared_mutex dbMutex;

        std::thread expiryThread;
        std::atomic<bool> stopExpiryThread{false};

        void activeDeleteLoop(){
            
        }
    public:
        RedisDatabase() = default;

        // String
        void setString(const std::string& key, const std::string& value);
        std::shared_ptr<RedisString> getString(const std::string& key);

        // hash
        void hset(const std::string& key, const std::string& field, const std::string& value);
        std::optional<std::string> hget(const std::string& key, const std::string& field);
        
        // list
        int lpush(const std::string& key, const std::vector<std::string>& elements);

        // --- LPOP ---
        // Notice this requires a UNIQUE lock because it mutates the list.
        std::optional<std::string> lpop(const std::string& key);

        // --- LRANGE ---
        // Requires only a SHARED lock because we are just reading.
        std::vector<std::string> lrange(const std::string& key, int start, int stop);

        // set
        int sadd(const std::string& key, const std::vector<std::string>& members);

        // --- SISMEMBER ---
        std::optional<int> sismember(const std::string& key, const std::string& member);
    };
}