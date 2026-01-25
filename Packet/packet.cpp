#include "packet.h"

namespace MyRedis{

    Packet::Packet(std::size_t maxPacketSize = 1000)
    :maxPacketSize(maxPacketSize){}

    void Packet::append(const void* data, uint32_t size){
        if(buffer.size() + size > maxPacketSize){
            throw PacketException("Packet size exceeded max packet size.");
        }

        buffer.insert(buffer.end(), (char*)data, (char*)data+size);
    }

    void Packet::clear(){
        buffer.clear();
    }

    void Packet::changeState(BufferState state){
        this->state = state;
    }

    BufferState Packet::getState(){
        return state;
    }

}