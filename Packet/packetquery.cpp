#include "packetquery.h"


namespace MyRedis{

    void PacketQuery::clear(){
        dataStructureType = -1;
        dataStructureNameSize = -1;
        dataStructureName.clear();
        commandNumber = -1;
        keySize = -1;
        key.clear();
        valueDataType = -1;
        valueSize = -1;
        value.clear();
    }

    const bool PacketQuery::isFull(){
        if(valueSize != -1 and value.size() == valueSize){
            return true;
        }
        return false;   
    }
}