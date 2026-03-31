#pragma once
#include <iostream>
#include "Packet/packet.h"
#include "SharedContext/sharedlock.h"
#include <queue>
#include <mutex>
#include <memory>

namespace MyRedis{

    class InQueue{
    public:
        static std::shared_ptr<InQueue> getInstance(std::shared_ptr<SharedLock> ctx);
        InQueue(const InQueue&) = delete;
        InQueue& operator=(const InQueue&) = delete;
        void emplace(std::shared_ptr<Packet>newPacket);
        void pop();

        ~InQueue();
        
    private:
        InQueue(std::shared_ptr<SharedLock> ctx);
        std::shared_ptr<SharedLock> ctx;
        static std::shared_ptr<InQueue> instance;
        std::queue<std::shared_ptr<Packet>> packetQueue;
    };

    
    
    

    class OutQueue{
    public:


    private:
        int32_t clientCount = 0;

    };

}