#include "threadpool.h"


namespace MyRedis{

    ThreadPool::ThreadPool(){
        inQueue = InQueue::getInstance();
        dispatcher = Dispatcher::getInstance();
        initiate();
    }

    void ThreadPool::initiate(){
        for(int i=0;i<noOfThreads;i++){
            processThreads.emplace_back(&ThreadPool::workerLoop, this);
        }
    }

    std::shared_ptr<ThreadPool> ThreadPool::getInstance(){
        static std::shared_ptr<ThreadPool> instance = std::shared_ptr<ThreadPool>(new ThreadPool());
        return instance;
    }

    void ThreadPool::workerLoop(){
        while(true){
            std::shared_ptr<ProcessJob> job = inQueue->pop();
            if (!job) {
                return; 
            }

            dispatcher->dispatch(job);
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