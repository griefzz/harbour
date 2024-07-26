///  _  _             _
/// | || | __ _  _ _ | |__  ___  _  _  _ _   A Web Server Framework For Modern C++
/// | __ |/ _` || '_|| '_ \/ _ \| || || '_|  https://github.com/griefzz/harbour
/// |_||_|\__,_||_|  |_.__/\___/ \_,_||_|    License: MIT
///

#include <harbour/harbour.hpp>

#include "args.hpp"
#include "ships.hpp"

using namespace harbour;

int main(int argc, char **argv) {
    auto settings = server::Settings::defaults();

    auto args = Args::create(argc, argv)
                        .var("port", "Port to use for connections")
                        .var("cert", "Certificate path for SSL in PEM format")
                        .var("key", "Private key path for SSL in PEM format")
                        .flag("ssl", "Enable SSL")
                        .flag("help", "Display program usage");

    if (auto help = args.get<bool>("help")) {
        args.print();
        return 0;
    }

    if (auto port = args.get<server::port_type>("port")) {
        settings.port = *port;
    }

    if (auto ssl = args.get<std::size_t>("ssl")) {
        auto cert = args.get<std::string>("cert");
        if (!cert.has_value()) {
            log::critical("You need to specify a cert path when ssl is enabled");
            args.print();
            return 0;
        }

        auto key = args.get<std::string>("key");
        if (!key.has_value()) {
            log::critical("You need to specify a key path when ssl is enabled");
            args.print();
            return 0;
        }

        settings.with_ssl_paths(*cert, *key);

    } else {
        Harbour hb(settings);
        hb.dock(Ship);
        hb.sail();
    }

    return 0;
}