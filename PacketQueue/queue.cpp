#include "queue.h"

namespace MyRedis{

    std::shared_ptr<InQueue> InQueue::instance{nullptr};

    InQueue::InQueue(std::shared_ptr<SharedLock> ctx)
    :ctx(ctx){}

    std::shared_ptr<InQueue> InQueue::getInstance(std::shared_ptr<SharedLock> ctx){
        if(instance==nullptr){
            std::lock_guard<std::mutex> lock(ctx->queue_mtx);
            if(instance == nullptr){
                instance = std::make_shared<InQueue>(ctx);
            }
        }
        return instance;
    }

    std::shared_ptr<Packet> getpacket(std::shared_ptr<Packet>newPacket){
        return newPacket;
    }

    void InQueue::emplace(std::shared_ptr<Packet> newPacket){
        std::lock_guard<std::mutex> lock(ctx->queue_mtx);
        packetQueue.push(newPacket);
        ctx->queue_done = true;
        ctx->queue_cv.notify_one();
    }

    void InQueue::pop(){
        std::unique_lock<std::mutex> lock(ctx->queue_mtx);
        ctx->queue_cv.wait(lock, [this]{ return this->ctx->queue_done || !this->packetQueue.empty(); });
        packetQueue.pop();
    }
    
    InQueue::~InQueue(){
        instance.reset();
    }
}