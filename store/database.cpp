#include "database.h"

namespace MyRedis{

    RedisDatabase::RedisDatabase(){
        expiryThread = std::thread(&RedisDatabase::activeDeleteLoop, this);
    }

    RedisDatabase::~RedisDatabase() {
        // Graceful Shutdown
        stopExpiryThread = true;
        if (expiryThread.joinable()) {
            expiryThread.join();
        }
    }

    void RedisDatabase::activeDeleteLoop(){
        while (!stopExpiryThread) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            {
                std::unique_lock<std::shared_mutex> lock(dbMutex);
                
                // Note: A true Redis instance randomly samples 20 keys with an expiry
                // to prevent locking the DB for too long. For our scale, iterating 
                // the whole map (or a batch) is mathematically fine.
                
                for (auto it = keyspace.begin(); it != keyspace.end(); ) {
                    if (it->second->isExpired()) {
                        it = keyspace.erase(it); 
                    } else {
                        ++it;
                    }
                }
            }
        }    
    }

    void RedisDatabase::set(const std::string& key, const std::string& value){
        std::unique_lock<std::shared_mutex> lock(dbMutex);
        keyspace[key] = std::make_shared<RedisString>(value);
    }


    // Returns std::nullopt if the key doesn't exist, is expired, or is the wrong type
    std::shared_ptr<RedisString> RedisDatabase::get(const std::string& key) {
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

    // --- GETSET ---
    // Sets a new value and returns the old one. Requires unique_lock because it writes.
    std::optional<std::string> RedisDatabase::getset(const std::string& key, const std::string& value) {
        std::unique_lock<std::shared_mutex> lock(dbMutex);
        auto it = keyspace.find(key);
        std::optional<std::string> oldValue = std::nullopt;

        if (it != keyspace.end() && !it->second->isExpired()) {
            std::shared_ptr<RedisString> strObj = std::dynamic_pointer_cast<RedisString>(it->second);
            if (!strObj) throw std::invalid_argument("WRONGTYPE");
            oldValue = strObj->get(); // Save the old value
        }

        // Overwrite with the new value
        keyspace[key] = std::make_shared<RedisString>(value);
        return oldValue;
    }

    // --- SUBSTR ---
    // Requires shared_lock because it only reads.
    std::string RedisDatabase::substr(const std::string& key, int start, int stop) {
        std::shared_lock<std::shared_mutex> lock(dbMutex);
        auto it = keyspace.find(key);
        
        // Redis returns an empty string if the key doesn't exist
        if (it == keyspace.end() || it->second->isExpired()) {
            return ""; 
        }

        std::shared_ptr<RedisString> strObj = std::dynamic_pointer_cast<RedisString>(it->second);
        if (!strObj) throw std::invalid_argument("WRONGTYPE");

        return strObj->substr(start, stop);
    }

    // --- MSET ---
    // Requires unique_lock. Takes a vector of Key-Value pairs.
    void RedisDatabase::mset(const std::vector<std::pair<std::string, std::string>>& keyValues) {
        std::unique_lock<std::shared_mutex> lock(dbMutex);
        for (const auto& kv : keyValues) {
            keyspace[kv.first] = std::make_shared<RedisString>(kv.second);
        }
    }

    // --- MGET ---
    // Requires shared_lock. Returns a vector of optionals.
    std::vector<std::optional<std::string>> RedisDatabase::mget(const std::vector<std::string>& keys) {
        std::shared_lock<std::shared_mutex> lock(dbMutex);
        std::vector<std::optional<std::string>> results;
        
        for (const auto& key : keys) {
            auto it = keyspace.find(key);
            if (it == keyspace.end() || it->second->isExpired()) {
                results.push_back(std::nullopt);
            } else {
                std::shared_ptr<RedisString> strObj = std::dynamic_pointer_cast<RedisString>(it->second);
                if (!strObj) {
                    // Subtle Redis rule: MGET returns nil for WRONGTYPE, it doesn't crash the command
                    results.push_back(std::nullopt); 
                } else {
                    results.push_back(strObj->get());
                }
            }
        }
        return results;
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

    // --- HDEL ---
    size_t RedisDatabase::hdel(const std::string& key, const std::string& field) {
        std::unique_lock<std::shared_mutex> lock(dbMutex); // UNIQUE lock (Mutating)

        auto it = keyspace.find(key);
        if (it == keyspace.end() || it->second->isExpired()) {
            return 0; // Key doesn't exist, so 0 fields were deleted
        }

        std::shared_ptr<RedisHash> hashObj = std::dynamic_pointer_cast<RedisHash>(it->second);
        if (!hashObj) {
            throw std::invalid_argument("WRONGTYPE");
        }

        size_t removedCount = hashObj->hdel(field);

        // Redis Rule: If the hash is now empty, delete the top-level key!
        if (hashObj->hlen() == 0) {
            keyspace.erase(it);
        }

        return removedCount;
    }

    // Inside RedisDatabase class
    // --- HEXISTS ---
    std::optional<int> RedisDatabase::hexists(const std::string& key, const std::string& field) {
        std::shared_lock<std::shared_mutex> lock(dbMutex);
        auto it = keyspace.find(key);
        
        // If key doesn't exist, the field doesn't exist (return 0)
        if (it == keyspace.end() || it->second->isExpired()) return std::nullopt; 

        std::shared_ptr<RedisHash> hashObj = std::dynamic_pointer_cast<RedisHash>(it->second);
        if (!hashObj) throw std::invalid_argument("WRONGTYPE");

        return hashObj->hexists(field);
    }

    // --- HLEN ---
    std::optional<size_t> RedisDatabase::hlen(const std::string& key) {
        std::shared_lock<std::shared_mutex> lock(dbMutex);
        auto it = keyspace.find(key);
        
        // If key doesn't exist, length is 0
        if (it == keyspace.end() || it->second->isExpired()) return std::nullopt;

        std::shared_ptr<RedisHash> hashObj = std::dynamic_pointer_cast<RedisHash>(it->second);
        if (!hashObj) throw std::invalid_argument("WRONGTYPE");

        return hashObj->hlen();
    }

    // --- HGETDEL ---
    std::optional<std::string> RedisDatabase::hgetdel(const std::string& key, const std::string& field) {
        std::unique_lock<std::shared_mutex> lock(dbMutex); // UNIQUE lock (Mutating)
        auto it = keyspace.find(key);
        
        if (it == keyspace.end() || it->second->isExpired()) return std::nullopt;

        std::shared_ptr<RedisHash> hashObj = std::dynamic_pointer_cast<RedisHash>(it->second);
        if (!hashObj) throw std::invalid_argument("WRONGTYPE");

        auto result = hashObj->hgetdel(field);

        // Redis Rule: If the hash is now empty, delete the top-level key!
        if (hashObj->hlen() == 0) {
            keyspace.erase(it);
        }

        return result;
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

    // --- RPUSH ---
    // Pushes elements to the TAIL of the list. Returns the new length.
    int RedisDatabase::rpush(const std::string& key, const std::vector<std::string>& elements) {
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

        return listObj->rpush(elements);
    }

    // --- RPOP ---
    // Removes and returns the last element. Deletes the key if empty.
    std::optional<std::string> RedisDatabase::rpop(const std::string& key) {
        std::unique_lock<std::shared_mutex> lock(dbMutex);

        auto it = keyspace.find(key);
        if (it == keyspace.end() || it->second->isExpired()) {
            return std::nullopt; 
        }

        std::shared_ptr<RedisList> listObj = std::dynamic_pointer_cast<RedisList>(it->second);
        if (!listObj) throw std::invalid_argument("WRONGTYPE");

        auto result = listObj->rpop();

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

    // --- SCARD ---
    size_t RedisDatabase::scard(const std::string& key) {
        std::shared_lock<std::shared_mutex> lock(dbMutex);
        auto it = keyspace.find(key);
        
        // If the set doesn't exist, cardinality is 0
        if (it == keyspace.end() || it->second->isExpired()) return 0;

        std::shared_ptr<RedisSet> setObj = std::dynamic_pointer_cast<RedisSet>(it->second);
        if (!setObj) throw std::invalid_argument("WRONGTYPE");

        return setObj->scard();
    }

    // --- SREM ---
    int RedisDatabase::srem(const std::string& key, const std::vector<std::string>& members) {
        std::unique_lock<std::shared_mutex> lock(dbMutex); // UNIQUE lock (Mutating)
        auto it = keyspace.find(key);
        
        // If the set doesn't exist, 0 elements were removed
        if (it == keyspace.end() || it->second->isExpired()) return 0;

        std::shared_ptr<RedisSet> setObj = std::dynamic_pointer_cast<RedisSet>(it->second);
        if (!setObj) throw std::invalid_argument("WRONGTYPE");

        int removedCount = setObj->srem(members);

        // Redis Rule: If the set is now empty, delete the top-level key!
        if (setObj->scard() == 0) {
            keyspace.erase(it);
        }

        return removedCount;
    }

    int RedisDatabase::zadd(const std::string& key, double score, const std::string& member) {
        std::unique_lock<std::shared_mutex> lock(dbMutex);
        auto it = keyspace.find(key);
        std::shared_ptr<RedisZSet> zsetObj;

        if (it == keyspace.end() || it->second->isExpired()) {
            zsetObj = std::make_shared<RedisZSet>();
            keyspace[key] = zsetObj;
        } else {
            zsetObj = std::dynamic_pointer_cast<RedisZSet>(it->second);
            if (!zsetObj) throw std::invalid_argument("WRONGTYPE");
        }
        return zsetObj->zadd(score, member);
    }

    std::vector<std::string> RedisDatabase::zrange(const std::string& key, int start, int stop) {
        std::shared_lock<std::shared_mutex> lock(dbMutex);
        auto it = keyspace.find(key);
        if (it == keyspace.end() || it->second->isExpired()) return {};

        std::shared_ptr<RedisZSet> zsetObj = std::dynamic_pointer_cast<RedisZSet>(it->second);
        if (!zsetObj) throw std::invalid_argument("WRONGTYPE");
        return zsetObj->zrange(start, stop);
    }

    // --- ZCARD ---
    size_t RedisDatabase::zcard(const std::string& key) {
        std::shared_lock<std::shared_mutex> lock(dbMutex);
        auto it = keyspace.find(key);
        if (it == keyspace.end() || it->second->isExpired()) return 0;

        std::shared_ptr<RedisZSet> zsetObj = std::dynamic_pointer_cast<RedisZSet>(it->second);
        if (!zsetObj) throw std::invalid_argument("WRONGTYPE");
        return zsetObj->zcard();
    }

    // --- ZCOUNT ---
    size_t RedisDatabase::zcount(const std::string& key, double min, double max) {
        std::shared_lock<std::shared_mutex> lock(dbMutex);
        auto it = keyspace.find(key);
        if (it == keyspace.end() || it->second->isExpired()) return 0;

        std::shared_ptr<RedisZSet> zsetObj = std::dynamic_pointer_cast<RedisZSet>(it->second);
        if (!zsetObj) throw std::invalid_argument("WRONGTYPE");
        return zsetObj->zcount(min, max);
    }

    // --- ZRANK ---
    std::optional<int> RedisDatabase::zrank(const std::string& key, const std::string& member) {
        std::shared_lock<std::shared_mutex> lock(dbMutex);
        auto it = keyspace.find(key);
        if (it == keyspace.end() || it->second->isExpired()) return std::nullopt;

        std::shared_ptr<RedisZSet> zsetObj = std::dynamic_pointer_cast<RedisZSet>(it->second);
        if (!zsetObj) throw std::invalid_argument("WRONGTYPE");
        return zsetObj->zrank(member);
    }

    // --- ZSCORE ---
    std::optional<double> RedisDatabase::zscore(const std::string& key, const std::string& member) {
        std::shared_lock<std::shared_mutex> lock(dbMutex);
        auto it = keyspace.find(key);
        if (it == keyspace.end() || it->second->isExpired()) return std::nullopt;

        std::shared_ptr<RedisZSet> zsetObj = std::dynamic_pointer_cast<RedisZSet>(it->second);
        if (!zsetObj) throw std::invalid_argument("WRONGTYPE");
        return zsetObj->zscore(member);
    }

}