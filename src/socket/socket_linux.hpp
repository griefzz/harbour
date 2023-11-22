#pragma once
#include <functional>
#include <string>
#include <string_view>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include "../config.hpp"
#include "../logger.hpp"
#include "../request.hpp"
#include "../response.hpp"

constexpr int MAX_EVENTS = 10000;

auto start_server(uint32_t port, const std::function<std::string(std::string_view)> &handler) noexcept -> void {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx             = SSL_CTX_new(method);
    if (!ctx) {
        Logger::error("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        return;
    }

    SSL_CTX_set_ecdh_auto(ctx, 1);

    // Set the key and cert
    if (SSL_CTX_use_certificate_file(ctx, ServerCertificatePath.c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return;
    }

    if (SSL_CTX_use_PrivateKey_file(ctx, ServerPrivateKeyPath.c_str(), SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        return;
    }

    // Create a socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        Logger::error("Error creating socket");
        return;
    }

    int reuse = 1;
    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(reuse)) < 0) {
        Logger::error("setsockopt(SO_REUSEADDR) failed");
        close(server_socket);
        return;
    }

    if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, (const char *) &reuse, sizeof(reuse)) < 0) {
        Logger::error("setsockopt(SO_REUSEPORT) failed");
        close(server_socket);
        return;
    }

    int quickack = 1;
    if (setsockopt(server_socket, IPPROTO_TCP, TCP_QUICKACK, (const char *) &quickack, sizeof(quickack)) < 0) {
        Logger::error("setsockopt(TCP_QUICKACK) failed");
        close(server_socket);
        return;
    }

    // Set up the server address structure
    sockaddr_in server_address{};
    server_address.sin_family      = AF_INET;
    server_address.sin_addr.s_addr = INADDR_ANY;
    server_address.sin_port        = htons(port);

    // Bind the socket
    if (bind(server_socket, reinterpret_cast<struct sockaddr *>(&server_address), sizeof(server_address)) == -1) {
        Logger::error("Error binding socket");
        close(server_socket);
        return;
    }

    // Listen for incoming connections
    if (listen(server_socket, SOMAXCONN) == -1) {
        Logger::error("Error listening on socket");
        close(server_socket);
        return;
    }

    // Create an epoll instance
    int epoll_fd = epoll_create1(0);
    if (epoll_fd == -1) {
        Logger::error("Error creating epoll instance");
        close(server_socket);
        return;
    }

    // Add the server socket to the epoll event list
    epoll_event event;
    event.events  = EPOLLIN;
    event.data.fd = server_socket;
    if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1) {
        Logger::error("Error adding server socket to epoll");
        close(server_socket);
        close(epoll_fd);
        return;
    }

    Logger::info(std::format("Listening on {}\n", port));

    // Buffer to store incoming data
    constexpr int BUFFER_SIZE = 1024;
    char buffer[BUFFER_SIZE];

    while (true) {
        // Wait for events using epoll
        epoll_event events[MAX_EVENTS];
        int num_events = epoll_wait(epoll_fd, events, MAX_EVENTS, -1);
        if (num_events == -1) {
            Logger::error("Error in epoll_wait");
            break;
        }

        // Process events
        for (int i = 0; i < num_events; ++i) {
            if (events[i].data.fd == server_socket) {
                // Accept a new connection
                sockaddr_in client_address{};
                socklen_t client_address_len = sizeof(client_address);
                int client_socket            = accept(server_socket, reinterpret_cast<struct sockaddr *>(&client_address), &client_address_len);
                if (client_socket == -1) {
                    Logger::error("Error accepting connection");
                    continue;
                }

                // Add the new client socket to the epoll event list
                event.events  = EPOLLIN;
                event.data.fd = client_socket;
                if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
                    Logger::error("Error adding client socket to epoll");
                    close(client_socket);
                }
            } else {
                // Handle data from a client
                int client_socket = events[i].data.fd;
                SSL *ssl          = SSL_new(ctx);
                SSL_set_fd(ssl, client_socket);

                if (auto code = SSL_accept(ssl); code <= 0) {
                    ERR_print_errors_fp(stderr);
                    // Send a 301 Response if you request http
                    if (SSL_get_error(ssl, code) == SSL_ERROR_SYSCALL) {
                        SSL_shutdown(ssl);
                        SSL_free(ssl);
                        int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
                        if (bytes_received <= 0) {
                            Logger::error("Error in doing a 301 recovery!");
                        } else {
                            std::string msg = "HTTP/1.1 301 Moved Permanently\nLocation: https://127.0.0.1:8080/\nConnection: close\n\n";
                            if (send(client_socket, msg.data(), msg.size(), 0) == -1) {
                                Logger::error("Error sending 301 recovery data to client");
                            }
                        }

                        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, nullptr);
                        close(client_socket);
                        continue;
                    }
                }

                size_t bytes_received = 0;
                if (SSL_read_ex(ssl, buffer, BUFFER_SIZE, &bytes_received) <= 0) {
                    // Connection closed or error
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, nullptr);
                    close(client_socket);
                } else {
                    // Handle the received data (you can customize this part)
                    buffer[bytes_received] = '\0';

                    auto message  = std::string_view(buffer, bytes_received);
                    auto response = handler(message);

                    if (SSL_write(ssl, response.c_str(), response.size()) <= 0) {
                        Logger::error("Error sending data to client");
                    }

                    SSL_shutdown(ssl);
                    SSL_free(ssl);

                    // For simplicity, just close the connection after handling data
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, nullptr);
                    close(client_socket);
                }
            }
        }
    }

    // Cleanup
    SSL_CTX_free(ctx);
    EVP_cleanup();
    close(server_socket);
    close(epoll_fd);
}