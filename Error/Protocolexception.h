#pragma once
#include <system_error>
#include <string>
#include "error.h"

namespace MyRedis {

    class ProtocolException : public std::system_error {
    public:
        explicit ProtocolException(const std::string& specificReason);
        explicit ProtocolException(const char* specificReason);
        
        virtual ~ProtocolException() noexcept = default;
    };

}