#include "packetmanager.h"
#include "PacketQueue/queue.h"

namespace MyRedis{

    ProcessJob::ProcessJob(std::vector<std::string> query, std::shared_ptr<PacketResponseManager> packetManager, uint64_t ticket)
    : packetQuery(std::move(query)), packetResponseManager(std::move(packetManager)), ticket(ticket){}

    PacketManager::PacketManager(){
        inQueue = InQueue::getInstance();
        inPacket = std::make_shared<InPacket>();
    }

    void PacketManager::processReceivedData(const char* data, int length) {
        inPacket->appendData(data, length);

        while (inPacket->hasReadyQueries()) {
            uint64_t ticket = issue_seq.fetch_add(1, std::memory_order_relaxed);
            inQueue->emplace(std::make_shared<ProcessJob>(inPacket->popNextQuery(), shared_from_this(), ticket));
        }
    }

    void PacketManager::pushOrderedResponse(uint64_t ticket, const std::string& responseStr) {
        std::lock_guard<std::mutex> lock(writeMutex);
        
        response_pq.push({ticket, responseStr});

        while (!response_pq.empty() && response_pq.top().ticket == next_ticket_to_send) {
            outQueue.push(std::make_shared<OutPacket>(response_pq.top().data));
            response_pq.pop();
            next_ticket_to_send++;

            outQueueSize.fetch_add(1, std::memory_order_relaxed);
        }
    }

    void PacketManager::createInPacket(){
        inPacket.reset();
        inPacket = std::make_shared<InPacket>(); 
    }

    bool PacketManager::hasDataToSend() const{
        return outQueueSize.load(std::memory_order_relaxed) > 0;
    }

    const char* PacketManager::getWriteBuffer() const{
        std::lock_guard<std::mutex> lock(writeMutex);
        if(outQueue.empty()){
            return nullptr;
        }

        return outQueue.front()->getWriteBuffer();
    }

    std::optional<uint32_t> PacketManager::getWriteRemainingSize() const{
        std::lock_guard<std::mutex> lock(writeMutex);
        if(outQueue.empty()){
            return std::nullopt;
        }

        return outQueue.front()->getWriteRemainingSize();
    }

    void PacketManager::resolveWrite(int32_t bytesSent){
        std::lock_guard<std::mutex> lock(writeMutex);
        if(outQueue.empty()){
            return;
        }

        outQueue.front()->resolveWrite(bytesSent);
        if(outQueue.front()->isEmpty()){
            outQueue.pop();
            outQueueSize.fetch_sub(1, std::memory_order_relaxed);
        }
    }

}