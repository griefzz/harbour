#pragma once
#include <iostream>
#include <cstring>
#include <functional>
#include <string>
#include <array>

#include <sys/socket.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <unistd.h>
#include <fcntl.h>

#include <openssl/ssl.h>
#include <openssl/err.h>

#include <harbour/config.hpp>
#include <harbour/logger.hpp>

// check for an error and report a msg then exit failure
#define check_error_fatal(cond, msg) \
    do {                             \
        if ((cond)) {                \
            Logger::error((msg));    \
            exit(EXIT_FAILURE);      \
        }                            \
    } while (0);

// check for an error and report a msg then return a value
#define check_error_ret(cond, msg, ret) \
    do {                                \
        if ((cond)) {                   \
            Logger::error((msg));       \
            return (ret);               \
        }                               \
    } while (0);

using ConnectionHandler = std::function<std::string(std::string_view)>;

// Max amount of epoll events to que up
constexpr int MAX_EVENTS = 10000;

// Decode the ssl error and return some status codes for handling it
auto SSL_status(SSL *ssl, int status) noexcept -> int {
    auto err = SSL_get_error(ssl, status);
    switch (err) {
        case SSL_ERROR_NONE:
            return 0;// Success

        case SSL_ERROR_WANT_WRITE:
            return 1;// Wait

        case SSL_ERROR_WANT_READ:
            return 1;// Wait

        case SSL_ERROR_ZERO_RETURN:
            return -1;// Shutdown

        case SSL_ERROR_SYSCALL:
            return -1;// Shutdown

        case SSL_ERROR_SSL:
            return -1;// Redirect to https

        default:
            return -1;
    }
}

// Initialize our SSL context with our cert and private key
auto SSL_init() noexcept -> SSL_CTX * {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();

    auto ctx = SSL_CTX_new(TLS_server_method());
    check_error_ret(!ctx, "Failed to initialize SSL Context", nullptr);

    SSL_CTX_set_ecdh_auto(ctx, 1);
    SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

    // Set the key and cert
    check_error_ret(SSL_CTX_use_certificate_file(ctx, ServerCertificatePath().c_str(), SSL_FILETYPE_PEM) <= 0,
                    "Unable to load SSL certificate",
                    nullptr);

    check_error_ret(SSL_CTX_use_PrivateKey_file(ctx, ServerPrivateKeyPath().c_str(), SSL_FILETYPE_PEM) <= 0,
                    "Unable to load SSL private key",
                    nullptr);
    return ctx;
}

// Set a socket to be non-blocking, returns true on success
auto setnonblocking(int fd) noexcept -> bool {
    check_error_ret(fd < 0, "fd < 0", false);

    int flags = fcntl(fd, F_GETFL, 0);
    check_error_ret(flags == -1, "fnctl failed", false);

    flags = fcntl(fd, F_SETFL, flags | O_NONBLOCK);
    check_error_ret(flags == -1, "fnctl failed", false);

    return true;
}

// Create and configure a socket on the specified port
auto create_socket(uint32_t port) noexcept -> int {
    // Create a socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    check_error_ret(fd == -1, "Socket creation failed", -1);

    int optval = 1;
    check_error_ret(setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0,
                    "setsockopt(SO_REUSEADDR) failed",
                    -1);

    check_error_ret(setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0,
                    "setsockopt(SO_REUSEPORT) failed",
                    -1);

    check_error_ret(setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &optval, sizeof(optval)) < 0,
                    "setsockopt(TCP_QUICKACK) failed",
                    -1);

    // Set up the server address structure
    sockaddr_in server_address{};
    server_address.sin_family      = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port        = htons(port);

    // Bind the socket
    check_error_ret(bind(fd, reinterpret_cast<struct sockaddr *>(&server_address), sizeof(server_address)) == -1,
                    "Error binding socket",
                    -1);

    // Listen for connections
    check_error_ret(listen(fd, SOMAXCONN) < 0, "Error listening on socket", -1);

    return fd;
}

