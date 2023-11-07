#include <iostream>
#include "cache.h"// initialize the cache at startup
#include "server.h"

auto main() -> int {
    Server server(8080);
    server.serve();

    return 0;
}
