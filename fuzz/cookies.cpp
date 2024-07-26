///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///

#include <string>

#include <harbour/harbour.hpp>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    auto fuzz_data = reinterpret_cast<const char *>(data);
    std::string cookie(fuzz_data, fuzz_data + size);
    auto result = harbour::cookies::detail::parse(cookie);
    return 0;
}