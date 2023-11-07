#pragma once
#include <print>
#include <functional>
#include <string>
#include <string_view>
#include <winsock2.h>
#include "../defines.h"

auto start_server(u32 port, std::function<std::string(std::string_view)> handler) -> int {
    // Initialize Winsock
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0) {
        std::print(stderr, "WSAStartup failed.");
        return 1;
    }

    // Create a socket for the server
    SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (listenSocket == INVALID_SOCKET) {
        std::print(stderr, "Failed to create socket.\n");
        WSACleanup();
        return 1;
    }

    // Bind the socket to an IP address and port
    sockaddr_in serverAddress;
    serverAddress.sin_family      = AF_INET;
    serverAddress.sin_addr.s_addr = INADDR_ANY; // Use any available network interface
    serverAddress.sin_port        = htons(port);// Port to listen on

    if (bind(listenSocket, (sockaddr *) &serverAddress, sizeof(serverAddress)) == SOCKET_ERROR) {
        std::print(stderr, "Bind failed.\n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    // Start listening for incoming connections
    if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
        std::print(stderr, "Listen failed.\n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::print("Server is listening on port {}...\n", port);

    // Accept an incoming connection
    sockaddr_in clientAddress;
    int clientAddressSize = sizeof(clientAddress);
    SOCKET clientSocket   = accept(listenSocket, (sockaddr *) &clientAddress, &clientAddressSize);
    if (clientSocket == INVALID_SOCKET) {
        std::print(stderr, "Accept failed.\n");
        closesocket(listenSocket);
        WSACleanup();
        return 1;
    }

    std::print("Client connected.\n");

    // Receive and send data
    char buffer[1024];
    int bytesRead;

    //while (true) {
    bytesRead = recv(clientSocket, buffer, sizeof(buffer), 0);
    if (bytesRead <= 0) {
        std::print(stderr, "Connection closed or error in receiving data.\n");
        //break;
    }

    std::string_view request(buffer, bytesRead);
    std::print("Received: {}\n", request);

    auto response = handler(request);

    send(clientSocket, response.c_str(), static_cast<int>(response.size()), 0);
    // }

    // Close the sockets and clean up
    closesocket(clientSocket);
    closesocket(listenSocket);
    WSACleanup();

    return 0;
}