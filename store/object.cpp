#include "object.h"

namespace MyRedis{

    void RedisObject::setExpiry(int milliseconds) {
        hasExpiry = true;
        expiresAt = std::chrono::system_clock::now() + std::chrono::milliseconds(milliseconds);
    }

    bool RedisObject::isExpired() const {
        if (!hasExpiry) return false;
        return std::chrono::system_clock::now() > expiresAt;
    }

}