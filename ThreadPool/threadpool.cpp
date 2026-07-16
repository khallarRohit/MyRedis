#include "threadpool.h"
#include "PacketQueue/queue.h"
#include "Dispatcher/dispatcher.h"

namespace MyRedis{

    ThreadPool::ThreadPool()
    :inQueue(InQueue::getInstance()){
        noOfThreads = std::thread::hardware_concurrency();

        if (noOfThreads <= 0) noOfThreads = 4;

        initiate();
    }

    void ThreadPool::initiate(){
        for(int i=0;i<noOfThreads;i++){
            processThreads.emplace_back(&ThreadPool::workerLoop, this);
        }
    }

    ThreadPool& ThreadPool::getInstance(){
        static ThreadPool instance;
        return instance;
    }

    void ThreadPool::workerLoop(){
        while(true){
            std::shared_ptr<ProcessJob> job = inQueue->pop();
            if (!job) {
                return; 
            }

            Dispatcher::getInstance().dispatch(std::move(job));
        }
    }

    ThreadPool::~ThreadPool(){
        inQueue->shutdown();

        for(auto &t:processThreads){
            if(t.joinable()){
                t.join();
            }
        }
    }

}