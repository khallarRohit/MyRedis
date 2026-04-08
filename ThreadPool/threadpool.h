#pragma once
#include <iostream>
#include <vector>
#include <mutex>
#include <condition_variable>
#include <thread>
#include <functional>
#include <memory>

namespace MyRedis{

    class InQueue;
    class Dispatcher;

    class ThreadPool{
    private:
        ThreadPool();

        int noOfThreads = 10;
        std::vector<std::thread> processThreads;        
        std::shared_ptr<InQueue> inQueue;
        
        void workerLoop();

    public:
        ThreadPool(const ThreadPool&) = delete;
        ThreadPool& operator=(const ThreadPool&) = delete;

        

        static ThreadPool& getInstance();
        void initiate();

        ~ThreadPool();
    };

}