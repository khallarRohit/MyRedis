#include "database.h"
#include <chrono>

namespace MyRedis {

    RedisDatabase::RedisDatabase() {
        expiryThread = std::thread(&RedisDatabase::activeDeleteLoop, this);
    }

    RedisDatabase::~RedisDatabase() {
        stopExpiryThread = true;
        if (expiryThread.joinable()) {
            expiryThread.join();
        }
    }

    void RedisDatabase::activeDeleteLoop() {
        while (!stopExpiryThread) {
            std::this_thread::sleep_for(std::chrono::milliseconds(100));

            if (keyspace.size() == 0) continue;

            do {
                auto isExpiredPredicate = [](const std::string& k, const std::shared_ptr<RedisObject>& obj) {
                    return obj->isExpired();
                };

                auto result = keyspace.sample_and_erase_if(20, isExpiredPredicate);

                if (result.sampled == 0) break;
                
                double expiredRatio = static_cast<double>(result.expired) / result.sampled;
                
                if (expiredRatio <= 0.25) {
                    break; 
                }
                
            } while (!stopExpiryThread);
        }    
    }

    void RedisDatabase::set(const std::string& key, const std::string& value) {
        // Safe, single-pass concurrent overwrite
        keyspace.insert_or_assign(key, std::make_shared<RedisString>(value));
    }

    std::shared_ptr<RedisString> RedisDatabase::get(const std::string& key) {
        auto objOpt = keyspace.find(key);
        
        if (!objOpt.has_value()) {
            return nullptr;
        }

        auto obj = objOpt.value();

        if (obj->isExpired()) {
            keyspace.erase(key); 
            return nullptr;
        }

        return std::dynamic_pointer_cast<RedisString>(obj);
    }

// --- GETSET ---
    std::optional<std::string> RedisDatabase::getset(const std::string& key, const std::string& value) {
        auto objOpt = keyspace.find(key);
        
        if (objOpt.has_value()) {
            auto obj = objOpt.value();
            if (obj->isExpired()) {
                keyspace.erase(key);
            } else {
                auto strObj = std::dynamic_pointer_cast<RedisString>(obj);
                if (!strObj) throw std::invalid_argument("WRONGTYPE");
                
                std::string oldVal = strObj->get(); 
                
                strObj->set(value);
                return oldVal;
            }
        }

        keyspace.insert_or_assign(key, std::make_shared<RedisString>(value));
        return std::nullopt;
    }

    // --- SUBSTR ---
    std::string RedisDatabase::substr(const std::string& key, int start, int stop) {
        auto strObj = get(key); 
        
        if (!strObj) {
            return ""; 
        }

        return strObj->substr(start, stop);
    }

    // --- MSET ---
    void RedisDatabase::mset(const std::vector<std::pair<std::string, std::string>>& keyValues) {
        for (const auto& kv : keyValues) {
            keyspace.insert_or_assign(kv.first, std::make_shared<RedisString>(kv.second));
        }
    }

    // --- MGET ---
    std::vector<std::optional<std::string>> RedisDatabase::mget(const std::vector<std::string>& keys) {
        std::vector<std::optional<std::string>> results;
        results.reserve(keys.size());
        
        for (const auto& key : keys) {
            auto strObj = get(key); 
            
            if (!strObj) {
                results.push_back(std::nullopt); 
            } else {
                results.push_back(strObj->get());
            }
        }
        
        return results;
    }

// --- HSET ---
    void RedisDatabase::hset(const std::string& key, const std::string& field, const std::string& value) {
        auto objOpt = keyspace.find(key);
        std::shared_ptr<RedisHash> hashObj;

        if (objOpt.has_value()) {
            hashObj = std::dynamic_pointer_cast<RedisHash>(objOpt.value());
            if (!hashObj) throw std::invalid_argument("WRONGTYPE");
            
            if (hashObj->isExpired()) {
                keyspace.erase(key);
                hashObj = nullptr;
            }
        }

        if (!hashObj) {
            hashObj = std::make_shared<RedisHash>();
            
            auto [existing, inserted] = keyspace.insert(key, hashObj);
            
            if (!inserted && existing.has_value()) {
                hashObj = std::dynamic_pointer_cast<RedisHash>(existing.value());
                if (!hashObj) throw std::invalid_argument("WRONGTYPE");
            }
        }

        hashObj->hset(field, value);
    }

