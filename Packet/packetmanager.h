#pragma once
#include "packet.h"
#include "ThreadPool/threadpool.h"
#include <queue>
#include <memory>
#include <optional>
#include <atomic>
#include <mutex>


namespace MyRedis{

    class PacketManager;
    class InQueue;

    struct QueuedResponse {
        uint64_t ticket;
        std::string data;
        
        bool operator>(const QueuedResponse& other) const {
            return ticket > other.ticket;
        }
    };

    class PacketResponseManager{
    public:
        virtual ~PacketResponseManager() = default;
        virtual void pushOrderedResponse(uint64_t ticket, const std::string& responseStr) = 0;
    };

    class ProcessJob{
    public:
        std::vector<std::string> packetQuery;
        std::shared_ptr<PacketResponseManager> packetResponseManager;
        uint64_t ticket{};
        bool isAofRecovery{false};

        ProcessJob(std::vector<std::string> query, std::shared_ptr<PacketResponseManager> packetManager, uint64_t ticket);
        ~ProcessJob() = default;

        void sendReply(const std::string& reply) {
            if (isAofRecovery) {
                return; 
            }

            if (packetResponseManager) {
                packetResponseManager->pushOrderedResponse(ticket, reply); 
            }
        }
    };

    class PacketManager: public PacketResponseManager, public std::enable_shared_from_this<PacketManager>{
    public:
        PacketManager();
        ~PacketManager() = default;

        PacketManager(const PacketManager&) = delete;
        PacketManager& operator=(const PacketManager&) = delete;

        // read methods
        void processReceivedData(const char* data, int length);
        void createInPacket();

        // write methods
        void pushOrderedResponse(uint64_t ticket, const std::string& responseStr) override;
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
        std::atomic<int> outQueueSize{0};
        std::queue<std::shared_ptr<OutPacket>> outQueue;

        // Per-Client Sequencing State 
        std::atomic<uint64_t> issue_seq{0};
        uint64_t next_ticket_to_send{0};
        std::priority_queue<QueuedResponse, std::vector<QueuedResponse>, std::greater<QueuedResponse>> response_pq;
    };

}