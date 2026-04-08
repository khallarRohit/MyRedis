#pragma once
#include <iostream>
#include "Packet/packet.h"
#include "Packet/packetmanager.h"
#include <condition_variable>
#include <queue>
#include <mutex>
#include <memory>


namespace MyRedis{
    class ProcessJob;

    class InQueue{
    public:
        InQueue(const InQueue&) = delete;
        InQueue& operator=(const InQueue&) = delete;

        static std::shared_ptr<InQueue> getInstance();

        void emplace(std::shared_ptr<ProcessJob> job);
        std::shared_ptr<ProcessJob> pop();
        void shutdown();

        ~InQueue() = default;
    private:
        InQueue() = default;
        bool shutting_down = false;

        std::mutex queue_mtx;
        std::condition_variable queue_cv;
        std::queue<std::shared_ptr<ProcessJob>> queue;
    };

}