    // --- HGET ---
    std::optional<std::string> RedisDatabase::hget(const std::string& key, const std::string& field) {
        auto objOpt = keyspace.find(key);
        
        if (!objOpt.has_value()) {
            return std::nullopt; 
        }

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return std::nullopt;
        }

        auto hashObj = std::dynamic_pointer_cast<RedisHash>(obj);
        if (!hashObj) throw std::invalid_argument("WRONGTYPE");

        return hashObj->hget(field);
    }

    // --- HDEL ---
    size_t RedisDatabase::hdel(const std::string& key, const std::string& field) {
        auto objOpt = keyspace.find(key);
        
        if (!objOpt.has_value()) {
            return 0; 
        }

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return 0;
        }

        auto hashObj = std::dynamic_pointer_cast<RedisHash>(obj);
        if (!hashObj) throw std::invalid_argument("WRONGTYPE");

        size_t removedCount = hashObj->hdel(field);

        // Redis Rule: If the hash is now empty, delete the top-level key!
        // We use erase(key) safely because the ConcurrentHashMap handles the lock.
        if (hashObj->hlen() == 0) {
            keyspace.erase(key);
        }

        return removedCount;
    }

    // --- HEXISTS ---
    std::optional<int> RedisDatabase::hexists(const std::string& key, const std::string& field) {
        auto objOpt = keyspace.find(key);
        
        if (!objOpt.has_value()) {
            return std::nullopt; 
        }

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return std::nullopt;
        }

        auto hashObj = std::dynamic_pointer_cast<RedisHash>(obj);
        if (!hashObj) throw std::invalid_argument("WRONGTYPE");

        return hashObj->hexists(field);
    }

// --- HLEN ---
    std::optional<size_t> RedisDatabase::hlen(const std::string& key) {
        auto objOpt = keyspace.find(key);
        
        if (!objOpt.has_value()) {
            return std::nullopt;
        }

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return std::nullopt;
        }

        auto hashObj = std::dynamic_pointer_cast<RedisHash>(obj);
        if (!hashObj) throw std::invalid_argument("WRONGTYPE");

        return hashObj->hlen();
    }

    // --- HGETDEL ---
    std::optional<std::string> RedisDatabase::hgetdel(const std::string& key, const std::string& field) {
        auto objOpt = keyspace.find(key);
        
        if (!objOpt.has_value()) {
            return std::nullopt;
        }

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return std::nullopt;
        }

        auto hashObj = std::dynamic_pointer_cast<RedisHash>(obj);
        if (!hashObj) throw std::invalid_argument("WRONGTYPE");

        auto result = hashObj->hgetdel(field);

        // Redis Rule: If the hash is now empty, delete the top-level key!
        if (hashObj->hlen() == 0) {
            keyspace.erase(key);
        }

        return result;
    }

    // --- LPUSH ---
    int RedisDatabase::lpush(const std::string& key, const std::vector<std::string>& elements) {
        auto objOpt = keyspace.find(key);
        std::shared_ptr<RedisList> listObj;

        if (objOpt.has_value()) {
            listObj = std::dynamic_pointer_cast<RedisList>(objOpt.value());
            if (!listObj) throw std::invalid_argument("WRONGTYPE");
            
            if (listObj->isExpired()) {
                keyspace.erase(key);
                listObj = nullptr;
            }
        }

        if (!listObj) {
            listObj = std::make_shared<RedisList>();
            
            auto [existing, inserted] = keyspace.insert(key, listObj);
            
            if (!inserted && existing.has_value()) {
                listObj = std::dynamic_pointer_cast<RedisList>(existing.value());
                if (!listObj) throw std::invalid_argument("WRONGTYPE");
            }
        }

        return listObj->lpush(elements);
    }

    // --- LPOP ---
    std::optional<std::string> RedisDatabase::lpop(const std::string& key) {
        auto objOpt = keyspace.find(key);
        
        if (!objOpt.has_value()) {
            return std::nullopt; 
        }

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return std::nullopt;
        }

        auto listObj = std::dynamic_pointer_cast<RedisList>(obj);
        if (!listObj) throw std::invalid_argument("WRONGTYPE");

        auto result = listObj->lpop();

        // Redis Rule: Delete the key if the list is now empty!
        if (listObj->size() == 0) {
            keyspace.erase(key);
        }

        return result;
    }

