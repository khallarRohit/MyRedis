#include "packet.h"

namespace MyRedis{

    const InPacket::BufferState InPacket::getBufferState() const{
        return bufferState;
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
                        expectedElements = std::stoi(line.substr(1));
                        packetQuery.clear();

                        if (expectedElements == 0) {
                            currentState = RESPState::EXPECTING_ARRAY_LEN; 
                        } else {
                            currentState = RESPState::EXPECTING_BULK_LEN;
                        }
                    }
                }else if(currentState == RESPState::EXPECTING_BULK_LEN){
                    if(line[0] == '$'){
                        currentBulkLength = std::stoi(line.substr(1));
                        currentState = RESPState::EXPECTING_BULK_DATA;
                    }
                }

            }else if(currentState == RESPState::EXPECTING_BULK_DATA){
                int totalRequiredBytes = currentBulkLength + 2;
                if(readBuffer.length() < totalRequiredBytes){
                    return;
                }

                std::string argument = readBuffer.substr(0, currentBulkLength);
                packetQuery.push_back(argument);

                readBuffer.erase(0, totalRequiredBytes);

                if(packetQuery.size() == expectedElements){

                    // TODO => process command

                    currentState = RESPState::EXPECTING_ARRAY_LEN;
                    expectedElements = 0;
                }else{
                    currentState = RESPState::EXPECTING_BULK_LEN;
                }
            }

        }
    }

    
    const char* OutPacket::getWriteBuffer() const{
        // std::lock_guard<std::mutex> lock(writeMutex); // TODO
        return writeBuffer.data() + bytesSentOffset;
    }

    uint32_t OutPacket::getWriteRemainingSize() const {
        // std::lock_guard<std::mutex> lock(writeMutex); // TODO
        return writeBuffer.size() - bytesSentOffset;
    }

    void OutPacket::resolveWrite(int32_t bytesSent){
        // std::lock_guard<std::mutex> lock(writeMutex); // TODO
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