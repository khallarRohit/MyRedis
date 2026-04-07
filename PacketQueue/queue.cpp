#include "queue.h"

namespace MyRedis{

    std::shared_ptr<InQueue> InQueue::getInstance(){
        static std::shared_ptr<InQueue> instance = std::shared_ptr<InQueue>(new InQueue());
        return instance;
    }

    void InQueue::emplace(std::shared_ptr<ProcessJob> job){
        std::lock_guard<std::mutex> lock(queue_mtx);
        queue.push(std::move(job));
        queue_cv.notify_one();
    }

    std::shared_ptr<ProcessJob> InQueue::pop(){
        std::unique_lock<std::mutex> lock(queue_mtx);
        queue_cv.wait(lock, [this]{
            return !this->queue.empty() || shutting_down; 
        });

        if (shutting_down && queue.empty()) {
            return nullptr;
        }

        auto job = std::move(queue.front());
        queue.pop();
        return job;
    }
    
    void InQueue::shutdown() {
        {
            std::lock_guard<std::mutex> lock(queue_mtx);
            shutting_down = true;
        }
        queue_cv.notify_all(); 
    }
}