#include "queue.h"


namespace MyRedis{

    ProcessQueue* ProcessQueue::instance{nullptr};

    ProcessQueue::ProcessQueue(std::shared_ptr<SharedLock> ctx)
    :ctx(ctx){
        std::cout << "Process Queue created successfully." << std::endl;
    }

    ProcessQueue* ProcessQueue::getInstance(std::shared_ptr<SharedLock> ctx){
        if(instance==nullptr){
            instance = new ProcessQueue(ctx);
        }
        return instance;
    }

    std::shared_ptr<Packet> getpacket(std::shared_ptr<Packet>newPacket){
        return newPacket;
    }

    void ProcessQueue::emplace(std::shared_ptr<Packet> newPacket){
        std::lock_guard<std::mutex> lock(ctx->queue_mtx);
        packetQueue.push(newPacket);
        ctx->queue_done = true;
        ctx->queue_cv.notify_one();
    }

    void ProcessQueue::pop(){
        std::unique_lock<std::mutex> lock(ctx->queue_mtx);
        ctx->queue_cv.wait(lock, [this]{ return ctx->queue_done; });
        packetQueue.pop();
    }
    

    
}