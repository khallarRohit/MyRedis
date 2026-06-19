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

    // void InPacket::appendData(const char* data, int length){
    //     readBuffer.append(data, length);

    //     while(!readBuffer.empty()){
    //         if(currentState == RESPState::EXPECTING_ARRAY_LEN
    //         or currentState == RESPState::EXPECTING_BULK_LEN){
    //             size_t crlfPos = readBuffer.find("\r\n");
    //             if (crlfPos == std::string::npos) {
    //                 return;
    //             }

    //             std::string line = readBuffer.substr(0, crlfPos);
    //             readBuffer.erase(0, crlfPos + 2);

    //             if(currentState == RESPState::EXPECTING_ARRAY_LEN){
    //                 if(line[0] == '*'){
    //                     try{
    //                         expectedElements = std::stoi(line.substr(1));
    //                         currentQuery.clear();

    //                         if (expectedElements == 0) {
    //                             currentState = RESPState::EXPECTING_ARRAY_LEN; 
    //                         } else {
    //                             currentState = RESPState::EXPECTING_BULK_LEN;
    //                         }
    //                     }catch(...){
    //                         readBuffer.clear();
    //                         currentState = RESPState::EXPECTING_ARRAY_LEN;
    //                         return;
    //                     }

    //                 }
    //             }else if(currentState == RESPState::EXPECTING_BULK_LEN){
    //                 if(line[0] == '$'){
    //                     try{
    //                         currentBulkLength = std::stoi(line.substr(1));
    //                         currentState = RESPState::EXPECTING_BULK_DATA;
    //                     }catch(...){
    //                         readBuffer.clear();
    //                         currentState = RESPState::EXPECTING_ARRAY_LEN;
    //                         return;
    //                     }
    //                 }
    //             }

    //         }else if(currentState == RESPState::EXPECTING_BULK_DATA){
    //             int totalRequiredBytes = currentBulkLength + 2;
    //             if(readBuffer.length() < totalRequiredBytes){
    //                 return;
    //             }

    //             std::string argument = readBuffer.substr(0, currentBulkLength);
    //             currentQuery.push_back(argument);
    //             readBuffer.erase(0, totalRequiredBytes);

    //             if(currentQuery.size() == expectedElements){

    //                 readyQueries.push(currentQuery);
    //                 currentQuery.clear();

    //                 currentState = RESPState::EXPECTING_ARRAY_LEN;
    //                 expectedElements = 0;
    //             }else{
    //                 currentState = RESPState::EXPECTING_BULK_LEN;
    //             }
    //         }

    //     }
    // }

    void InPacket::appendData(const char* data, int length) {
        // 1. Append new bytes to the end of the buffer
        readBuffer.append(data, length);

        // 2. Process bytes starting from where we left off last time
        while (readIndex < readBuffer.length()) {
            
            if (currentState == RESPState::EXPECTING_ARRAY_LEN || 
                currentState == RESPState::EXPECTING_BULK_LEN) {
                
                // Search only in the unparsed portion of the buffer
                size_t crlfPos = readBuffer.find("\r\n", readIndex);
                if (crlfPos == std::string::npos) {
                    break; // Incomplete line, wait for more data
                }

                // Extract the line string using string_view or substr without modifying the main buffer
                std::string line = readBuffer.substr(readIndex, crlfPos - readIndex);
                
                // Move the cursor forward past the line and the "\r\n"
                readIndex = crlfPos + 2;

                if (currentState == RESPState::EXPECTING_ARRAY_LEN) {
                    if (!line.empty() && line[0] == '*') {
                        try {
                            expectedElements = std::stoi(line.substr(1));
                            currentQuery.clear();

                            if (expectedElements == 0) {
                                currentState = RESPState::EXPECTING_ARRAY_LEN;
                            } else {
                                currentState = RESPState::EXPECTING_BULK_LEN;
                            }
                        } catch (...) {
                            readBuffer.clear();
                            readIndex = 0;
                            currentState = RESPState::EXPECTING_ARRAY_LEN;
                            return;
                        }
                    }
                } else if (currentState == RESPState::EXPECTING_BULK_LEN) {
                    if (!line.empty() && line[0] == '$') {
                        try {
                            currentBulkLength = std::stoi(line.substr(1));
                            currentState = RESPState::EXPECTING_BULK_DATA;
                        } catch (...) {
                            readBuffer.clear();
                            readIndex = 0;
                            currentState = RESPState::EXPECTING_ARRAY_LEN;
                            return;
                        }
                    }
                }

            } else if (currentState == RESPState::EXPECTING_BULK_DATA) {
                int totalRequiredBytes = currentBulkLength + 2; // Data length + \r\n
                
                // Check if enough bytes are left in the unparsed portion
                if ((readBuffer.length() - readIndex) < static_cast<size_t>(totalRequiredBytes)) {
                    break; // Incomplete bulk data, wait for next network frame
                }

                std::string argument = readBuffer.substr(readIndex, currentBulkLength);
                currentQuery.push_back(argument);
                
                // Advance the cursor past the payload and trailing \r\n
                readIndex += totalRequiredBytes;

                if (currentQuery.size() == expectedElements) {
                    readyQueries.push(currentQuery);
                    currentQuery.clear();

                    currentState = RESPState::EXPECTING_ARRAY_LEN;
                    expectedElements = 0;
                } else {
                    currentState = RESPState::EXPECTING_BULK_LEN;
                }
            }
        }

        // 3. Maintenance: Erase fully consumed chunks to prevent unbounded memory growth
        if (readIndex > 0) {
            readBuffer.erase(0, readIndex);
            readIndex = 0; // Reset cursor back to the new front
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