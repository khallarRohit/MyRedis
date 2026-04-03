#pragma once
#include "Packet/packetmanager.h"
#include "store/database.h"
#include <unordered_map>
#include <functional>
#include <string>
#include <vector>
#include <memory>
#include <algorithm>

namespace MyRedis{

    using CommandHandler = std::function<void(std::shared_ptr<ProcessJob>)>;

    class Dispatcher{
    private:
        std::unordered_map<std::string, CommandHandler> registry;   
        void registerCommand(std::string name, CommandHandler handler);

    public:
        void dispatch(std::shared_ptr<ProcessJob> job);

        void registerPING();
        void registerECHO();

        void registerStringCommands(std::shared_ptr<RedisDatabase> db);
        void registeHashCommands(std::shared_ptr<RedisDatabase> db);
        void registeListCommands(std::shared_ptr<RedisDatabase> db);
        void registeSetCommands(std::shared_ptr<RedisDatabase> db);
        void registeZSetCommands(std::shared_ptr<RedisDatabase> db);
    };

}