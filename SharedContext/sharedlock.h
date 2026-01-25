#pragma once
#include <mutex>
#include <condition_variable>

namespace MyRedis{

    struct SharedLock{
        std::mutex queue_mtx;
        std::condition_variable queue_cv;
        bool queue_done{false};
    };

}

