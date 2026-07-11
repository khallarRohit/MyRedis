#include "aoflogger.h"
#include <iostream>

namespace MyRedis {

    AofLogger::AofLogger(const std::string& path) : filePath(path) {
        // Reserve memory to prevent reallocation during active pushes
        frontBuffer.reserve(1024);
        backBuffer.reserve(1024);
        
        // Start the background disk thread
        writerThread = std::thread(&AofLogger::backgroundWriteLoop, this);
    }

    AofLogger::~AofLogger() {
        stopFlag.store(true, std::memory_order_release);
        cv.notify_all(); // Wake up the thread if it's sleeping
        
        if (writerThread.joinable()) {
            writerThread.join();
        }
    }

    void AofLogger::logCommand(const std::string& rawRespCommand) {
        {
            // Lock only for the exact duration of the push
            std::lock_guard<std::mutex> lock(mtx);
            frontBuffer.push_back(rawRespCommand);
        }
        // Wake up the background thread to process it
        cv.notify_one();
    }

    void AofLogger::backgroundWriteLoop() {
        // Open file in Append and Binary mode
        std::ofstream file(filePath, std::ios::app | std::ios::binary);
        
        if (!file.is_open()) {
            std::cerr << "[AOF] Failed to open AOF file at " << filePath << "\n";
            return;
        }

        while (!stopFlag.load(std::memory_order_acquire)) {
            {
                std::unique_lock<std::mutex> lock(mtx);
                
                // Wait until there is data to write OR 1 second has passed (everysec policy) OR we are shutting down
                cv.wait_for(lock, std::chrono::seconds(1), [this]() {
                    return stopFlag.load(std::memory_order_acquire) || !frontBuffer.empty();
                });

                // Fast pointer swap! (O(1) operation)
                frontBuffer.swap(backBuffer);
            } // MUTEX UNLOCKED HERE. Worker threads can resume pushing immediately.

            // If we have data, write it to disk (completely isolated from the rest of the server)
            if (!backBuffer.empty()) {
                for (const auto& cmd : backBuffer) {
                    file.write(cmd.c_str(), cmd.size());
                }
                file.flush(); // Force OS to sync buffer to disk (similar to fsync)
                backBuffer.clear(); // Empty the back buffer for the next swap
            }
        }
        
        // Final flush on shutdown
        file.flush();
    }
}