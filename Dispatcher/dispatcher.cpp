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

    void Dispatcher::setAofLogger(std::shared_ptr<AofLogger> logger) {
        this->aofLogger = logger;
    }

    std::string Dispatcher::reconstructRESP(const std::vector<std::string>& args) {
        std::string resp = "*" + std::to_string(args.size()) + "\r\n";
        for (const auto& arg : args) {
            resp += "$" + std::to_string(arg.length()) + "\r\n" + arg + "\r\n";
        }
        return resp;
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
            job->sendReply("-ERR unknown command '" + commandName + "'\r\n");
        }
    }

    void Dispatcher::registerPING(){
        registerCommand("PING", [](std::shared_ptr<ProcessJob> job){
            if(job->packetQuery.size() == 1){
                job->sendReply( "+PONG\r\n");
            }else{
                std::string response = "$" + std::to_string(job->packetQuery[1].length()) + "\r\n" + job->packetQuery[1] + "\r\n";
                job->sendReply( response);
            }
        });
    }

    void Dispatcher::registerECHO(){
        registerCommand("ECHO", [](std::shared_ptr<ProcessJob> job){
            if(job->packetQuery.size() >= 2){
                std::string response = "$" + std::to_string(job->packetQuery[1].length()) + "\r\n" + job->packetQuery[1] + "\r\n";
                job->sendReply( response);
            }else{
                job->sendReply( "-ERR wrong number of arguments for 'echo' command\r\n");
            }
        });           
    }

    void Dispatcher::registerCONFIG() {
        registerCommand("CONFIG", [](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            
            // Ensure we have enough arguments e.g., ["CONFIG", "GET", "save"]
            if (args.size() >= 3) {
                std::string subCommand = args[1];
                std::transform(subCommand.begin(), subCommand.end(), subCommand.begin(), ::toupper);
                
                std::string param = args[2];
                std::transform(param.begin(), param.end(), param.begin(), ::tolower);

                if (subCommand == "GET") {
                    if (param == "save") {
                        // Empty save parameter means RDB is disabled
                        job->sendReply( "*2\r\n$4\r\nsave\r\n$0\r\n\r\n");
                        return;
                    } 
                    else if (param == "appendonly") {
                        // "no" means AOF is disabled
                        job->sendReply( "*2\r\n$10\r\nappendonly\r\n$2\r\nno\r\n");
                        return;
                    }
                }
            }
            
            job->sendReply( "-ERR unsupported CONFIG parameter\r\n");
        });
    }

    void Dispatcher::registerStringCommands(std::shared_ptr<RedisDatabase> db){
        registerCommand("SET", [this, db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                db->set(args[1], args[2]);
                job->sendReply( "+OK\r\n");

                if (this->aofLogger) {
                    this->aofLogger->logCommand(this->reconstructRESP(args));
                }
            } else {
                job->sendReply( "-ERR syntax error or wrong number of arguments\r\n");
            }
        });

        registerCommand("GET", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 2) {
                auto strObj = db->get(args[1]);
                if (strObj == nullptr) {
                    job->sendReply( "$-1\r\n");
                } else {
                    std::string val = strObj->get();
                    job->sendReply( "$" + std::to_string(val.length()) + "\r\n" + val + "\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'get' command\r\n");
            }
        });

        registerCommand("GETSET", [this, db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    auto oldVal = db->getset(args[1], args[2]);
                    if (oldVal.has_value()) {
                        job->sendReply( "$" + std::to_string(oldVal.value().length()) + "\r\n" + oldVal.value() + "\r\n");
                    } else {
                        job->sendReply( "$-1\r\n");
                    }

                    if (this->aofLogger) {
                        this->aofLogger->logCommand(this->reconstructRESP(args));
                    }
                } catch (...) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments\r\n");
            }
        });

        registerCommand("SUBSTR", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 4) {
                try {
                    int start = std::stoi(args[2]);
                    int stop = std::stoi(args[3]);
                    
                    std::string result = db->substr(args[1], start, stop);
                    job->sendReply( "$" + std::to_string(result.length()) + "\r\n" + result + "\r\n");
                    
                } catch (const std::invalid_argument& e) {
                    if (std::string(e.what()) == "WRONGTYPE") {
                        job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                    } else {
                        job->sendReply( "-ERR value is not an integer or out of range\r\n");
                    }
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'substr' command\r\n");
            }
        });

        registerCommand("MSET", [this, db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() >= 3 && args.size() % 2 != 0) {
                std::vector<std::pair<std::string, std::string>> keyValues;
                for (size_t i = 1; i < args.size(); i += 2) {
                    keyValues.push_back({args[i], args[i+1]});
                }
                
                db->mset(keyValues);
                job->sendReply( "+OK\r\n");

                if (this->aofLogger) {
                    this->aofLogger->logCommand(this->reconstructRESP(args));
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'mset' command\r\n");
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
                job->sendReply( response);
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'mget' command\r\n");
            }
        });
    }

    void Dispatcher::registeHashCommands(std::shared_ptr<RedisDatabase> db){
        registerCommand("HSET", [this, db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 4) {
                try {
                    db->hset(args[1], args[2], args[3]);
                    job->sendReply( "+OK\r\n");
                } catch (...) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }

                if (this->aofLogger) {
                    this->aofLogger->logCommand(this->reconstructRESP(args));
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments\r\n");
            }
        });

        registerCommand("HGET", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    auto result = db->hget(args[1], args[2]);
                    if (result.has_value()) {
                        job->sendReply( "$" + std::to_string(result.value().length()) + "\r\n" + result.value() + "\r\n");
                    } else {
                        job->sendReply( "$-1\r\n");
                    }
                } catch (...) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments\r\n");
            }
        });
        
        // COMMAND: HDEL key field
        registerCommand("HDEL", [this, db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    size_t removed = db->hdel(args[1], args[2]);
                    
                    // Redis integer reply format: :<number>\r\n
                    job->sendReply( ":" + std::to_string(removed) + "\r\n");

                    if (this->aofLogger) {
                        this->aofLogger->logCommand(this->reconstructRESP(args));
                    }
                } catch (const std::invalid_argument& e) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'hdel' command\r\n");
            }
        });

        // COMMAND: HEXISTS key field
        registerCommand("HEXISTS", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    auto result = db->hexists(args[1], args[2]);
                    int exists = result.value_or(0); // If key doesn't exist, return 0
                    job->sendReply( ":" + std::to_string(exists) + "\r\n");
                } catch (...) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'hexists' command\r\n");
            }
        });

        // COMMAND: HLEN key
        registerCommand("HLEN", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 2) {
                try {
                    auto result = db->hlen(args[1]);
                    size_t len = result.value_or(0); // If key doesn't exist, length is 0
                    job->sendReply( ":" + std::to_string(len) + "\r\n");
                } catch (...) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'hlen' command\r\n");
            }
        });

        // COMMAND: HGETDEL key field
        registerCommand("HGETDEL", [this, db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    auto result = db->hgetdel(args[1], args[2]);
                    if (result.has_value()) {
                        job->sendReply( "$" + std::to_string(result.value().length()) + "\r\n" + result.value() + "\r\n");
                    } else {
                        job->sendReply( "$-1\r\n"); // Field or Key didn't exist
                    }

                    if (this->aofLogger) {
                        this->aofLogger->logCommand(this->reconstructRESP(args));
                    }
                } catch (...) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'hgetdel' command\r\n");
            }
        });
    }

    void Dispatcher::registeListCommands(std::shared_ptr<RedisDatabase> db){
        registerCommand("LPUSH", [this, db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() >= 3) {
                try {
                    std::vector<std::string> elements(args.begin() + 2, args.end());
                    int newLen = db->lpush(args[1], elements);
                    job->sendReply( ":" + std::to_string(newLen) + "\r\n");

                    if (this->aofLogger) {
                        this->aofLogger->logCommand(this->reconstructRESP(args));
                    }
                } catch (...) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments\r\n");
            }
        });

        registerCommand("LPOP", [this, db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 2) {
                try {
                    auto result = db->lpop(args[1]);
                    if (result.has_value()) {
                        job->sendReply( "$" + std::to_string(result.value().length()) + "\r\n" + result.value() + "\r\n");
                    } else {
                        job->sendReply( "$-1\r\n");
                    }

                    if (this->aofLogger) {
                        this->aofLogger->logCommand(this->reconstructRESP(args));
                    }
                } catch (...) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments\r\n");
            }
        });
    
        registerCommand("RPUSH", [this, db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() >= 3) {
                try {
                    std::vector<std::string> elements(args.begin() + 2, args.end());
                    int newLength = db->rpush(args[1], elements);
                    job->sendReply( ":" + std::to_string(newLength) + "\r\n");

                    if (this->aofLogger) {
                        this->aofLogger->logCommand(this->reconstructRESP(args));
                    }
                } catch (const std::invalid_argument& e) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'rpush' command\r\n");
            }
        });

        registerCommand("RPOP", [this, db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 2) {
                try {
                    auto result = db->rpop(args[1]);
                    if (result.has_value()) {
                        job->sendReply( "$" + std::to_string(result.value().length()) + "\r\n" + result.value() + "\r\n");
                    } else {
                        job->sendReply( "$-1\r\n"); // Key didn't exist or list was empty
                    }

                    if (this->aofLogger) {
                        this->aofLogger->logCommand(this->reconstructRESP(args));
                    }
                } catch (const std::invalid_argument& e) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'rpop' command\r\n");
            }
        });
    }

    void Dispatcher::registeSetCommands(std::shared_ptr<RedisDatabase> db){
        registerCommand("SADD", [this, db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() >= 3) {
                try {
                    std::vector<std::string> members(args.begin() + 2, args.end());
                    int added = db->sadd(args[1], members);
                    job->sendReply( ":" + std::to_string(added) + "\r\n");

                    if (this->aofLogger) {
                        this->aofLogger->logCommand(this->reconstructRESP(args));
                    }
                } catch (...) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments\r\n");
            }
        });

        registerCommand("SISMEMBER", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    auto exists = db->sismember(args[1], args[2]);
                    job->sendReply( ":" + std::to_string(exists.value_or(0)) + "\r\n");
                } catch (...) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments\r\n");
            }
        });

        // COMMAND: SCARD key
        registerCommand("SCARD", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 2) {
                try {
                    size_t count = db->scard(args[1]);
                    job->sendReply( ":" + std::to_string(count) + "\r\n");
                } catch (const std::invalid_argument& e) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'scard' command\r\n");
            }
        });

        // COMMAND: SREM key member [member ...]
        registerCommand("SREM", [this, db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() >= 3) {
                try {
                    // Extract all members to be removed
                    std::vector<std::string> members(args.begin() + 2, args.end());
                    
                    int removed = db->srem(args[1], members);
                    job->sendReply( ":" + std::to_string(removed) + "\r\n");

                    if (this->aofLogger) {
                        this->aofLogger->logCommand(this->reconstructRESP(args));
                    }
                } catch (const std::invalid_argument& e) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'srem' command\r\n");
            }
        });
    }

    void Dispatcher::registeZSetCommands(std::shared_ptr<RedisDatabase> db){
        registerCommand("ZADD", [this, db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 4) {
                try {
                    double score = std::stod(args[2]);
                    int added = db->zadd(args[1], score, args[3]);
                    job->sendReply( ":" + std::to_string(added) + "\r\n");
        
                    if (this->aofLogger) {
                        this->aofLogger->logCommand(this->reconstructRESP(args));
                    }
                } catch (const std::invalid_argument& e) {
                    if (std::string(e.what()) == "WRONGTYPE") {
                        job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                    } else {
                        job->sendReply( "-ERR value is not a valid float\r\n");
                    }
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments\r\n");
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
                    job->sendReply( response);
                } catch (...) {
                    job->sendReply( "-ERR value is not an integer or wrong type\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments\r\n");
            }
        });
    
        // COMMAND: ZCARD key
        registerCommand("ZCARD", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 2) {
                try {
                    size_t count = db->zcard(args[1]);
                    job->sendReply( ":" + std::to_string(count) + "\r\n");
                } catch (...) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'zcard' command\r\n");
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
                    job->sendReply( ":" + std::to_string(count) + "\r\n");
                } catch (const std::invalid_argument& e) {
                    if (std::string(e.what()) == "WRONGTYPE") {
                        job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                    } else {
                        job->sendReply( "-ERR min or max is not a float\r\n");
                    }
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'zcount' command\r\n");
            }
        });

        // COMMAND: ZRANK key member
        registerCommand("ZRANK", [db](std::shared_ptr<ProcessJob> job) {
            const auto& args = job->packetQuery;
            if (args.size() == 3) {
                try {
                    auto rank = db->zrank(args[1], args[2]);
                    if (rank.has_value()) {
                        job->sendReply( ":" + std::to_string(rank.value()) + "\r\n");
                    } else {
                        job->sendReply( "$-1\r\n"); // Member or key doesn't exist
                    }
                } catch (...) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'zrank' command\r\n");
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

                        job->sendReply( "$" + std::to_string(scoreStr.length()) + "\r\n" + scoreStr + "\r\n");
                    } else {
                        job->sendReply( "$-1\r\n"); // Member or key doesn't exist
                    }
                } catch (...) {
                    job->sendReply( "-WRONGTYPE Operation against a key holding the wrong kind of value\r\n");
                }
            } else {
                job->sendReply( "-ERR wrong number of arguments for 'zscore' command\r\n");
            }
        });
    }
}