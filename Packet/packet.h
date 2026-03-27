#pragma once
#include "packettask.h"
#include "packetquery.h"
#include "packetexception.h"
#include <vector>
#include <cstdint>
#include <memory>

namespace MyRedis{

    class Packet{
    public:
        Packet(const QueryType& queryType);
        void resolveTask(const uint32_t& bytesReceived);
        void clear();
        PacketTask getTask();
        BufferState getState();
        uint32_t getExtractionOffset();
        std::unique_ptr<PacketQuery> packetQuery{nullptr};
    private:
        void initializeAsGetQuery();
        void initializeAsPostQuery();
    };


}