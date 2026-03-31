#include "packetmanager.h"


namespace MyRedis{

    PacketManager::PacketManager(std::shared_ptr<SharedLock> ctx){
        inQueue = InQueue::getInstance(ctx);
    }

    void PacketManager::queueInPacket(){
        inQueue->emplace(inPacket);
        inPacket.reset();
    }

    std::shared_ptr<InPacket> PacketManager::getInPacket(){
        return inPacket;
    }

    void PacketManager::createInPacket(){
        inPacket.reset();
        inPacket = std::make_shared<InPacket>(); 
    }

    void PacketManager::queueResponse(std::shared_ptr<OutPacket> outPacket){
        std::lock_guard<std::mutex> lock(writeMutex);
        outQueue.push_back(outPacket);
    }

    bool PacketManager::hasDataToSend() const{
        // std::lock_guard<std::mutex> lock(writeMutex); // TODO
        return !outQueue.empty();
    }
}