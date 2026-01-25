#include <iostream>
#include "Packet/packet.h"
#include "SharedContext/sharedlock.h"
#include <queue>
#include <mutex>
#include <memory>


namespace MyRedis{

    class ProcessQueue{
    public:
        static ProcessQueue* getInstance(std::shared_ptr<SharedLock> ctx);
        std::shared_ptr<SharedLock> ctx;
        std::queue<std::shared_ptr<Packet>> packetQueue;
        void emplace(std::shared_ptr<Packet>newPacket);
        void pop();
                
    private:
        ProcessQueue(std::shared_ptr<SharedLock> ctx);
        static ProcessQueue* instance;
    };
    

}