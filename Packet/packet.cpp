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

    void InPacket::appendData(const char* data, int length) {
        // 1. Append new bytes to the end of the buffer
        readBuffer.append(data, length);

        while (readIndex < readBuffer.length()) {
            
            if (currentState == RESPState::EXPECTING_ARRAY_LEN || 
                currentState == RESPState::EXPECTING_BULK_LEN) {
                
                // Search only in the unparsed portion of the buffer
                size_t crlfPos = readBuffer.find("\r\n", readIndex);
                if (crlfPos == std::string::npos) {
                    break; // Incomplete data
                }

                std::string_view line = readBuffer.substr(readIndex, crlfPos - readIndex);
                readIndex = crlfPos + 2;

                if (currentState == RESPState::EXPECTING_ARRAY_LEN) {
                    if (!line.empty() && line[0] == '*') {
                        auto [ptr, ec] = std::from_chars(line.data() + 1, line.data() + line.size(), expectedElements);
                        
                        if (ec != std::errc() || expectedElements < 0) {
                            throw ProtocolException("Invalid array length");
                        }

                        currentQuery.clear();
                        if (expectedElements == 0) {
                            currentState = RESPState::EXPECTING_ARRAY_LEN;
                        } else {
                            currentState = RESPState::EXPECTING_BULK_LEN;
                        }
                    } else {
                        throw ProtocolException("Expected '*' for array length");
                    }
                } else if (currentState == RESPState::EXPECTING_BULK_LEN) {
                    if (!line.empty() && line[0] == '$') {
                        auto [ptr, ec] = std::from_chars(line.data() + 1, line.data() + line.size(), currentBulkLength);
                        
                        if (ec != std::errc() || currentBulkLength < 0) {
                            throw ProtocolException("Invalid bulk string length");
                        }
                        currentState = RESPState::EXPECTING_BULK_DATA;
                    } else {
                        throw ProtocolException("Expected '$' for bulk string length");
                    }
                }

            } else if (currentState == RESPState::EXPECTING_BULK_DATA) {
                int totalRequiredBytes = currentBulkLength + 2; // Data length + \r\n
                
                if ((readBuffer.length() - readIndex) < static_cast<size_t>(totalRequiredBytes)) {
                    break; // Incomplete bulk data, wait for next network frame
                }

                std::string argument = readBuffer.substr(readIndex, currentBulkLength);
                currentQuery.push_back(argument);
                
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

        if (readIndex > 0) {
            if (readIndex == readBuffer.length()) {
                readBuffer.clear();
                readIndex = 0;
            } else if (readIndex > 16384) { 
                // Threshold hit (16KB). 
                readBuffer.erase(0, readIndex);
                readIndex = 0; 
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