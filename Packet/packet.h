#pragma once
#include "packetexception.h"
#include <vector>
#include <queue>
#include <cstdint>
#include <string>
#include <memory>

namespace MyRedis{
    enum BufferState{
        FULL,
        NOTFULL,
    };

    class InPacket{
    private:

        enum class RESPState{
            EXPECTING_ARRAY_LEN,
            EXPECTING_BULK_LEN,
            EXPECTING_BULK_DATA,
        };

        std::string readBuffer{""};
        std::vector<std::string> currentQuery;
        std::queue<std::vector<std::string>> readyQueries;

        BufferState bufferState{NOTFULL};
        RESPState currentState{RESPState::EXPECTING_ARRAY_LEN};
        int32_t expectedElements{0}; // how many strings in the array?
        int32_t currentBulkLength{0}; // how long is the current string?

    public:
        void appendData(const char* data, int length);
        const BufferState getBufferState() const;

        bool hasReadyQueries() const;
        std::vector<std::string> popNextQuery();
    };

    class OutPacket{
    public:
        OutPacket(const std::string& data);
        ~OutPacket() = default;
        
        bool isEmpty() const;
        const char* getWriteBuffer() const;
        uint32_t getWriteRemainingSize() const;
        void resolveWrite(int32_t bytesSent);

    private:
        std::string writeBuffer{""};
        int32_t bytesSentOffset{0};
    };

}