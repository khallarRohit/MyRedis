#include "packet.h"

namespace MyRedis{

    Packet::Packet(){};

    void Packet::resolveTask(uint32_t bytesReceived){
        extractionOffset += bytesReceived;

        if(packetTask == PacketTask::DSTYPE){
            if(extractionOffset == 4){
                packetTask = PacketTask::DSNAMESZ;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::DSNAMESZ){
            if(extractionOffset == 4){
                packetTask = PacketTask::DSNAME;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::DSNAME){
            if(extractionOffset == packetQuery.dataStructureNameSize){
                packetTask = PacketTask::DSCOMMAND;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::DSCOMMAND){
            if(extractionOffset == 4){
                packetTask = PacketTask::KEYSZ;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::KEYSZ){
            if(extractionOffset == 4){
                packetTask = PacketTask::KEY;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::KEY){
            if(extractionOffset == packetQuery.keySize){
                packetTask = PacketTask::VALUEDT;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::VALUEDT){
            if(extractionOffset == 4){
                packetTask = PacketTask::VALUESZ;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::VALUESZ){
            if(extractionOffset == 4){
                packetTask = PacketTask::VALUE;
                extractionOffset = 0;
            }   
        }else if(packetTask == PacketTask::VALUE){
            if(extractionOffset == packetQuery.valueSize){
                packetTask = PacketTask::DONE;
                extractionOffset = 0;
                state = BufferState::FULL;
            }
        }
    }

    void Packet::clear(){
        packetQuery.clear();
        cleanBuffer();
        cleanTask();
        extractionOffset = 0;
    }

    PacketTask Packet::getTask(){
        return packetTask;
    }

    void Packet::cleanBuffer(){
        this->state = BufferState::NOTFULL;
    }

    void Packet::cleanTask(){
        this->packetTask = PacketTask::DSTYPE;
    }

    BufferState Packet::getState(){
        return state;
    }

}