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
            std::vector<std::string> query = inPacket->popNextQuery();

            uint64_t ticket = issue_seq.fetch_add(1, std::memory_order_relaxed);

            auto job = std::make_shared<ProcessJob>(std::move(query), shared_from_this(), ticket);
            inQueue->emplace(job);
        }
    }

    void PacketManager::pushOrderedResponse(uint64_t ticket, const std::string& responseStr) {
        std::lock_guard<std::mutex> lock(writeMutex);
        
        // 2. Push to the client's min-heap
        response_pq.push({ticket, responseStr});

        // 3. Drain the min-heap into the actual outgoing TCP queue
        while (!response_pq.empty() && response_pq.top().ticket == next_ticket_to_send) {
            outQueue.push(std::make_shared<OutPacket>(response_pq.top().data));
            response_pq.pop();
            next_ticket_to_send++;
        }
    }

    void PacketManager::createInPacket(){
        inPacket.reset();
        inPacket = std::make_shared<InPacket>(); 
    }


    bool PacketManager::hasDataToSend() const{
        std::lock_guard<std::mutex> lock(writeMutex);
        return !outQueue.empty();
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
        }
    }

}