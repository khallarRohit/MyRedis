#pragma once
#include "packettask.h"
#include "packetquery.h"
#include "packetexception.h"
#include <vector>
#include <cstdint>

namespace MyRedis{

    class Packet{
    public:
        Packet();
        void resolveTask(uint32_t bytesReceived);
        void clear();
        PacketTask getTask();
        BufferState getState();
    private:
        void cleanBuffer();
        void cleanTask();
        BufferState state{BufferState::NOTFULL};
        PacketTask packetTask{PacketTask::DSTYPE};
        PacketQuery packetQuery{};
        uint32_t extractionOffset{};
    };


}