#pragma once
#include <iostream>
#include <iomanip>
#include <ctime>

#ifdef _WIN32
struct tm *gmtime_r(time_t const *timep, struct tm *tmp)
{
    if (gmtime_s(tmp, timep) == 0)
        return tmp;
    return 0;
}
#endif

std::string getCurrentHttpDate()
{
    std::time_t now = std::time(nullptr);

    // Convert to a tm struct in UTC
    std::tm timeInfo;
    gmtime_r(&now, &timeInfo);

    // Format the time in the required HTTP Date format
    char buffer[30];
    std::strftime(buffer, sizeof(buffer), "%a, %d %b %Y %H:%M:%S GMT", &timeInfo);

    return buffer;
}