#pragma once
// #include "packet.h"
#include "ThreadPool/threadpool.h"
#include <queue>
#include <memory>


namespace MyRedis{

    struct ProcessJob{
        std::vector<std::string>& packetQuery;
        std::shared_ptr<PacketResponseManager> packetResponseManager;
    };

    class PacketResponseManager{
    public:
        virtual void queueResponse(std::shared_ptr<OutPacket> outPacket) = 0;
    };

    class PacketManager: public PacketResponseManager{
    public:
        PacketManager(std::shared_ptr<SharedLock> ctx);
        PacketManager(const PacketManager&) = delete;
        PacketManager& operator=(const PacketManager&) = delete;

        // read methods
        void createInPacket();
        void queueInPacket();
        std::shared_ptr<InPacket> getInPacket();

        // write methods
        void queueResponse(std::shared_ptr<OutPacket> outPacket) override;
        bool hasDataToSend() const;

    private:
        // read state variables
        std::shared_ptr<InPacket> inPacket{nullptr};
        std::shared_ptr<InQueue> inQueue{nullptr};

        // write state variables
        mutable std::mutex writeMutex;
        std::deque<std::shared_ptr<OutPacket>> outQueue;

    };

}