#pragma once
#include <cstdint>
#include <vector>

namespace MyRedis{

    struct PacketQuery{
        int32_t dataStructureType{-1};
        int32_t dataStructureNameSize{-1};
        std::vector<char> dataStructureName; // string 
        int32_t commandNumber{-1};
        int32_t keySize{-1};
        std::vector<char> key; // string 
        int32_t valueDataType{-1};
        int32_t valueSize{-1};
        std::vector<char> value; // string, boolean, int

        void clear();
        const bool isFull();
    };

}