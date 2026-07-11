#pragma once
#include <string>
#include <vector>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <fstream>
#include <atomic>

namespace MyRedis {

    class AofLogger {
    private:
        std::vector<std::string> frontBuffer;
        std::vector<std::string> backBuffer;
        
        std::mutex mtx;
        std::condition_variable cv;
        std::atomic<bool> stopFlag{false};
        std::thread writerThread;
        
        std::string filePath;
        
        void backgroundWriteLoop();

    public:
        AofLogger(const std::string& path);
        ~AofLogger();

        // Prevent copying
        AofLogger(const AofLogger&) = delete;
        AofLogger& operator=(const AofLogger&) = delete;

        // Called by your worker threads to log a mutating command
        void logCommand(const std::string& rawRespCommand);
    };

}