#include "WSAexception.h"
#include <WS2tcpip.h>

namespace MyRedis{
    void throwWSAError(const std::string& context){
        int errorCode = WSAGetLastError();
        std::error_code ec(errorCode, std::system_category());
        throw std::system_error(ec, context);
    }


    std::string getWSAMessage(int errorCode){
        const std::error_category& category = std::system_category();
        return category.message(errorCode);
    }
}



