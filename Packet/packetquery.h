#pragma once
#define WIN32_LEAN_AND_MEAN
#include <WinSock2.h>
#include "packettask.h"
#include <cstdint>
#include <vector>


namespace MyRedis{

    class PacketQuery{
    public:
        virtual void clear() = 0;
        virtual PacketTask getTask() const = 0;
        virtual BufferState getState() const = 0;
        virtual uint32_t getExtractionOffset() const = 0;
        virtual void resolveTask(const uint32_t& bytesReceived) = 0;
        virtual QueryType getQueryType() const = 0;
        virtual ~PacketQuery() = default;
        virtual char* getCurrentTargetBuffer() = 0;
        virtual int32_t getRemainingBufferSize() const = 0;

    protected:
        int32_t dataStructureType{-1};
        int32_t dataStructureNameSize{-1};
        std::vector<char> dataStructureName; // string 
        int32_t commandNumber{-1};
        int32_t keySize{-1};
        std::vector<char> key; // string 
        BufferState state{BufferState::NOTFULL};
        PacketTask packetTask{PacketTask::DSTYPE};
        uint32_t extractionOffset{};
        virtual void cleanBuffer() = 0;
        virtual void cleanTask() = 0;
    };

    class GetQuery: public PacketQuery{
    public:
        GetQuery() = default;
        void clear() override;
        QueryType getQueryType() const override;
        void resolveTask(const uint32_t& bytesReceived) override;
        char* getCurrentTargetBuffer() override;
        int32_t getRemainingBufferSize() const override;
        ~GetQuery() = default;
    };

    class PostQuery: public PacketQuery{
    public:
        PostQuery() = default;
        void clear() override;
        QueryType getQueryType() const override;
        void resolveTask(const uint32_t& bytesReceived) override;
        char* getCurrentTargetBuffer() override;
        int32_t getRemainingBufferSize() const override;
        ~PostQuery() = default;
        
    private:
        int32_t valueDataType{-1};
        int32_t valueSize{-1};
        std::vector<char> value; // string, boolean, int
    };

}