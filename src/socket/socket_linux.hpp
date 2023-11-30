#pragma once
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <cstring>
#include <signal.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netinet/tcp.h>
#include <openssl/ssl.h>
#include <openssl/err.h>

#include <harbour/config.hpp>
#include <harbour/logger.hpp>

using ConnectionHandler = std::function<std::string(std::string_view)>;

// Max amount of epoll events to que up
constexpr int MAX_EVENTS = 10000;

// Convenience wrapper for calling custom destructors
template<typename T>
using deleted_unique_ptr = std::unique_ptr<T, std::function<void(T *)>>;

static void ssl_deleter(SSL *ssl) {
    SSL_shutdown(ssl);
    SSL_free(ssl);
}

// RAII wrapper for a single client connection storing its socket and epoll event
struct HarbourClient {
    int socket;
    int epoll_fd;

    explicit HarbourClient(int socket, int epoll_fd) : socket(socket), epoll_fd(epoll_fd) {}

    ~HarbourClient() {
        close(socket);
        epoll_ctl(epoll_fd, EPOLL_CTL_DEL, socket, nullptr);
    }
};

// Redirect clients to HTTPS
static void handle_redirect(HarbourClient *client) {
    std::string msg = "HTTP/1.1 301 Moved Permanently\nLocation: https://127.0.0.1:8080/\nConnection: close\n\n";
    if (send(client->socket, msg.data(), msg.size(), 0) == -1) {
        Logger::error("Error sending 301 recovery data to client");
    }
}

