#pragma once
#include <cstdint>

namespace MyRedis{

        enum PacketTask{
            DSTYPE,
            DSNAME,
            DSCOMMAND,
            KEY,
            VALUEDT,
            VALUESZ,
            VALUE,
        };


    // struct PacketTask{

    //     const uint32_t PacketSize[7] = {
    //         4,
    //         8,
    //         12,
    //         120,
    //         124,
    //         128,
    //         128,
    //     };
    // };



    enum BufferState{
        FULL,
        NOTFULL,
    };

}