///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///

#include <harbour/harbour.hpp>
#include <memory>

extern "C" int LLVMFuzzerTestOneInput(const uint8_t *data, size_t size) {
    std::shared_ptr<harbour::server::Socket> sock;
    auto fuzz_data = reinterpret_cast<const char *>(data);
    auto req = harbour::Request::create(sock, fuzz_data, size);
    return 0;
}