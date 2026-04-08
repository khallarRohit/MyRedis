#include "packet.h"

namespace MyRedis{

    const BufferState InPacket::getBufferState() const{
        return bufferState;
    }

    bool InPacket::hasReadyQueries() const {
        return !readyQueries.empty();
    }

    std::vector<std::string> InPacket::popNextQuery() {
        if (readyQueries.empty()) return {};
        
        std::vector<std::string> nextQuery = readyQueries.front();
        readyQueries.pop();
        return nextQuery;
    }

    void InPacket::appendData(const char* data, int length){
        readBuffer.append(data, length);

        while(!readBuffer.empty()){
            if(currentState == RESPState::EXPECTING_ARRAY_LEN
            or currentState == RESPState::EXPECTING_BULK_LEN){
                size_t crlfPos = readBuffer.find("\r\n");
                if (crlfPos == std::string::npos) {
                    return;
                }

                std::string line = readBuffer.substr(0, crlfPos);
                readBuffer.erase(0, crlfPos + 2);

                if(currentState == RESPState::EXPECTING_ARRAY_LEN){
                    if(line[0] == '*'){
                        try{
                            expectedElements = std::stoi(line.substr(1));
                            currentQuery.clear();

                            if (expectedElements == 0) {
                                currentState = RESPState::EXPECTING_ARRAY_LEN; 
                            } else {
                                currentState = RESPState::EXPECTING_BULK_LEN;
                            }
                        }catch(...){
                            readBuffer.clear();
                            currentState = RESPState::EXPECTING_ARRAY_LEN;
                            return;
                        }

                    }
                }else if(currentState == RESPState::EXPECTING_BULK_LEN){
                    if(line[0] == '$'){
                        try{
                            currentBulkLength = std::stoi(line.substr(1));
                            currentState = RESPState::EXPECTING_BULK_DATA;
                        }catch(...){
                            readBuffer.clear();
                            currentState = RESPState::EXPECTING_ARRAY_LEN;
                            return;
                        }
                    }
                }

            }else if(currentState == RESPState::EXPECTING_BULK_DATA){
                int totalRequiredBytes = currentBulkLength + 2;
                if(readBuffer.length() < totalRequiredBytes){
                    return;
                }

                std::string argument = readBuffer.substr(0, currentBulkLength);
                currentQuery.push_back(argument);
                readBuffer.erase(0, totalRequiredBytes);

                if(currentQuery.size() == expectedElements){

                    readyQueries.push(currentQuery);
                    currentQuery.clear();

                    currentState = RESPState::EXPECTING_ARRAY_LEN;
                    expectedElements = 0;
                }else{
                    currentState = RESPState::EXPECTING_BULK_LEN;
                }
            }

        }
    }

    OutPacket::OutPacket(const std::string& data) 
    : writeBuffer(data) {}
    
    const char* OutPacket::getWriteBuffer() const{
        return writeBuffer.data() + bytesSentOffset;
    }

    uint32_t OutPacket::getWriteRemainingSize() const {
        return writeBuffer.size() - bytesSentOffset;
    }

    void OutPacket::resolveWrite(int32_t bytesSent){
        bytesSentOffset += bytesSent;
        
        if(bytesSentOffset >= writeBuffer.size()){
            writeBuffer.clear();
            bytesSentOffset = 0;
        }
    }

    bool OutPacket::isEmpty() const{
        return bytesSentOffset == writeBuffer.size();
    }

}