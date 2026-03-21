#include "queue.h"

namespace MyRedis{

    std::shared_ptr<ProcessQueue> ProcessQueue::instance{nullptr};

    ProcessQueue::ProcessQueue(std::shared_ptr<SharedLock> ctx)
    :ctx(ctx){
        std::cout << "Process Queue created successfully." << std::endl;
    }

    std::shared_ptr<ProcessQueue> ProcessQueue::getInstance(std::shared_ptr<SharedLock> ctx){
        std::lock_guard<std::mutex> lock(ctx->queue_mtx);
        if(instance==nullptr){
            instance = std::make_shared<ProcessQueue>(ctx);
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
        ctx->queue_cv.wait(lock, [this]{ return this->ctx->queue_done || !this->packetQueue.empty(); });
        packetQueue.pop();
    }
    
    ProcessQueue::~ProcessQueue(){
        instance.reset();
    }
}