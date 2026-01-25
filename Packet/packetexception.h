#pragma once
#include <string>

namespace MyRedis{

    class PacketException{
    public:
        PacketException(std::string exception)
        :exception(exception){}
    
        std::string message(){
            return exception;
        }

    private:
        std::string exception;
    };


}