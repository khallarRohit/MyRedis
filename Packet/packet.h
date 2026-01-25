#pragma once
#include "packettask.h"
#include "packetexception.h"
#include <vector>
#include <cstdint>


namespace MyRedis{

    class Packet{
    public:
        Packet(std::size_t maxPacketSize = 1000);
        void append(const void* data, uint32_t size);
        void clear();
        void changeState(BufferState state);
        BufferState getState();
    private:
        std::vector<char> buffer;
        BufferState state{BufferState::NOTFULL};
        std::size_t maxPacketSize{};
    };


}