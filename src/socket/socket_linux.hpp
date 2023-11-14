#pragma once
#include <functional>
#include <string>
#include <string_view>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/epoll.h>

#include "../logger.h"

constexpr int MAX_EVENTS = 10000;

auto start_server(uint32_t port, const std::function<std::string(std::string_view)> &handler) noexcept -> void {
    // Create a socket
    int server_socket = socket(AF_INET, SOCK_STREAM, 0);
    if (server_socket == -1) {
        Logger::error("Error creating socket");
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
                int client_socket  = events[i].data.fd;
                int bytes_received = recv(client_socket, buffer, BUFFER_SIZE, 0);
                if (bytes_received <= 0) {
                    // Connection closed or error
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, nullptr);
                    close(client_socket);
                } else {
                    // Handle the received data (you can customize this part)
                    buffer[bytes_received] = '\0';

                    auto message  = std::string_view(buffer, bytes_received);
                    auto response = handler(message);

                    if (send(client_socket, response.c_str(), response.size(), 0) == -1) {
                        Logger::error("Error sending data to client");
                    }

                    // For simplicity, just close the connection after handling data
                    epoll_ctl(epoll_fd, EPOLL_CTL_DEL, client_socket, nullptr);
                    close(client_socket);
                }
            }
        }
    }

    // Cleanup
    close(server_socket);
    close(epoll_fd);
}