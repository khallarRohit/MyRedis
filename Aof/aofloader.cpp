#include "aofloader.h"

namespace MyRedis {

    static bool readLine(std::ifstream& file, std::string& line) {
        line.clear();
        char c;
        while (file.get(c)) {
            if (c == '\r') {
                if (file.get(c) && c == '\n') {
                    return true; 
                }
            }
            line += c;
        }
        return !line.empty();
    }

    void AofLoader::load(const std::string& filePath) {
        std::ifstream file(filePath, std::ios::binary);
        if (!file.is_open()) {
            std::cout << "[AOF] No existing AOF file found. Starting fresh.\n";
            return;
        }

        std::cout << "[AOF] Loading dataset into memory...\n";
        
        std::string line;
        int commandsLoaded = 0;

        while (readLine(file, line)) {
            if (line.empty() || line[0] != '*') continue;

            int numArgs = std::stoi(line.substr(1));
            std::vector<std::string> args;
            args.reserve(numArgs);

            for (int i = 0; i < numArgs; ++i) {
                readLine(file, line);
                if (line.empty() || line[0] != '$') break;

                int strLen = std::stoi(line.substr(1));
                
                std::string argData;
                argData.resize(strLen);
                file.read(&argData[0], strLen);
                
                args.push_back(argData);

                char cr, lf;
                file.get(cr); file.get(lf);
            }

            if (args.size() == numArgs) {
                auto ghostJob = std::make_shared<ProcessJob>(std::move(args), nullptr, 0);
                ghostJob->isAofRecovery = true; 

                Dispatcher::getInstance().dispatch(ghostJob);
                commandsLoaded++;
            }
        }

        std::cout << "[AOF] Recovery complete. Restored " << commandsLoaded << " commands.\n";
    }
}