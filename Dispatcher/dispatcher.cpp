#include "dispatcher.h"

namespace MyRedis{

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
        if(it == registry.end()){
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

}