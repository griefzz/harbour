#pragma once
#include <iostream>
#include <cstring>
#include <functional>
#include <string>

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

using ConnectionHandler = std::function<std::string(std::string_view)>;

// Max amount of epoll events to que up
constexpr int MAX_EVENTS = 10000;

/// Decode the ssl error and return some status codes for handling it
auto SSL_status(SSL *ssl, int status) -> int {
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

/// Initialize our SSL context with our cert and private key
auto init_ssl() -> SSL_CTX * {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    auto ctx = SSL_CTX_new(TLS_server_method());
    if (!ctx) {
        Logger::error("Failed to initialize SSL Context");
        return nullptr;
    }

    SSL_CTX_set_ecdh_auto(ctx, 1);
    SSL_CTX_set_mode(ctx, SSL_MODE_AUTO_RETRY);

    // Set the key and cert
    if (SSL_CTX_use_certificate_file(ctx, ServerCertificatePath().c_str(), SSL_FILETYPE_PEM) <= 0) {
        Logger::error("Unable to load SSL certificate");
        return nullptr;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, ServerPrivateKeyPath().c_str(), SSL_FILETYPE_PEM) <= 0) {
        Logger::error("Unable to load SSL private key");
        return nullptr;
    }

    return ctx;
}

/// Set a socket to be non-blocking, returns true on success
auto setnonblocking(int fd) -> bool {
    if (fd < 0) return false;
    int flags = fcntl(fd, F_GETFL, 0);
    if (flags == -1) return false;
    flags = (flags | O_NONBLOCK);
    return (fcntl(fd, F_SETFL, flags) == 0) ? true : false;
}

/// Create and configure a socket on the specified port
auto create_socket(uint32_t port) -> std::optional<int> {
    // Create a socket
    int fd = socket(AF_INET, SOCK_STREAM, 0);
    if (fd == -1) {
        perror("socket");
        Logger::error("Socket creation failed");
        return {};
    }

    int optval = 1;
    if (setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        Logger::error("setsockopt(SO_REUSEADDR) failed");
        return {};
    }

    if (setsockopt(fd, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        Logger::error("setsockopt(SO_REUSEPORT) failed");
        return {};
    }

    if (setsockopt(fd, IPPROTO_TCP, TCP_QUICKACK, &optval, sizeof(optval)) < 0) {
        perror("setsockopt");
        Logger::error("setsockopt(TCP_QUICKACK) failed");
        return {};
    }

    // Set up the server address structure
    sockaddr_in server_address{};
    server_address.sin_family      = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port        = htons(port);

    // Bind the socket
    if (bind(fd, reinterpret_cast<struct sockaddr *>(&server_address), sizeof(server_address)) == -1) {
        perror("bind");
        Logger::error("Error binding socket");
        close(fd);
        return {};
    }

    // Listen for connections
    if (listen(fd, SOMAXCONN) < 0) {
        perror("listen");
        Logger::error("Error listening on socket");
        close(fd);
        return {};
    }

    return fd;
}

// Function to call whenever we want to send/recieve data from a client
auto handle_client(epoll_event *event, ConnectionHandler handler) -> void {
    SSL *ssl              = (SSL *) event->data.ptr;
    const int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];
    size_t bytes_read{};
    int status{};

    // Clear the buffer
    std::memset(buffer, 0, BUFFER_SIZE);

    // Read data from the socket
    do {
        status = SSL_read_ex(ssl, buffer, BUFFER_SIZE - 1, &bytes_read);
        status = SSL_status(ssl, status);
    } while (status == 1);

    if (bytes_read > 0) {
        const std::string in(buffer, bytes_read);
        auto out = handler(in);
        do {
            status = SSL_write(ssl, out.data(), out.size());
            status = SSL_status(ssl, status);
        } while (status == 1);
    } else if (bytes_read == 0) {
        // Connection closed by the client
        //std::cout << "Client disconnected." << std::endl;
    } else {
        // An error occurred
        Logger::error("Failed to read from the socket.");
    }

    SSL_shutdown(ssl);
}

// Initialize a TLS capable server on port, passing all data to handler and returning its result to the client
static auto start_server(uint32_t port, ConnectionHandler handler) noexcept -> void {
    auto ctx = init_ssl();
    if (!ctx) {
        Logger::error("Failed to init SSL");
        exit(EXIT_FAILURE);
    }

    int listen_sock{};
    if (auto result = create_socket(port); result.has_value()) {
        listen_sock = *result;
    } else {
        Logger::error("Failed to create socket");
        exit(EXIT_FAILURE);
    }

    struct epoll_event ev, events[MAX_EVENTS];
    int epollfd = epoll_create1(0);
    if (epollfd == -1) {
        perror("epoll_create1");
        Logger::error("epoll_create failed");
        exit(EXIT_FAILURE);
    }

    ev.events  = EPOLLIN;
    ev.data.fd = listen_sock;
    if (epoll_ctl(epollfd, EPOLL_CTL_ADD, listen_sock, &ev) == -1) {
        perror("epoll_ctl: listen_sock");
        Logger::error("epoll_ctl failed");
        exit(EXIT_FAILURE);
    }

    for (;;) {
        int nfds = epoll_wait(epollfd, events, MAX_EVENTS, -1);
        if (nfds == -1) {
            perror("epoll_wait");
            Logger::error("epoll_wait failed");
            exit(EXIT_FAILURE);
        }

        for (int n = 0; n < nfds; ++n) {
            if (events[n].data.fd == listen_sock) {
                sockaddr_in client_address{};
                socklen_t client_address_len = sizeof(client_address);
                int conn                     = accept(listen_sock, reinterpret_cast<struct sockaddr *>(&client_address), &client_address_len);
                if (conn == -1) {
                    perror("accept");
                    break;
                }

                if (!setnonblocking(conn)) {
                    perror("setnonblocking");
                    Logger::error("Failed to set fd as non-blocking");
                    close(conn);
                    break;
                }

                auto ssl = SSL_new(ctx);
                SSL_set_fd(ssl, conn);
                int status;
                bool redirected = false;
                do {
                    status = SSL_accept(ssl);
                    status = SSL_status(ssl, status);
                    if (status == -1) {
                        std::string msg = "HTTP/1.1 301 Moved Permanently\nLocation: https://127.0.0.1:8080/\nConnection: close\n\n";
                        if (send(conn, msg.data(), msg.size(), MSG_NOSIGNAL) == -1) {
                            perror("send");
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
                if (epoll_ctl(epollfd, EPOLL_CTL_ADD, conn, &ev) == -1) {
                    perror("epoll_ctl: conn");
                    Logger::error("epoll_ctl failed");
                    exit(EXIT_FAILURE);
                }
            } else {
                handle_client(&events[n], handler);
            }
        }
    }

    close(listen_sock);
    close(epollfd);
    SSL_CTX_free(ctx);
    EVP_cleanup();
}