// --- RPUSH ---
    // Pushes elements to the TAIL of the list. Returns the new length.
    int RedisDatabase::rpush(const std::string& key, const std::vector<std::string>& elements) {
        auto objOpt = keyspace.find(key);
        std::shared_ptr<RedisList> listObj;

        if (objOpt.has_value()) {
            listObj = std::dynamic_pointer_cast<RedisList>(objOpt.value());
            if (!listObj) throw std::invalid_argument("WRONGTYPE");
            
            // Lazy Deletion
            if (listObj->isExpired()) {
                keyspace.erase(key);
                listObj = nullptr;
            }
        }

        if (!listObj) {
            listObj = std::make_shared<RedisList>();
            
            // Thread-safe insert check
            auto [existing, inserted] = keyspace.insert(key, listObj);
            
            if (!inserted && existing.has_value()) {
                listObj = std::dynamic_pointer_cast<RedisList>(existing.value());
                if (!listObj) throw std::invalid_argument("WRONGTYPE");
            }
        }

        return listObj->rpush(elements);
    }

    // --- RPOP ---
    // Removes and returns the last element. Deletes the key if empty.
    std::optional<std::string> RedisDatabase::rpop(const std::string& key) {
        auto objOpt = keyspace.find(key);
        
        if (!objOpt.has_value()) {
            return std::nullopt; 
        }

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return std::nullopt;
        }

        auto listObj = std::dynamic_pointer_cast<RedisList>(obj);
        if (!listObj) throw std::invalid_argument("WRONGTYPE");

        auto result = listObj->rpop();

        // Redis Rule: Delete the key if the list is now empty!
        if (listObj->size() == 0) {
            keyspace.erase(key);
        }

        return result;
    }

    // --- LRANGE ---
    // Fully lock-free at the database level!
    std::vector<std::string> RedisDatabase::lrange(const std::string& key, int start, int stop) {
        auto objOpt = keyspace.find(key);
        
        if (!objOpt.has_value()) {
            return {};
        }

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return {};
        }

        auto listObj = std::dynamic_pointer_cast<RedisList>(obj);
        if (!listObj) throw std::invalid_argument("WRONGTYPE");

        return listObj->lrange(start, stop);
    }

    // --- SADD ---
    int RedisDatabase::sadd(const std::string& key, const std::vector<std::string>& members) {
        auto objOpt = keyspace.find(key);
        std::shared_ptr<RedisSet> setObj;

        if (objOpt.has_value()) {
            setObj = std::dynamic_pointer_cast<RedisSet>(objOpt.value());
            if (!setObj) throw std::invalid_argument("WRONGTYPE");
            
            if (setObj->isExpired()) {
                keyspace.erase(key);
                setObj = nullptr;
            }
        }

        if (!setObj) {
            setObj = std::make_shared<RedisSet>();
            
            auto [existing, inserted] = keyspace.insert(key, setObj);
            
            if (!inserted && existing.has_value()) {
                setObj = std::dynamic_pointer_cast<RedisSet>(existing.value());
                if (!setObj) throw std::invalid_argument("WRONGTYPE");
            }
        }

        return setObj->sadd(members);
    }

    // --- SISMEMBER ---
    std::optional<int> RedisDatabase::sismember(const std::string& key, const std::string& member) {
        auto objOpt = keyspace.find(key);
        
        if (!objOpt.has_value()) {
            return std::nullopt; 
        }

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return std::nullopt;
        }

        auto setObj = std::dynamic_pointer_cast<RedisSet>(obj);
        if (!setObj) throw std::invalid_argument("WRONGTYPE");

        return setObj->sismember(member);
    }

