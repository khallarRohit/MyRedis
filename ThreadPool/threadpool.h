#pragma once

#include <iostream>
#include <mutex>
#include <condition_variable>
#include "PacketQueue/queue.h"
#include "SharedContext/sharedlock.h"
#include <thread>
#include <functional>
#include <memory>

namespace MyRedis{

    class ThreadPool{
    private:
        ThreadPool(std::shared_ptr<SharedLock> ctx, const int noOfThreads);
        static ThreadPool* poolInstance;
        int noOfThreads;
        std::shared_ptr<SharedLock> ctx;      
        std::shared_ptr<ProcessQueue> packetQueue;
        std::vector<std::thread> processThread;
        std::function<void()> getPacket{
            [this](){
                while(true){
                    {
                        std::unique_lock<std::mutex> lock(ctx->queue_mtx);
                        ctx->queue_cv.wait(lock,[&](){
                            return ctx->queue_done || !instance->packetQueue.empty();
                        });
                        if(ctx->queue_done and instance->packetQueue.empty()) return;
                        instance->packetQueue.pop();
                    }
                    this->createQuery();
                    this->store();
                }
            }
        };
    public:
        static ThreadPool* getInstance(std::shared_ptr<SharedLock> ctx, const int noOfThreads);
        void initiate();
        void createQuery(); // create query from buffer
        void store(); // performs query to store
        ~ThreadPool();
    };

}