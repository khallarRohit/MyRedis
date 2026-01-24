#pragma once

namespace MyRedis{

    enum class Error{
        InvalidIPv4,
        InvalidIPv6,
        UninitializedSocket,
    };

}