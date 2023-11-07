#pragma once
#include <format>
#include <iostream>
#include <functional>
#include <string>
#include <string_view>
#include <winsock2.h>

auto start_server(uint32_t port, const std::function<std::string(std::string_view)> &handler) -> void {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::cerr << std::format("WSAStartup failed.");
        return;
    }

    // Create a socket for the server
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        std::cerr << std::format("Failed to create socket.\n");
        WSACleanup();
        return;
    }

    // Bind the socket to an IP address and port
    sockaddr_in serverAddress;
    serverAddress.sin_family      = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Use any available network interface
    serverAddress.sin_port        = htons(port);// Port to listen on

    if (bind(listenSocket, (sockaddr *) &serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::cerr << std::format("Bind failed.\n");
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    // Start listening for incoming connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::cerr << std::format("Listen failed.\n");
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    std::cout << std::format("Server is listening on port {}...\n", port);

    // Accept an incoming connection
    sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    SOCKET clientSocket   = accept(listenSocket, (sockaddr *) &clientAddress, &clientAddressSize);
    if (clientSocket == INVALID_SOCKET) {
        std::cerr << std::format("Accept failed.\n");
        closesocket(listenSocket);
        WSACleanup();
        return;
    }

    std::cout << "Client connected.\n";

    // Receive and send data
    char buffer[2048];

    while (true) {
        auto bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
        if (bytesRead == 0) {
            std::cerr << "Connection closing...\n";
            break;
        }
        if (bytesRead < 0) {
            std::cerr << std::format("recv failed\n");
            break;
        }

        std::string_view request(buffer, bytesRead);
        std::cout << std::format("Received: {}\n", request);

        auto response = handler(request);

        auto re = send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
        if (re == SOCKET_ERROR) {
            std::cerr << std::format("Connection closed or error in sending data.\n");
            break;
        }

        std::cout << std::format("got: {}\n", re);
    }


    // Close the sockets and clean up
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();
}