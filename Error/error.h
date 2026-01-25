#pragma once
#include <iostream>
#include <string>

namespace MyRedis{

    enum class Error{
        InvalidIPv4,
        InvalidIPv6,
        UninitializedSocket,
    };
}



