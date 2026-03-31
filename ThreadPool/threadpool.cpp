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
            processThread.emplace_back(&ThreadPool::workerLoop, this);
        }
    }

    ThreadPool* ThreadPool::getInstance(std::shared_ptr<SharedLock> ctx, const int noOfThreads){
        if(poolInstance == nullptr){
            return new ThreadPool(ctx, noOfThreads); 
        }
        return poolInstance;
    }

    void ThreadPool::workerLoop(){
        while(true){
            {
                std::unique_lock<std::mutex> lock(ctx->queue_mtx);
                ctx->queue_cv.wait(lock, [this]() {
                    return ctx->queue_done || !packetQueue->empty();
                });

                if (ctx->queue_done && packetQueue->empty()) {
                    return; // Shutdown signal received, exit the thread cleanly
                }
                // manager = packetQueue->front(); 
                // packetQueue->pop();
            }

            if (manager) {
                // Get the parsed RESP strings (e.g., ["ECHO", "hello world"])
                std::vector<std::string> args = manager->getParsedArguments();
                
                // Let the CommandDispatcher magically route and execute the command!
                dispatcher.dispatch(manager, args);
            }
        }
    }

    ThreadPool::~ThreadPool(){
        {
            std::lock_guard<std::mutex> guard(ctx->queue_mtx);
            ctx->queue_done = true;
        }
        ctx->queue_cv.notify_all();
        for(auto &t:processThread){
            if(t.joinable()){
                t.join();
            }
        }
    }

}