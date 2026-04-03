#include "object.h"

namespace MyRedis{

    class RedisString : public RedisObject{
    private:
        std::string value{};
    public:
        RedisString(const std::string& val);
        ~RedisString() override = default;

        DataType getType() const override;

        std::string get() const;
        void set(const std::string& val);
        std::string substr(int start, int stop) const;
    };

}