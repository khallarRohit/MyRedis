#include "dispatcher.h"

namespace MyRedis{


    Dispatcher& Dispatcher::getInstance(){
        static Dispatcher instance;
        return instance;
    }

    void Dispatcher::registerCommand(std::string name, CommandHandler handler){
        std::transform(name.begin(), name.end(), name.begin(), ::toupper);
        registry[name] = handler;
    }

    void Dispatcher::dispatch(std::shared_ptr<ProcessJob> job){
        if(job->packetQuery.empty()){
            return;
        }

        std::string commandName = job->packetQuery[0];
        std::transform(commandName.begin(), commandName.end(), commandName.begin(), ::toupper);

        auto it = registry.find(commandName);
        if(it != registry.end()){
            it->second(job);
        }else{
            // need an outpacket constructor
            job->packetResponseManager->queueResponse("-ERR unknown command '" + commandName + "'\r\n");
        }
    }

    void Dispatcher::registerPING(){
        registerCommand("PING", [](std::shared_ptr<ProcessJob> job){
            if(job->packetQuery.size() == 1){
                job->packetResponseManager->queueResponse("+PONG\r\n");
            }else{
                std::string response = "$" + std::to_string(job->packetQuery[1].length()) + "\r\n" + job->packetQuery[1] + "\r\n";
                job->packetResponseManager->queueResponse(response);
            }
        });
    }

    void Dispatcher::registerECHO(){
        registerCommand("ECHO", [](std::shared_ptr<ProcessJob> job){
            if(job->packetQuery.size() >= 2){
                std::string response = "$" + std::to_string(job->packetQuery[1].length()) + "\r\n" + job->packetQuery[1] + "\r\n";
                job->packetResponseManager->queueResponse(response);
            }else{
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'echo' command\r\n");
            }
        });           
    }

