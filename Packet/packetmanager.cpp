#include "packetmanager.h"


namespace MyRedis{

    PacketManager::PacketManager(std::shared_ptr<SharedLock> ctx){
        processQueue = ProcessQueue::getInstance(ctx);
    }

    void PacketManager::processPacket(){
        processQueue->emplace(packet);
        packet.reset();
    }

    std::shared_ptr<ProcessQueue> PacketManager::getInstance(){
        return processQueue;
    };

    std::shared_ptr<Packet> PacketManager::getPacket(){
        return packet;
    }

    void PacketManager::createPacket(const QueryType& queryType){
        packet.reset();
        packet = std::make_shared<Packet>(queryType); 
    }

    void PacketManager::queueResponse(const std::string& response){
        std::lock_guard<std::mutex> lock(writeMutex);
        if(currentWriteBuffer.empty()){
            currentWriteBuffer = response;
            bytesSentOffset = 0;
        }else{
            pendingResponses.push(response);
        }
    }

    bool PacketManager::hasDataToSend() const{
        // std::lock_guard<std::mutex> lock(writeMutex); // TODO
        return !currentWriteBuffer.empty();
    }

    const char* PacketManager::getCurrentWriteBuffer() const{
        // std::lock_guard<std::mutex> lock(writeMutex); // TODO
        return currentWriteBuffer.data() + bytesSentOffset;
    }

    uint32_t PacketManager::getWriteRemainingSize() const {
        // std::lock_guard<std::mutex> lock(writeMutex); // TODO
        return currentWriteBuffer.size() - bytesSentOffset;
    }

    void PacketManager::resolveWrite(int32_t bytesSent){
        // std::lock_guard<std::mutex> lock(writeMutex); // TODO
        bytesSentOffset += bytesSent;
        
        if(bytesSentOffset >= currentWriteBuffer.size()){
            currentWriteBuffer.clear();
            bytesSentOffset = 0;
            
            if (!pendingResponses.empty()) {
                currentWriteBuffer = pendingResponses.front();
                pendingResponses.pop();
            }
        }
    }

}