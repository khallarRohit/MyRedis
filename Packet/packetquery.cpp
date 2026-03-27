#include "packetquery.h"


namespace MyRedis{

    PacketTask PacketQuery::getTask() const {
        return packetTask;
    }

    BufferState PacketQuery::getState() const {
        return state;
    }

    uint32_t PacketQuery::getExtractionOffset() const {
        return extractionOffset;
    }

    void PacketQuery::cleanBuffer() {
        state = BufferState::NOTFULL;
    }

    void PacketQuery::cleanTask() {
        packetTask = PacketTask::DSTYPE;
    }

    QueryType GetQuery::getQueryType() const{
        return GET;
    }

    void GetQuery::clear(){
        dataStructureType = -1;
        dataStructureNameSize = -1;
        dataStructureName.clear();
        commandNumber = -1;
        keySize = -1;
        key.clear();
        cleanBuffer();
        cleanTask();
        extractionOffset = 0;
    }

    char* GetQuery::getCurrentTargetBuffer(){
        switch(packetTask) {
            case PacketTask::DSTYPE:    return (char*)&dataStructureType + extractionOffset;
            case PacketTask::DSNAMESZ:  return (char*)&dataStructureNameSize + extractionOffset;
            case PacketTask::DSNAME:    return dataStructureName.data() + extractionOffset;
            case PacketTask::DSCOMMAND: return (char*)&commandNumber + extractionOffset;
            case PacketTask::KEYSZ:     return (char*)&keySize + extractionOffset;
            case PacketTask::KEY:       return key.data() + extractionOffset;
            default: return nullptr;
        }    
    }

    int32_t GetQuery::getRemainingBufferSize() const{
        switch(packetTask) {
            case PacketTask::DSTYPE:    return sizeof(dataStructureType) - extractionOffset;
            case PacketTask::DSNAMESZ:  return sizeof(dataStructureNameSize) - extractionOffset;
            case PacketTask::DSNAME:    return dataStructureNameSize - extractionOffset;
            case PacketTask::DSCOMMAND: return sizeof(commandNumber) - extractionOffset;
            case PacketTask::KEYSZ:     return sizeof(keySize) - extractionOffset;
            case PacketTask::KEY:       return keySize - extractionOffset;
            default: return 0;
        }
    }

    void GetQuery::resolveTask(const uint32_t& bytesReceived){
        extractionOffset += bytesReceived;

        if(packetTask == PacketTask::DSTYPE){
            if(extractionOffset == 4){
                dataStructureType = ntohl(dataStructureType);
                packetTask = PacketTask::DSNAMESZ;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::DSNAMESZ){
            if(extractionOffset == 4){
                dataStructureNameSize = ntohl(dataStructureNameSize);
                dataStructureName.resize(dataStructureNameSize);
                packetTask = PacketTask::DSNAME;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::DSNAME){
            if(extractionOffset == dataStructureNameSize){
                packetTask = PacketTask::DSCOMMAND;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::DSCOMMAND){
            if(extractionOffset == 4){
                commandNumber = ntohl(commandNumber);
                packetTask = PacketTask::KEYSZ;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::KEYSZ){
            if(extractionOffset == 4){
                keySize = ntohl(keySize);
                key.resize(keySize);
                packetTask = PacketTask::KEY;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::KEY){
            if(extractionOffset == keySize){
                packetTask = PacketTask::DONE;
                extractionOffset = 0;
                state = BufferState::FULL;
            }
        }
    }

    QueryType PostQuery::getQueryType() const{
        return POST;
    }

    char* PostQuery::getCurrentTargetBuffer(){
        switch(packetTask) {
            case PacketTask::DSTYPE:    return (char*)&dataStructureType + extractionOffset;
            case PacketTask::DSNAMESZ:  return (char*)&dataStructureNameSize + extractionOffset;
            case PacketTask::DSNAME:    return dataStructureName.data() + extractionOffset;
            case PacketTask::DSCOMMAND: return (char*)&commandNumber + extractionOffset;
            case PacketTask::KEYSZ:     return (char*)&keySize + extractionOffset;
            case PacketTask::KEY:       return key.data() + extractionOffset;
            case PacketTask::VALUEDT:   return (char*)&valueDataType + extractionOffset;
            case PacketTask::VALUESZ:   return (char*)&valueSize + extractionOffset;
            case PacketTask::VALUE:     return value.data() + extractionOffset;
            default: return nullptr;
        }
    }

    int32_t PostQuery::getRemainingBufferSize() const{
        switch(packetTask) {
            case PacketTask::DSTYPE:    return sizeof(dataStructureType) - extractionOffset;
            case PacketTask::DSNAMESZ:  return sizeof(dataStructureNameSize) - extractionOffset;
            case PacketTask::DSNAME:    return dataStructureNameSize - extractionOffset;
            case PacketTask::DSCOMMAND: return sizeof(commandNumber) - extractionOffset;
            case PacketTask::KEYSZ:     return sizeof(keySize) - extractionOffset;
            case PacketTask::KEY:       return keySize - extractionOffset;
            case PacketTask::VALUEDT:   return sizeof(valueDataType) - extractionOffset;
            case PacketTask::VALUESZ:   return sizeof(valueSize) - extractionOffset;
            case PacketTask::VALUE:     return valueSize - extractionOffset;
            default: return 0;
        }
    }

    void PostQuery::clear(){
        dataStructureType = -1;
        dataStructureNameSize = -1;
        dataStructureName.clear();
        commandNumber = -1;
        keySize = -1;
        key.clear();
        valueDataType = -1;
        valueSize = -1;
        value.clear();
        cleanBuffer();
        cleanTask();
        extractionOffset = 0;
    }

    void PostQuery::resolveTask(const uint32_t& bytesReceived){
        extractionOffset += bytesReceived;

        if(packetTask == PacketTask::DSTYPE){
            if(extractionOffset == 4){
                dataStructureType = ntohs(dataStructureType);
                packetTask = PacketTask::DSNAMESZ;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::DSNAMESZ){
            if(extractionOffset == 4){
                dataStructureNameSize = ntohs(dataStructureNameSize);
                packetTask = PacketTask::DSNAME;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::DSNAME){
            if(extractionOffset == dataStructureNameSize){
                packetTask = PacketTask::DSCOMMAND;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::DSCOMMAND){
            if(extractionOffset == 4){
                commandNumber = ntohs(commandNumber);
                packetTask = PacketTask::KEYSZ;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::KEYSZ){
            if(extractionOffset == 4){
                keySize = ntohs(keySize);
                packetTask = PacketTask::KEY;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::KEY){
            if(extractionOffset == keySize){
                packetTask = PacketTask::VALUEDT;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::VALUEDT){
            if(extractionOffset == 4){
                valueDataType = ntohs(valueDataType);
                packetTask = PacketTask::VALUESZ;
                extractionOffset = 0;
            }
        }else if(packetTask == PacketTask::VALUESZ){
            if(extractionOffset == 4){
                valueSize = ntohs(valueSize);
                packetTask = PacketTask::VALUE;
                extractionOffset = 0;
            }   
        }else if(packetTask == PacketTask::VALUE){
            if(extractionOffset == valueSize){
                packetTask = PacketTask::DONE;
                extractionOffset = 0;
                state = BufferState::FULL;
            }
        }
    }
}