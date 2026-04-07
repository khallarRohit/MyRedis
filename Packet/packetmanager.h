#pragma once
#include "packet.h"
#include "ThreadPool/threadpool.h"
#include <queue>
#include <memory>
#include <optional>


namespace MyRedis{

    class PacketManager;

    class ProcessJob{
    public:
        std::vector<std::string> packetQuery;
        std::shared_ptr<PacketResponseManager> packetResponseManager;

        ProcessJob(std::vector<std::string> query, PacketManager* packetManager);
        ~ProcessJob() = default;
    };

    class PacketResponseManager{
    public:
        virtual ~PacketResponseManager() = default;
        virtual void queueResponse(const std::string& responseStr) = 0;
    };

    class PacketManager: public PacketResponseManager{
    public:
        PacketManager();
        ~PacketManager() = default;

        PacketManager(const PacketManager&) = delete;
        PacketManager& operator=(const PacketManager&) = delete;

        // read methods
        void processReceivedData(const char* data, int length);
        void createInPacket();

        // write methods
        void queueResponse(const std::string& responseStr) override;
        bool hasDataToSend() const;
        const char* getWriteBuffer() const;
        std::optional<uint32_t> getWriteRemainingSize() const;
        void resolveWrite(int32_t bytesSent);

    private:
        // read state variables
        std::shared_ptr<InPacket> inPacket{nullptr};
        std::shared_ptr<InQueue> inQueue{nullptr};

        // write state variables
        mutable std::mutex writeMutex;
        std::queue<std::shared_ptr<OutPacket>> outQueue;
    };

}