#pragma once
#include "PacketQueue/queue.h"
#include "Dispatcher/dispatcher.h"
#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <memory>

namespace MyRedis{

    class ThreadPool{
    private:
        ThreadPool();

        std::shared_ptr<Dispatcher> dispatcher;
        int noOfThreads = 10;
        
        std::shared_ptr<InQueue> inQueue;
        std::vector<std::thread> processThreads;

        void workerLoop();

    public:
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        static std::shared_ptr<ThreadPool> getInstance();
        void initiate();

        ~ThreadPool();
    };

}