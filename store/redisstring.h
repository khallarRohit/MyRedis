#include "object.h"

namespace MyRedis{

    class RedisString : public RedisObject{
    private:
        std::string value;
    public:
        RedisString(const std::string& val);
        DataType getType() const override;
        std::string getValue() const;
        void setValue(const std::string& val);
    };

}