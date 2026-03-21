#pragma once
#include <string>

namespace MyRedis{

    class PacketException{
    public:
        PacketException(const std::string& exception)
        :exception(exception){}
    
        std::string message(){
            return exception;
        }

    private:
        std::string exception;
    };


}