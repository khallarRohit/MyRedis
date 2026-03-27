#pragma once
#include <cstdint>

namespace MyRedis{

    enum PacketTask{
        DSTYPE,
        DSNAMESZ,
        DSNAME,
        DSCOMMAND,
        KEYSZ,
        KEY,
        VALUEDT,
        VALUESZ,
        VALUE,
        DONE,
    };


    enum BufferState{
        FULL,
        NOTFULL,
    };

    enum QueryType{
        GET,
        POST
    };

}