#pragma once
#include <system_error>
#include <string>

namespace MyRedis {

    enum class Error {
        Success = 0,
        InvalidIP,
        InvalidIPv4,
        InvalidIPv6,
        UninitializedSocket,
        SocketNotBound,
        SocketBound,
    };

    // 1. The Category Class
    class RedisErrorCategory : public std::error_category {
    public:
        const char* name() const noexcept override {
            return "MyRedis";
        }

        std::string message(int ev) const override {
            switch (static_cast<Error>(ev)) {
                case Error::Success:             return "Success";
                case Error::InvalidIP:           return "Invalid IP Address format";
                case Error::InvalidIPv4:         return "Invalid IPv4 Address";
                case Error::InvalidIPv6:         return "Invalid IPv6 Address";
                case Error::UninitializedSocket: return "Socket has not been initialized";
                case Error::SocketNotBound:      return "Socket is not bound to a port";
                case Error::SocketBound:         return "Socket is already bound";
                default:                         return "Unknown Redis error";
            }
        }
    }; 

    
    inline const std::error_category& GetRedisCategory() {
        static RedisErrorCategory instance;
        return instance;
    }
    
    inline std::error_code make_error_code(Error e) {
        return std::error_code(static_cast<int>(e), GetRedisCategory());
    }
}

namespace std {
    template <>
    struct is_error_code_enum<MyRedis::Error> : true_type {};
}