struct EPoller {
    /// @brief ssl -> socket -> opt -> bind -> listen -> epoll
    EPoller(uint32_t port, ConnectionHandler handler) : port(port), handler(std::move(handler)) {
        // Prevent SIGPIPE from crashing the server
        struct sigaction sig_in = {SIG_IGN};
        sigaction(SIGPIPE, &sig_in, nullptr);

        SSL_load_error_strings();
        OpenSSL_add_ssl_algorithms();
        ctx = SSL_CTX_new(TLS_server_method());
        if (!ctx) {
            Logger::error("Unable to create SSL context");
            ERR_print_errors_fp(stderr);
            return;
        }

        SSL_CTX_set_ecdh_auto(ctx, 1);

        // Set the key and cert
        if (SSL_CTX_use_certificate_file(ctx, ServerCertificatePath().c_str(), SSL_FILETYPE_PEM) <= 0) {
            Logger::error("Unable to load SSL certificate");
            ERR_print_errors_fp(stderr);
            return;
        }

        if (SSL_CTX_use_PrivateKey_file(ctx, ServerPrivateKeyPath().c_str(), SSL_FILETYPE_PEM) <= 0) {
            Logger::error("Unable to load SSL private key");
            ERR_print_errors_fp(stderr);
            return;
        }

        // Create a socket
        server_socket = socket(AF_INET, SOCK_STREAM, 0);
        if (server_socket == -1) {
            Logger::error("Error creating socket");
            return;
        }

        // Allow for reusing addresses/port (lets us rapidly restart server) and enable quickack for speed (needs testing)
        int reuse = 1;
        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEADDR, (const char *) &reuse, sizeof(reuse)) < 0) {
            Logger::error("setsockopt(SO_REUSEADDR) failed");
            return;
        }

        if (setsockopt(server_socket, SOL_SOCKET, SO_REUSEPORT, (const char *) &reuse, sizeof(reuse)) < 0) {
            Logger::error("setsockopt(SO_REUSEPORT) failed");
            return;
        }

        int quickack = 1;
        if (setsockopt(server_socket, IPPROTO_TCP, TCP_QUICKACK, (const char *) &quickack, sizeof(quickack)) < 0) {
            Logger::error("setsockopt(TCP_QUICKACK) failed");
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
            return;
        }

        // Listen for incoming connections
        if (listen(server_socket, SOMAXCONN) == -1) {
            Logger::error("Error listening on socket");
            return;
        }

        // Create an epoll instance
        epoll_fd = epoll_create1(0);
        if (epoll_fd == -1) {
            Logger::error("Error creating epoll instance");
            return;
        }

        // Add the server socket to the epoll event list
        epoll_event event;
        event.events  = EPOLLIN;
        event.data.fd = server_socket;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_socket, &event) == -1) {
            Logger::error("Error adding server socket to epoll");
            return;
        }

        Logger::info(std::format("Listening on {}", port));
    }

    /// @brief cleanup SSL and epoll
    ~EPoller() {
        close(server_socket);
        close(epoll_fd);
        SSL_CTX_free(ctx);
        EVP_cleanup();
    }

    /// @brief Main epoll event loop
    auto run() noexcept -> void {
        for (;;) {
            // Wait for events using epoll
            std::array<epoll_event, MAX_EVENTS> events;
            int num_events = epoll_wait(epoll_fd, events.data(), MAX_EVENTS, -1);
            if (num_events == -1) {
                Logger::error("Error in epoll_wait");
                return;
            }

            // Process events
            for (int i = 0; i < num_events; i++) {
                if (events[i].data.fd == server_socket) {
                    on_accept();
                } else {
                    on_message(events[i].data.fd);
                }
            }
        }
    }

    auto on_accept() const noexcept -> void {
        // Accept a new connection
        sockaddr_in client_address{};
        socklen_t client_address_len = sizeof(client_address);
        int client_socket            = accept(server_socket, reinterpret_cast<struct sockaddr *>(&client_address), &client_address_len);
        if (client_socket == -1) {
            Logger::error("Error accepting connection");
            return;
        }

        // Add the new client socket to the epoll event list
        epoll_event event;
        event.events  = EPOLLIN;
        event.data.fd = client_socket;
        if (epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_socket, &event) == -1) {
            Logger::error("Error adding client socket to epoll");
            close(client_socket);
        }
    }

    auto on_message(int event_fd) noexcept -> void {
        // Handle data from a client
        auto client = std::make_unique<HarbourClient>(event_fd, epoll_fd);
        deleted_unique_ptr<SSL> ssl(SSL_new(ctx), ssl_deleter);
        SSL_set_fd(ssl.get(), client->socket);

        // Client requested HTTP instead of HTTPS so redirect him to the HTTPS domain
        if (auto code = SSL_accept(ssl.get()); code <= 0) {
            ERR_print_errors_fp(stderr);
            // SSL_ERROR_SYSCALL looked to be the error i got when requesting HTTP...
            // needs more testing to be really stable
            if (SSL_get_error(ssl.get(), code) == SSL_ERROR_SYSCALL) {
                handle_redirect(client.get());
                return;
            }
        }

        // Recieve data from the client and send the handlers response
        size_t bytes_received = 0;
        if (SSL_read_ex(ssl.get(), buffer.data(), BUFFER_SIZE, &bytes_received) > 0) {
            buffer[bytes_received] = '\0';

            auto message  = std::string_view(buffer.data(), bytes_received);
            auto response = handler(message);

            if (SSL_write(ssl.get(), response.c_str(), static_cast<int>(response.size())) <= 0) {
                Logger::error("Error sending data to client");
            }
        }
    }

    // function to call on data recieved from client
    ConnectionHandler handler;

    // SSL context
    SSL_CTX *ctx;

    // file descripter for socket
    int server_socket;

    // file descriptor for epoll
    int epoll_fd;

    // port for server to listen on
    uint32_t port;

    // Buffer to store incoming data
    const int BUFFER_SIZE = 1024;
    std::array<char, 1024> buffer;
};

// Initialize a TLS capable server on port, passing all data to handler and returning its result to the client
static auto start_server(uint32_t port, ConnectionHandler handler) noexcept -> void {
    EPoller epoller(port, std::move(handler));
    epoller.run();
}