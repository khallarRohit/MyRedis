#include "packetexception.h"

namespace MyRedis{
    PacketException::PacketException(const std::string& exception)
    :exception(exception){}

    std::string PacketException::message(){
        return exception;
    }
}