#include "threadpool.h"


namespace MyRedis{

    ThreadPool* ThreadPool::poolInstance{nullptr};

    ThreadPool::ThreadPool(std::shared_ptr<SharedLock> ctx, const int noOfThreads)
    :noOfThreads(noOfThreads), ctx(ctx){
        instance = ProcessQueue::getInstance(ctx);
        initiate();
    }

    void ThreadPool::initiate(){
        for(int i=0;i<noOfThreads;i++){
            processThread.emplace_back(getPacket);
        }
    }

    ThreadPool* ThreadPool::getInstance(std::shared_ptr<SharedLock> ctx, const int noOfThreads){
        if(poolInstance == nullptr){
            return new ThreadPool(ctx, noOfThreads); 
        }
        return poolInstance;
    }

    ThreadPool::~ThreadPool(){
        {
            std::lock_guard<std::mutex> guard(ctx->queue_mtx);
            ctx->queue_done = true;
        }
        ctx->queue_cv.notify_all();
        for(auto &i:processThread){
            i.join();
        }
    }

}