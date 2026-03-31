#pragma once
#include "Packet/packetmanager.h"
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

    public:
        void registerCommand(std::string name, CommandHandler handler);
        void dispatch(std::shared_ptr<ProcessJob> job);
        void registerPING();
        void registerECHO();
    };

}