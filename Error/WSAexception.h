#pragma once
#include <system_error>

void throwWSAError(const std::string& context);

const std::string getWSAMessage(int errorCode);