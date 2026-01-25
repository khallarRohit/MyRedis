#pragma once
#include "packet.h"
#include "ThreadPool/threadpool.h"
#include <queue>
#include <memory>


namespace MyRedis{

    class PacketManager{
    public:
        

    private:
        ProcessQueue* queue;
        Packet packet;
        PacketTask task{PacketTask::DSTYPE};

    };

}