#include "database.h"

namespace MyRedis{

    void RedisDatabase::setString(const std::string& key, const std::string& value){
        std::unique_lock<std::shared_mutex> lock(dbMutex);
        keyspace[key] = std::make_shared<RedisString>(value);
    }

    // Returns std::nullopt if the key doesn't exist, is expired, or is the wrong type
    std::shared_ptr<RedisString> RedisDatabase::getString(const std::string& key) {
        std::shared_lock<std::shared_mutex> lock(dbMutex); // SHARED Read Lock

        auto it = keyspace.find(key);
        if (it == keyspace.end()) {
            return nullptr;
        }

        // Lazy Deletion Check
        if (it->second->isExpired()) {
            return nullptr;
        }

        std::shared_ptr<RedisString> strObj = std::dynamic_pointer_cast<RedisString>(it->second);

        return strObj;
    }

    void RedisDatabase::hset(const std::string& key, const std::string& field, const std::string& value) {
        std::unique_lock<std::shared_mutex> lock(dbMutex);
        
        auto it = keyspace.find(key);
        std::shared_ptr<RedisHash> hashObj;

        if (it == keyspace.end() || it->second->isExpired()) {
            // Key doesn't exist, create a new RedisHash!
            hashObj = std::make_shared<RedisHash>();
            keyspace[key] = hashObj;
        } else {
            // Key exists, safely cast it to a RedisHash
            hashObj = std::dynamic_pointer_cast<RedisHash>(it->second);
            if (!hashObj) {
                throw std::invalid_argument("WRONGTYPE");
            }
        }

        // Insert the data into your custom HashMap
        hashObj->hset(field, value);
    }

    std::optional<std::string> RedisDatabase::hget(const std::string& key, const std::string& field) {
        std::shared_lock<std::shared_mutex> lock(dbMutex);

        auto it = keyspace.find(key);
        if (it == keyspace.end() || it->second->isExpired()) {
            return std::nullopt; // Key doesn't exist
        }

        std::shared_ptr<RedisHash> hashObj = std::dynamic_pointer_cast<RedisHash>(it->second);
        if (!hashObj) {
            throw std::invalid_argument("WRONGTYPE");
        }

        // Query your custom HashMap
        return hashObj->hget(field);
    }

    int RedisDatabase::lpush(const std::string& key, const std::vector<std::string>& elements) {
        std::unique_lock<std::shared_mutex> lock(dbMutex);
        
        auto it = keyspace.find(key);
        std::shared_ptr<RedisList> listObj;

        if (it == keyspace.end() || it->second->isExpired()) {
            listObj = std::make_shared<RedisList>();
            keyspace[key] = listObj;
        } else {
            listObj = std::dynamic_pointer_cast<RedisList>(it->second);
            if (!listObj) throw std::invalid_argument("WRONGTYPE");
        }

        return listObj->lpush(elements);
    }

    // --- LPOP ---
    // Notice this requires a UNIQUE lock because it mutates the list.
    std::optional<std::string> RedisDatabase::lpop(const std::string& key) {
        std::unique_lock<std::shared_mutex> lock(dbMutex);

        auto it = keyspace.find(key);
        if (it == keyspace.end() || it->second->isExpired()) {
            return std::nullopt; 
        }

        std::shared_ptr<RedisList> listObj = std::dynamic_pointer_cast<RedisList>(it->second);
        if (!listObj) throw std::invalid_argument("WRONGTYPE");

        auto result = listObj->lpop();

        // Redis Rule: Delete the key if the list is now empty!
        if (listObj->size() == 0) {
            keyspace.erase(it);
        }

        return result;
    }

    // --- LRANGE ---
    // Requires only a SHARED lock because we are just reading.
    std::vector<std::string> RedisDatabase::lrange(const std::string& key, int start, int stop) {
        std::shared_lock<std::shared_mutex> lock(dbMutex);

        auto it = keyspace.find(key);
        if (it == keyspace.end() || it->second->isExpired()) {
            return {}; // Return empty array
        }

        std::shared_ptr<RedisList> listObj = std::dynamic_pointer_cast<RedisList>(it->second);
        if (!listObj) throw std::invalid_argument("WRONGTYPE");

        return listObj->lrange(start, stop);
    }

    int RedisDatabase::sadd(const std::string& key, const std::vector<std::string>& members) {
        std::unique_lock<std::shared_mutex> lock(dbMutex);
        
        auto it = keyspace.find(key);
        std::shared_ptr<RedisSet> setObj;

        if (it == keyspace.end() || it->second->isExpired()) {
            setObj = std::make_shared<RedisSet>();
            keyspace[key] = setObj;
        } else {
            setObj = std::dynamic_pointer_cast<RedisSet>(it->second);
            if (!setObj) {
                throw std::invalid_argument("WRONGTYPE");
            }
        }

        return setObj->sadd(members);
    }

    // --- SISMEMBER ---
    std::optional<int> RedisDatabase::sismember(const std::string& key, const std::string& member) {
        std::shared_lock<std::shared_mutex> lock(dbMutex);

        auto it = keyspace.find(key);
        if (it == keyspace.end() || it->second->isExpired()) {
            return std::nullopt; // Key doesn't exist (treat as 0)
        }

        std::shared_ptr<RedisSet> setObj = std::dynamic_pointer_cast<RedisSet>(it->second);
        if (!setObj) {
            throw std::invalid_argument("WRONGTYPE");
        }

        return setObj->sismember(member);
    }

}