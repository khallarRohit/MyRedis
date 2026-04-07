#pragma once
#include <string>
#include <system_error>

namespace MyRedis{
    void throwWSAError(const std::string& context);

    std::string getWSAMessage(int errorCode);
}

