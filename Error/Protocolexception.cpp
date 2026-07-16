#include "Protocolexception.h"

namespace MyRedis {

    ProtocolException::ProtocolException(const std::string& specificReason)
        : std::system_error(make_error_code(Error::ProtocolError), specificReason) {
    }

    ProtocolException::ProtocolException(const char* specificReason)
        : std::system_error(make_error_code(Error::ProtocolError), specificReason) {
    }

}