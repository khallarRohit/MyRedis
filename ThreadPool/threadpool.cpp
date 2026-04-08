#include "threadpool.h"
#include "PacketQueue/queue.h"
#include "Dispatcher/dispatcher.h"

namespace MyRedis{

    ThreadPool::ThreadPool()
    :inQueue(InQueue::getInstance()){
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

            Dispatcher::getInstance().dispatch(job);
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