    void Dispatcher::registerStringCommands(std::shared_ptr<RedisDatabase> db){
        registerCommand("SET", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                db->set(args[1], args[2]);
                job->packetResponseManager->queueResponse("+OK\r\n");
            } else {
                job->packetResponseManager->queueResponse("-ERR syntax error or wrong number of arguments\r\n");
            }
        });

        registerCommand("GET", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 2) {
                auto strObj = db->get(args[1]);
                if (strObj == nullptr) {
                    job->packetResponseManager->queueResponse("$-1\r\n");
                } else {
                    std::string val = strObj->get();
                    job->packetResponseManager->queueResponse("$" + std::to_string(val.length()) + "\r\n" + val + "\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'get' command\r\n");
            }
        });

        registerCommand("GETSET", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    auto oldVal = db->getset(args[1], args[2]);
                    if (oldVal.has_value()) {
                        job->packetResponseManager->queueResponse("$" + std::to_string(oldVal.value().length()) + "\r\n" + oldVal.value() + "\r\n");
                    } else {
                        job->packetResponseManager->queueResponse("$-1\r\n");
                    }
                } catch (...) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments\r\n");
            }
        });

        registerCommand("SUBSTR", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 4) {
                try {
                    int start = std::stoi(args[2]);
                    int stop = std::stoi(args[3]);
                    
                    std::string result = db->substr(args[1], start, stop);
                    job->packetResponseManager->queueResponse("$" + std::to_string(result.length()) + "\r\n" + result + "\r\n");
                    
                } catch (const std::invalid_argument& e) {
                    if (std::string(e.what()) == "WRONGTYPE") {
                        job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                    } else {
                        job->packetResponseManager->queueResponse("-ERR value is not an integer or out of range\r\n");
                    }
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'substr' command\r\n");
            }
        });

        registerCommand("MSET", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            // Total arguments must be odd: 1 for "MSET" + pairs of 2
            if (args.size() >= 3 && args.size() % 2 != 0) {
                std::vector<std::pair<std::string, std::string>> keyValues;
                for (size_t i = 1; i < args.size(); i += 2) {
                    keyValues.push_back({args[i], args[i+1]});
                }
                
                db->mset(keyValues);
                job->packetResponseManager->queueResponse("+OK\r\n");
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'mset' command\r\n");
            }
        });

        registerCommand("MGET", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() >= 2) {
                // Extract all keys
                std::vector<std::string> keys(args.begin() + 1, args.end());
                auto results = db->mget(keys);
                
                // Build a RESP Array response
                std::string response = "*" + std::to_string(results.size()) + "\r\n";
                for (const auto& optStr : results) {
                    if (optStr.has_value()) {
                        response += "$" + std::to_string(optStr.value().length()) + "\r\n" + optStr.value() + "\r\n";
                    } else {
                        response += "$-1\r\n"; // Nil response for missing/wrong-type keys
                    }
                }
                job->packetResponseManager->queueResponse(response);
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'mget' command\r\n");
            }
        });
    }

    void Dispatcher::registeHashCommands(std::shared_ptr<RedisDatabase> db){
        registerCommand("HSET", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 4) {
                try {
                    db->hset(args[1], args[2], args[3]);
                    job->packetResponseManager->queueResponse("+OK\r\n");
                } catch (...) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments\r\n");
            }
        });

        registerCommand("HGET", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    auto result = db->hget(args[1], args[2]);
                    if (result.has_value()) {
                        job->packetResponseManager->queueResponse("$" + std::to_string(result.value().length()) + "\r\n" + result.value() + "\r\n");
                    } else {
                        job->packetResponseManager->queueResponse("$-1\r\n");
                    }
                } catch (...) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments\r\n");
            }
        });
        
        // COMMAND: HDEL key field
        registerCommand("HDEL", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    size_t removed = db->hdel(args[1], args[2]);
                    
                    // Redis integer reply format: :<number>\r\n
                    job->packetResponseManager->queueResponse(":" + std::to_string(removed) + "\r\n");
                    
                } catch (const std::invalid_argument& e) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'hdel' command\r\n");
            }
        });

        // COMMAND: HEXISTS key field
        registerCommand("HEXISTS", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    auto result = db->hexists(args[1], args[2]);
                    int exists = result.value_or(0); // If key doesn't exist, return 0
                    job->packetResponseManager->queueResponse(":" + std::to_string(exists) + "\r\n");
                } catch (...) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'hexists' command\r\n");
            }
        });

        // COMMAND: HLEN key
        registerCommand("HLEN", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 2) {
                try {
                    auto result = db->hlen(args[1]);
                    size_t len = result.value_or(0); // If key doesn't exist, length is 0
                    job->packetResponseManager->queueResponse(":" + std::to_string(len) + "\r\n");
                } catch (...) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'hlen' command\r\n");
            }
        });

        // COMMAND: HGETDEL key field
        registerCommand("HGETDEL", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    auto result = db->hgetdel(args[1], args[2]);
                    if (result.has_value()) {
                        job->packetResponseManager->queueResponse("$" + std::to_string(result.value().length()) + "\r\n" + result.value() + "\r\n");
                    } else {
                        job->packetResponseManager->queueResponse("$-1\r\n"); // Field or Key didn't exist
                    }
                } catch (...) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'hgetdel' command\r\n");
            }
        });
    }

    void Dispatcher::registeListCommands(std::shared_ptr<RedisDatabase> db){
        registerCommand("LPUSH", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() >= 3) {
                try {
                    std::vector<std::string> elements(args.begin() + 2, args.end());
                    int newLen = db->lpush(args[1], elements);
                    job->packetResponseManager->queueResponse(":" + std::to_string(newLen) + "\r\n");
                } catch (...) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments\r\n");
            }
        });

        registerCommand("LPOP", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 2) {
                try {
                    auto result = db->lpop(args[1]);
                    if (result.has_value()) {
                        job->packetResponseManager->queueResponse("$" + std::to_string(result.value().length()) + "\r\n" + result.value() + "\r\n");
                    } else {
                        job->packetResponseManager->queueResponse("$-1\r\n");
                    }
                } catch (...) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments\r\n");
            }
        });
    
        registerCommand("RPUSH", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() >= 3) {
                try {
                    std::vector<std::string> elements(args.begin() + 2, args.end());
                    int newLength = db->rpush(args[1], elements);
                    job->packetResponseManager->queueResponse(":" + std::to_string(newLength) + "\r\n");
                } catch (const std::invalid_argument& e) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'rpush' command\r\n");
            }
        });

        registerCommand("RPOP", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 2) {
                try {
                    auto result = db->rpop(args[1]);
                    if (result.has_value()) {
                        job->packetResponseManager->queueResponse("$" + std::to_string(result.value().length()) + "\r\n" + result.value() + "\r\n");
                    } else {
                        job->packetResponseManager->queueResponse("$-1\r\n"); // Key didn't exist or list was empty
                    }
                } catch (const std::invalid_argument& e) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'rpop' command\r\n");
            }
        });
    }

    void Dispatcher::registeSetCommands(std::shared_ptr<RedisDatabase> db){
        registerCommand("SADD", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() >= 3) {
                try {
                    std::vector<std::string> members(args.begin() + 2, args.end());
                    int added = db->sadd(args[1], members);
                    job->packetResponseManager->queueResponse(":" + std::to_string(added) + "\r\n");
                } catch (...) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments\r\n");
            }
        });

        registerCommand("SISMEMBER", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    auto exists = db->sismember(args[1], args[2]);
                    job->packetResponseManager->queueResponse(":" + std::to_string(exists.value_or(0)) + "\r\n");
                } catch (...) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments\r\n");
            }
        });

        // COMMAND: SCARD key
        registerCommand("SCARD", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 2) {
                try {
                    size_t count = db->scard(args[1]);
                    job->packetResponseManager->queueResponse(":" + std::to_string(count) + "\r\n");
                } catch (const std::invalid_argument& e) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'scard' command\r\n");
            }
        });

        // COMMAND: SREM key member [member ...]
        registerCommand("SREM", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() >= 3) {
                try {
                    // Extract all members to be removed
                    std::vector<std::string> members(args.begin() + 2, args.end());
                    
                    int removed = db->srem(args[1], members);
                    job->packetResponseManager->queueResponse(":" + std::to_string(removed) + "\r\n");
                } catch (const std::invalid_argument& e) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'srem' command\r\n");
            }
        });
    }

    void Dispatcher::registeZSetCommands(std::shared_ptr<RedisDatabase> db){
        registerCommand("ZADD", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 4) {
                try {
                    double score = std::stod(args[2]);
                    int added = db->zadd(args[1], score, args[3]);
                    job->packetResponseManager->queueResponse(":" + std::to_string(added) + "\r\n");
                } catch (const std::invalid_argument& e) {
                    if (std::string(e.what()) == "WRONGTYPE") {
                        job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                    } else {
                        job->packetResponseManager->queueResponse("-ERR value is not a valid float\r\n");
                    }
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments\r\n");
            }
        });

        registerCommand("ZRANGE", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 4) {
                try {
                    int start = std::stoi(args[2]);
                    int stop = std::stoi(args[3]);
                    auto elements = db->zrange(args[1], start, stop);
                    
                    std::string response = "*" + std::to_string(elements.size()) + "\r\n";
                    for (const auto& el : elements) {
                        response += "$" + std::to_string(el.length()) + "\r\n" + el + "\r\n";
                    }
                    job->packetResponseManager->queueResponse(response);
                } catch (...) {
                    job->packetResponseManager->queueResponse("-ERR value is not an integer or wrong type\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments\r\n");
            }
        });
    
        // COMMAND: ZCARD key
        registerCommand("ZCARD", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 2) {
                try {
                    size_t count = db->zcard(args[1]);
                    job->packetResponseManager->queueResponse(":" + std::to_string(count) + "\r\n");
                } catch (...) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'zcard' command\r\n");
            }
        });

        // COMMAND: ZCOUNT key min max
        registerCommand("ZCOUNT", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 4) {
                try {
                    double min = std::stod(args[2]);
                    double max = std::stod(args[3]);
                    size_t count = db->zcount(args[1], min, max);
                    job->packetResponseManager->queueResponse(":" + std::to_string(count) + "\r\n");
                } catch (const std::invalid_argument& e) {
                    if (std::string(e.what()) == "WRONGTYPE") {
                        job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                    } else {
                        job->packetResponseManager->queueResponse("-ERR min or max is not a float\r\n");
                    }
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'zcount' command\r\n");
            }
        });

        // COMMAND: ZRANK key member
        registerCommand("ZRANK", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    auto rank = db->zrank(args[1], args[2]);
                    if (rank.has_value()) {
                        job->packetResponseManager->queueResponse(":" + std::to_string(rank.value()) + "\r\n");
                    } else {
                        job->packetResponseManager->queueResponse("$-1\r\n"); // Member or key doesn't exist
                    }
                } catch (...) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'zrank' command\r\n");
            }
        });

        // COMMAND: ZSCORE key member
        registerCommand("ZSCORE", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    auto score = db->zscore(args[1], args[2]);
                    if (score.has_value()) {
                        // Formatting the double as a string. Removing trailing zeros is ideal but this works perfectly.
                        std::string scoreStr = std::to_string(score.value());
                        scoreStr.erase(scoreStr.find_last_not_of('0') + 1, std::string::npos);
                        if (scoreStr.back() == '.') scoreStr.pop_back();

                        job->packetResponseManager->queueResponse("$" + std::to_string(scoreStr.length()) + "\r\n" + scoreStr + "\r\n");
                    } else {
                        job->packetResponseManager->queueResponse("$-1\r\n"); // Member or key doesn't exist
                    }
                } catch (...) {
                    job->packetResponseManager->queueResponse("-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->packetResponseManager->queueResponse("-ERR wrong number of arguments for 'zscore' command\r\n");
            }
        });
    }



}