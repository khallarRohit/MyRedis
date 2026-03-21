#pragma once
//#include "packet.h"
//#include "ThreadPool/threadpool.h"
#include <queue>
#include <memory>


namespace MyRedis{

    class PacketManager{
    public:
        PacketManager(std::shared_ptr<SharedLock> ctx);
        PacketManager(const PacketManager&) = delete;
        PacketManager& operator=(const PacketManager&) = delete;
        void processPacket();
        std::shared_ptr<Packet> getPacket();
        std::shared_ptr<ProcessQueue> getInstance();
    private:
        void createPacket();
        std::shared_ptr<Packet> packet{nullptr};
        std::shared_ptr<ProcessQueue> packetQueue{nullptr};
    };

}