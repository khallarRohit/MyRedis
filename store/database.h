#pragma once
#include "object.h"
#include "redisstring.h"
#include "redishash.h"
#include "redislist.h"
#include "redisset.h"
#include "rediszset.h"
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
        void activeDeleteLoop();

    public:
        RedisDatabase();
        ~RedisDatabase();

        // String
        void set(const std::string& key, const std::string& value);
        std::shared_ptr<RedisString> get(const std::string& key);
        std::optional<std::string> getset(const std::string& key, const std::string& value);
        std::string substr(const std::string& key, int start, int stop);
        void mset(const std::vector<std::pair<std::string, std::string>>& keyValues);
        std::vector<std::optional<std::string>> mget(const std::vector<std::string>& keys);


        // hash
        void hset(const std::string& key, const std::string& field, const std::string& value);
        std::optional<std::string> hget(const std::string& key, const std::string& field);
        size_t hdel(const std::string& key, const std::string& field);
        std::optional<int> hexists(const std::string& key, const std::string& field);
        std::optional<size_t> hlen(const std::string& key);
        std::optional<std::string> hgetdel(const std::string& key, const std::string& field);
        

        // list
        int lpush(const std::string& key, const std::vector<std::string>& elements);
        std::optional<std::string> lpop(const std::string& key);
        int rpush(const std::string& key, const std::vector<std::string>& elements);
        std::optional<std::string> rpop(const std::string& key);
        std::vector<std::string> lrange(const std::string& key, int start, int stop);


        // set
        int sadd(const std::string& key, const std::vector<std::string>& members);
        std::optional<int> sismember(const std::string& key, const std::string& member);
        size_t scard(const std::string& key);
        int srem(const std::string& key, const std::vector<std::string>& members);


        // zset
        int zadd(const std::string& key, double score, const std::string& member);
        std::vector<std::string> zrange(const std::string& key, int start, int stop);
        size_t RedisDatabase::zcard(const std::string& key);
        size_t RedisDatabase::zcount(const std::string& key, double min, double max);
        std::optional<int> RedisDatabase::zrank(const std::string& key, const std::string& member);
        std::optional<double> RedisDatabase::zscore(const std::string& key, const std::string& member);

    };
}