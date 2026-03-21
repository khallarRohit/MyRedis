#pragma once
#include <iostream>
#include "Packet/packet.h"
#include "SharedContext/sharedlock.h"
#include <queue>
#include <mutex>
#include <memory>

namespace MyRedis{

    class ProcessQueue{
    public:
        static std::shared_ptr<ProcessQueue> getInstance(std::shared_ptr<SharedLock> ctx);
        ProcessQueue(const ProcessQueue&) = delete;
        ProcessQueue& operator=(const ProcessQueue&) = delete;
        
        void emplace(std::shared_ptr<Packet>newPacket);
        void pop();

        ~ProcessQueue();
        
    private:
        ProcessQueue(std::shared_ptr<SharedLock> ctx);
        std::shared_ptr<SharedLock> ctx;
        static std::shared_ptr<ProcessQueue> instance;
        std::queue<std::shared_ptr<Packet>> packetQueue;
    };
    

}