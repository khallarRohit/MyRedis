#pragma once
// #include "packet.h"
#include "ThreadPool/threadpool.h"
#include <queue>
#include <memory>


namespace MyRedis{

    class PacketManager{
    public:
        PacketManager(std::shared_ptr<SharedLock> ctx);
        PacketManager(const PacketManager&) = delete;
        PacketManager& operator=(const PacketManager&) = delete;

        // read methods
        void processPacket();
        std::shared_ptr<Packet> getPacket();
        std::shared_ptr<ProcessQueue> getInstance();
        void createPacket(const QueryType& queryType);

        // read state variables
        uint32_t extractionOffSet{0};
        uint32_t queryType{-1};

        // write methods
        void queueResponse(const std::string& response);
        bool hasDataToSend() const;
        const char* getCurrentWriteBuffer() const;
        uint32_t getWriteRemainingSize() const;
        void resolveWrite(int32_t bytesSent);

    private:
        // read state variables
        std::shared_ptr<Packet> packet{nullptr};
        std::shared_ptr<ProcessQueue> processQueue{nullptr};

        // write state variables
        mutable std::mutex writeMutex;
        std::queue<std::string> pendingResponses;
        std::string currentWriteBuffer{""};
        int32_t bytesSentOffset{0};
    };

}