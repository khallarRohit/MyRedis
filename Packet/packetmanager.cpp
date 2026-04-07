#include "packetmanager.h"


namespace MyRedis{

    ProcessJob::ProcessJob(std::vector<std::string> query, PacketManager* packetManager)
    : packetQuery(query), packetResponseManager(packetManager){}


    PacketManager::PacketManager(){
        inQueue = InQueue::getInstance();
        inPacket = std::make_shared<InPacket>();
    }

    void PacketManager::processReceivedData(const char* data, int length) {
        inPacket->appendData(data, length);
        while (inPacket->hasReadyQueries()) {
            std::vector<std::string> query = inPacket->popNextQuery();
            auto job = std::make_shared<ProcessJob>(query, this);
            inQueue->emplace(job);
        }
    }

    void PacketManager::createInPacket(){
        inPacket.reset();
        inPacket = std::make_shared<InPacket>(); 
    }

    void PacketManager::queueResponse(const std::string& responseStr){
        std::lock_guard<std::mutex> lock(writeMutex);
        outQueue.push(std::make_shared<OutPacket>(responseStr));
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