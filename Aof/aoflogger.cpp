#include "aoflogger.h"
#include <iostream>

namespace MyRedis {

    AofLogger::AofLogger(const std::string& path) : filePath(path) {
        frontBuffer.reserve(1024);
        backBuffer.reserve(1024);
        
        writerThread = std::thread(&AofLogger::backgroundWriteLoop, this);
    }

    AofLogger::~AofLogger() {
        stopFlag.store(true, std::memory_order_release);
        cv.notify_all(); 
        
        if (writerThread.joinable()) {
            writerThread.join();
        }
    }

    void AofLogger::logCommand(const std::string& rawRespCommand) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            frontBuffer.push_back(rawRespCommand);
        }
        cv.notify_one();
    }

    void AofLogger::backgroundWriteLoop() {
        std::ofstream file(filePath, std::ios::app | std::ios::binary);
        
        if (!file.is_open()) {
            std::cerr << "[AOF] Failed to open AOF file at " << filePath << "\n";
            return;
        }

        while (!stopFlag.load(std::memory_order_acquire)) {
            {
                std::unique_lock<std::mutex> lock(mtx);
                
                cv.wait_for(lock, std::chrono::seconds(1), [this]() {
                    return stopFlag.load(std::memory_order_acquire) || !frontBuffer.empty();
                });

                frontBuffer.swap(backBuffer);
            } 

            if (!backBuffer.empty()) {
                for (const auto& cmd : backBuffer) {
                    file.write(cmd.c_str(), cmd.size());
                }
                file.flush();
                backBuffer.clear(); 
            }
        }
        
        file.flush();
    }
}