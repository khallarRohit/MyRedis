#pragma once
#include "Packet/packetmanager.h"
#include "store/database.h"
#include "AOF/aoflogger.h"
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

namespace MyRedis{

    class ProcessJob;

    using CommandHandler = std::function<void(std::shared_ptr<ProcessJob>)>;

    class Dispatcher{
    private:
        Dispatcher() = default;

        std::unordered_map<std::string, CommandHandler> registry;
        std::shared_ptr<AofLogger> aofLogger;

        void registerCommand(std::string name, CommandHandler handler);
        std::string reconstructRESP(const std::vector<std::string>& args);

    public:
        static Dispatcher& getInstance();

        ~Dispatcher() = default;

        Dispatcher(const Dispatcher& dispatcher) = delete;
        Dispatcher& operator=(const Dispatcher& dispatcher) = delete;

        void setAofLogger(std::shared_ptr<AofLogger> logger);

        void dispatch(std::shared_ptr<ProcessJob> job);

        void registerPING();
        void registerECHO();
        void registerCONFIG();

        void registerStringCommands(std::shared_ptr<RedisDatabase> db);
        void registeHashCommands(std::shared_ptr<RedisDatabase> db);
        void registeListCommands(std::shared_ptr<RedisDatabase> db);
        void registeSetCommands(std::shared_ptr<RedisDatabase> db);
        void registeZSetCommands(std::shared_ptr<RedisDatabase> db);
    };

}