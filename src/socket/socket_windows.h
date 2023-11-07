#pragma once
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <winsock2.h>
#include <ws2tcpip.h>
#include <format>
#include <iostream>
#include <functional>
#include <string>
#include <string_view>

auto start_server(uint32_t port, const std::function<std::string(std::string_view)> &handler) -> void {
    SOCKET listenSocket = INVALID_SOCKET;
    SOCKET clientSocket = INVALID_SOCKET;

    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << std::format("WSAStartup failed.");
        return;
    }

    struct addrinfo *result = NULL;
    struct addrinfo hints;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family   = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_protocol = IPPROTO_TCP;
    hints.ai_flags    = AI_PASSIVE;

    // Resolve the server address and port
    if (getaddrinfo(NULL, std::to_string(port).c_str(), &hints, &result) != 0) {
        printf("getaddrinfo failed.");
        WSACleanup();
        return;
    }

    // Create a SOCKET for the server to listen for client connections.
    listenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
    if (listenSocket == INVALID_SOCKET) {
        printf("socket failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        WSACleanup();
        return;
    }


    // Setup the TCP listening socket
    if (bind(listenSocket, result->ai_addr, (int) result->ai_addrlen) == SOCKET_ERROR) {
        printf("bind failed with error: %d\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    freeaddrinfo(result);

    // Start listening for incoming connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << "Listen failed.\n";
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    std::cout << std::format("Server is listening on port {}...\n", port);

    // Accept a client socket
    clientSocket = accept(listenSocket, NULL, NULL);
    if (clientSocket == INVALID_SOCKET) {
        printf("accept failed with error: %d\n", WSAGetLastError());
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    // No longer need server socket
    //closesocket(listenSocket);

    std::cout << "Client connected.\n";

    // Receive and send data
    char buffer[2048];

    while (true) {
        auto bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead == 0) {
            std::cerr << "Connection closing...\n";

            // Accept a new connection
            clientSocket = accept(listenSocket, NULL, NULL);
            if (clientSocket == INVALID_SOCKET) {
                printf("Accept failed with error: %d\n", WSAGetLastError());
                closesocket(listenSocket);
                break;
            }

            std::cout << "Client connected.\n";
            continue;
        }
        if (bytesRead < 0) {
            std::cerr << std::format("recv failed\n");
            break;
        }

        std::string_view request(buffer, bytesRead);
        auto response = handler(request);

        auto re = send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
        if (re == SOCKET_ERROR) {
            std::cerr << std::format("Connection closed or error in sending data.\n");
            break;
        }
    }

    if (shutdown(clientSocket, SD_SEND) == SOCKET_ERROR) {
        printf("Shutdown failed with error: %d\n", WSAGetLastError());
        closesocket(clientSocket);
        WSACleanup();
        return;
    };

    // Close the sockets and clean up
    closesocket(clientSocket);
    WSACleanup();
}