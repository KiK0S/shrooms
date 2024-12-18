#pragma once
#include <iostream>
#include <string>

#define LOG_IF(flag, message) \
    if (flag) std::cout << message << std::endl
