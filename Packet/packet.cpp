#include "packet.h"

namespace MyRedis{

    Packet::Packet(const QueryType& queryType) {
        if(queryType == GET){
            initializeAsGetQuery();
        }else{
            initializeAsPostQuery();
        }
    };

    void Packet::resolveTask(const uint32_t& bytesReceived){
        this->packetQuery->resolveTask(bytesReceived);   
    }

    uint32_t Packet::getExtractionOffset(){
        return this->packetQuery->getExtractionOffset();
    }

    void Packet::clear(){
        this->packetQuery->clear();
    }

    PacketTask Packet::getTask(){
        return this->packetQuery->getTask();
    }
    
    BufferState Packet::getState(){
        return this->packetQuery->getState();
    }

    void Packet::initializeAsGetQuery(){
        if(packetQuery != nullptr){
            return;
        }
        this->packetQuery = std::make_unique<GetQuery>();
    }
    void Packet::initializeAsPostQuery(){
        if(packetQuery != nullptr){
            return;
        }
        this->packetQuery = std::make_unique<PostQuery>();
    }

}