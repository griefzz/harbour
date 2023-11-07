#pragma once
#ifdef _WIN32
    #include "socket_windows.h"
#elif __linux__
    #include "socket_linux.h"
#elif defined(__APPLE__)
    #include "socket_mac.h"
#else
    #error Unsupported OS Architecture
#endif