#pragma once
#include <string>

namespace MyRedis{

    class PacketException{
    public:
        PacketException(const std::string& exception);
        std::string message();

    private:
        std::string exception;
    };


}