// Function to call whenever we want to send/recieve data from a client
auto handle_client(epoll_event *event, ConnectionHandler handler) noexcept -> void {
    constexpr auto BUFFER_SIZE = 1024;
    std::array<char, BUFFER_SIZE> buffer{};

    // Read data from the socket
    size_t bytes_read{};
    int status{};
    SSL *ssl = (SSL *) event->data.ptr;
    do {
        status = SSL_read_ex(ssl, buffer.data(), buffer.size() - 1, &bytes_read);
        status = SSL_status(ssl, status);
    } while (status == 1);

    if (bytes_read > 0) {
        const std::string request(buffer.data(), bytes_read);
        auto response = handler(request);
        do {
            status = SSL_write(ssl, response.data(), response.size());
            status = SSL_status(ssl, status);
        } while (status == 1);
    } else if (bytes_read == 0) {
        // Connection closed by the client
        // Logger::info("Connection closed");
    } else {
        // An error occurred
        Logger::error("Failed to read from the socket.");
    }

    SSL_shutdown(ssl);
}

// Initialize a TLS capable server on port, passing all data to handler and returning its result to the client
static auto start_server(uint32_t port, ConnectionHandler handler) noexcept -> void {
    // Initialize SSL
    auto ctx = SSL_init();
    check_error_fatal(!ctx, "Failed to init SSL");

    // Create a socket
    int serverfd{};
    check_error_fatal((serverfd = create_socket(port)) == -1, "Failed to create socket");

    // Initialze epoll
    struct epoll_event ev, events[MAX_EVENTS];
    int epollfd = epoll_create1(0);
    check_error_fatal(epollfd == -1, "epoll_create failed");

    // Configure epoll
    ev.events  = EPOLLIN;
    ev.data.fd = serverfd;
    check_error_fatal(epoll_ctl(epollfd, EPOLL_CTL_ADD, serverfd, &ev) == -1, "epoll_ctl failed");

    // Main event loop
    for (;;) {
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        check_error_fatal(nfds == -1, "epoll_wait failed");

        for (std::size_t n = 0; n < nfds; ++n) {
            // New client connection
            if (events[n].data.fd == serverfd) {
                sockaddr_in client_address{};
                socklen_t client_address_len = sizeof(client_address);
                int conn                     = accept(serverfd, reinterpret_cast<struct sockaddr *>(&client_address), &client_address_len);
                if (conn == -1) {
                    Logger::error("Failed to accept");
                    break;
                }

                if (!setnonblocking(conn)) {
                    Logger::error("Failed to set fd as non-blocking");
                    close(conn);
                    break;
                }

                auto ssl = SSL_new(ctx);
                SSL_set_fd(ssl, conn);
                int status{};
                bool redirected = false;
                do {
                    status = SSL_accept(ssl);
                    status = SSL_status(ssl, status);

                    // User requested HTTP, redirect to HTTPS
                    if (status == -1) {
                        std::string msg = "HTTP/1.1 301 Moved Permanently\nLocation: https://127.0.0.1:8080/\nConnection: close\n\n";
                        if (send(conn, msg.data(), msg.size(), MSG_NOSIGNAL) == -1) {
                            Logger::error("Failed to write to socket");
                        }
                        close(conn);
                        redirected = true;
                    }
                } while (status == 1 && !SSL_is_init_finished(ssl));

                if (redirected) break;

                ev.events   = EPOLLIN | EPOLLET;
                ev.data.fd  = conn;
                ev.data.ptr = ssl;
                check_error_fatal(epoll_ctl(epollfd, EPOLL_CTL_ADD, conn, &ev) == -1, "epoll_ctl failed");
            } else {
                handle_client(&events[n], handler);
            }
        }
    }

    close(serverfd);
    close(epollfd);
    SSL_CTX_free(ctx);
    EVP_cleanup();
}