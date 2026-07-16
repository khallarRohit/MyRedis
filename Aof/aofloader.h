#pragma once
#include "Dispatcher/dispatcher.h"
#include "store/database.h"
#include <fstream>
#include <iostream>
#include <charconv>
#include <string>
#include <vector>
#include <memory>

namespace MyRedis {

    class AofLoader {
    public:
        static void load(const std::string& filePath);
    };

}