// --- SCARD ---
    size_t RedisDatabase::scard(const std::string& key) {
        auto objOpt = keyspace.find(key);
        
        if (!objOpt.has_value()) return 0;

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return 0;
        }

        auto setObj = std::dynamic_pointer_cast<RedisSet>(obj);
        if (!setObj) throw std::invalid_argument("WRONGTYPE");

        return setObj->scard();
    }

    // --- SREM ---
    int RedisDatabase::srem(const std::string& key, const std::vector<std::string>& members) {
        auto objOpt = keyspace.find(key);
        
        if (!objOpt.has_value()) return 0;

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return 0;
        }

        auto setObj = std::dynamic_pointer_cast<RedisSet>(obj);
        if (!setObj) throw std::invalid_argument("WRONGTYPE");

        int removedCount = setObj->srem(members);

        // Redis Rule: If the set is now empty, delete the top-level key!
        if (setObj->scard() == 0) {
            keyspace.erase(key);
        }

        return removedCount;
    }

    // --- ZADD ---
    int RedisDatabase::zadd(const std::string& key, double score, const std::string& member) {
        auto objOpt = keyspace.find(key);
        std::shared_ptr<RedisZSet> zsetObj;

        if (objOpt.has_value()) {
            zsetObj = std::dynamic_pointer_cast<RedisZSet>(objOpt.value());
            if (!zsetObj) throw std::invalid_argument("WRONGTYPE");
            
            if (zsetObj->isExpired()) {
                keyspace.erase(key);
                zsetObj = nullptr;
            }
        }

        if (!zsetObj) {
            zsetObj = std::make_shared<RedisZSet>();
            
            // Thread-safe creation
            auto [existing, inserted] = keyspace.insert(key, zsetObj);
            
            if (!inserted && existing.has_value()) {
                zsetObj = std::dynamic_pointer_cast<RedisZSet>(existing.value());
                if (!zsetObj) throw std::invalid_argument("WRONGTYPE");
            }
        }

        return zsetObj->zadd(score, member);
    }

    // --- ZRANGE ---
    std::vector<std::string> RedisDatabase::zrange(const std::string& key, int start, int stop) {
        auto objOpt = keyspace.find(key);
        if (!objOpt.has_value()) return {};

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return {};
        }

        auto zsetObj = std::dynamic_pointer_cast<RedisZSet>(obj);
        if (!zsetObj) throw std::invalid_argument("WRONGTYPE");
        
        return zsetObj->zrange(start, stop);
    }

    // --- ZCARD ---
    size_t RedisDatabase::zcard(const std::string& key) {
        auto objOpt = keyspace.find(key);
        if (!objOpt.has_value()) return 0;

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return 0;
        }

        auto zsetObj = std::dynamic_pointer_cast<RedisZSet>(obj);
        if (!zsetObj) throw std::invalid_argument("WRONGTYPE");
        
        return zsetObj->zcard();
    }

    // --- ZCOUNT ---
    size_t RedisDatabase::zcount(const std::string& key, double min, double max) {
        auto objOpt = keyspace.find(key);
        if (!objOpt.has_value()) return 0;

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return 0;
        }

        auto zsetObj = std::dynamic_pointer_cast<RedisZSet>(obj);
        if (!zsetObj) throw std::invalid_argument("WRONGTYPE");
        
        return zsetObj->zcount(min, max);
    }

    // --- ZRANK ---
    std::optional<int> RedisDatabase::zrank(const std::string& key, const std::string& member) {
        auto objOpt = keyspace.find(key);
        if (!objOpt.has_value()) return std::nullopt;

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return std::nullopt;
        }

        auto zsetObj = std::dynamic_pointer_cast<RedisZSet>(obj);
        if (!zsetObj) throw std::invalid_argument("WRONGTYPE");
        
        return zsetObj->zrank(member);
    }

    // --- ZSCORE ---
    std::optional<double> RedisDatabase::zscore(const std::string& key, const std::string& member) {
        auto objOpt = keyspace.find(key);
        if (!objOpt.has_value()) return std::nullopt;

        auto obj = objOpt.value();
        if (obj->isExpired()) {
            keyspace.erase(key);
            return std::nullopt;
        }

        auto zsetObj = std::dynamic_pointer_cast<RedisZSet>(obj);
        if (!zsetObj) throw std::invalid_argument("WRONGTYPE");
        
        return zsetObj->zscore(member);
    }

}