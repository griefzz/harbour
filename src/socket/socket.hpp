#pragma once
#ifdef _WIN32
    #include "socket_windows.hpp"
#elif __linux__
    #include "socket_linux.hpp"
#elif defined(__APPLE__)
    #include "socket_darwin.hpp"
#else
    #error Unsupported OS Architecture
#endif
