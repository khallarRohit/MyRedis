#include "packetmanager.h"


namespace MyRedis{

    PacketManager::PacketManager(std::shared_ptr<SharedLock> ctx){
        packetQueue = ProcessQueue::getInstance(ctx);
        createPacket();
    }

    void PacketManager::processPacket(){
        packetQueue->emplace(packet);
        createPacket();
    }

    std::shared_ptr<ProcessQueue> PacketManager::getInstance(){
        return packetQuery;
    };

    std::shared_ptr<Packet> PacketManager::getPacket(){
        return packet;
    }

    void PacketManager::createPacket(){
        if(packet != nullptr) packet.reset();
        packet = std::make_shared<Packet>(); 
    }

}