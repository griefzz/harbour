#pragma once
#define WIN32_LEAN_AND_MEAN
#define _WINSOCK_DEPRECATED_NO_WARNINGS
#include <windows.h>
#include <winsock2.h>
#include <mswsock.h>
#include <ws2tcpip.h>

#include <format>
#include <iostream>
#include <functional>
#include <string>
#include <string_view>
#include <format>
#include <memory>
#include "../logger.h"

#pragma comment(lib, "Ws2_32.lib")
#pragma comment(lib, "Mswsock.lib")

#define CON_BUFFSIZE 1024
#define DATA_BUFSIZE 128000

struct sender_reciever {
    enum class State {
        Accept,
        Read,
        Write
    };

    typedef struct _PER_IO_DATA {
        SOCKET socket;
        OVERLAPPED overlapped;
        char data_buff[DATA_BUFSIZE];
        unsigned int buff_len;
        State type;
        char *sdata;
        unsigned int bytes_read;
    } PER_IO_DATA, *LPPER_IO_DATA;

    typedef struct _SOCK_DATA {
        SOCKET socket;
        OVERLAPPED overlapped;
        char addr_buff[CON_BUFFSIZE];
        State type;
    } SOCK_DATA, *LPSOCK_DATA;

    sender_reciever(uint32_t port, const std::function<std::string(std::string_view)> &handler) : port(port), handler(handler) {
        //Listening socket
        SOCKET listener;

        //Windows socket startup
        WSADATA wsaData;
        if (WSAStartup(MAKEWORD(2, 0), &wsaData) != 0) {
            Logger::error("WSAStartup failed.\n");
            exit(1);
        }

        //Create a completion port
        HANDLE comp_port = CreateIoCompletionPort(INVALID_HANDLE_VALUE, NULL, 0, 0);

        //Create listening socket and
        //put it in overlapped mode - WSA_FLAG_OVERLAPPED
        listener = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        if (listener == SOCKET_ERROR) {
            Logger::error(std::format("Socket creation failed: {}\n", WSAGetLastError()));
            exit(1);
        }

        //Addr of listening socket
        struct sockaddr_in addr;
        addr.sin_family      = AF_INET;
        addr.sin_addr.s_addr = htonl(INADDR_ANY);
        addr.sin_port        = htons(port);

        // Setup the TCP listening socket
        if (bind(listener, (sockaddr *) &addr, sizeof(addr)) == SOCKET_ERROR) {
            Logger::error(std::format("bind failed with error: : {}\n", WSAGetLastError()));
            exit(1);
        }

        // Start listening for incoming connections
        if (listen(listener, SOMAXCONN) == SOCKET_ERROR) {
            Logger::error(std::format("Listen failedwith error: {}\n", WSAGetLastError()));
            exit(1);
        }

        //Completion port for newly accepted sockets
        //Link it to the main completion port
        HANDLE sock_port = CreateIoCompletionPort((HANDLE) listener, comp_port, 0, 0);

        Logger::info(std::format("Listening on {}\n", port));

        accept_con(listener);
        while (1) {
            unsigned int bytes_read;
            uintptr_t comp_key;
            OVERLAPPED *ovl = NULL;

            SOCK_DATA *sock_data;
            PER_IO_DATA *per_io_data = NULL;
            State type;
            BOOL res;

            res = GetQueuedCompletionStatus(comp_port, (LPDWORD) &bytes_read, (PULONG_PTR) &comp_key, &ovl, INFINITE);
            if (!res) {
                continue;
            }

            if (comp_key == 0) {
                sock_data = (SOCK_DATA *) CONTAINING_RECORD(ovl, SOCK_DATA, overlapped);
                type      = sock_data->type;
            } else {
                sock_data   = (SOCK_DATA *) comp_key;
                per_io_data = (PER_IO_DATA *) CONTAINING_RECORD(ovl, PER_IO_DATA, overlapped);
                type        = per_io_data->type;
            }

            if (bytes_read == 0 && (type == State::Read || type == State::Write)) {
                //Logger::info("Accepted and closed");
                closesocket(sock_data->socket);
                free(sock_data);
                free(per_io_data);
                continue;
            }

            if (type == State::Accept) {
                //Logger::info(std::format("New con: {}\n", sock_data->socket));
                accept_con(listener);
                HANDLE read_port = CreateIoCompletionPort((HANDLE) sock_data->socket, comp_port, (ULONG_PTR) sock_data, 0);
                rec_data(sock_data);
                continue;
            }

            if (type == State::Read) {
                //Logger::info(std::format("Received data to {}\n", (int) sock_data->socket));

                per_io_data->bytes_read = bytes_read;
                char *sdata             = (char *) malloc(sizeof(char) * bytes_read + 1);
                memcpy_s((void *) sdata, (sizeof(char) * bytes_read + 1), per_io_data->data_buff, bytes_read);

                sdata[bytes_read] = '\0';

                per_io_data->sdata = sdata;

                send_data(sock_data, per_io_data);
                continue;
            }

            if (type == State::Write) {
                if (per_io_data->sdata) {
                    free(per_io_data->sdata);
                }
                free(per_io_data);
                closesocket(sock_data->socket);
                free(sock_data);
                continue;
            }
        }
    }

    auto accept_con(SOCKET listener) noexcept -> int {
        SOCKET accept        = WSASocket(AF_INET, SOCK_STREAM, 0, NULL, 0, WSA_FLAG_OVERLAPPED);
        SOCK_DATA *sock_data = (SOCK_DATA *) malloc(sizeof(SOCK_DATA));
        sock_data->socket    = accept;
        unsigned int bytes_read;
        ZeroMemory(&(sock_data->overlapped), sizeof(OVERLAPPED));
        sock_data->type = State::Accept;

        if (!AcceptEx(listener, accept, &(sock_data->addr_buff), 0, sizeof(SOCKADDR_IN) + 16, sizeof(SOCKADDR_IN) + 16, (LPDWORD) &bytes_read, &(sock_data->overlapped))) {
            int a = WSAGetLastError();
            if (WSAGetLastError() != ERROR_IO_PENDING) {
                closesocket(accept);
                printf("Accept filed with code %d\n", WSAGetLastError());
                free(sock_data);
                return 0;
            }
        }
        return 1;
    }

    auto rec_data(SOCK_DATA *sock_data) noexcept -> int {
        PER_IO_DATA *per_io_data = (PER_IO_DATA *) malloc(sizeof(PER_IO_DATA));
        per_io_data->socket      = sock_data->socket;
        ZeroMemory(&(per_io_data->overlapped), sizeof(OVERLAPPED));
        per_io_data->type       = State::Read;
        per_io_data->bytes_read = 0;

        WSABUF wsa_buf;
        wsa_buf.len = sizeof(per_io_data->data_buff);
        wsa_buf.buf = (CHAR *) &per_io_data->data_buff;

        unsigned int flags = 0, bytes_read;

        if (SOCKET_ERROR == WSARecv(per_io_data->socket, &wsa_buf, 1, (LPDWORD) &bytes_read, (LPDWORD) &flags, &per_io_data->overlapped, NULL)) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                printf("Recv failed with: %d\n", WSAGetLastError());
                free(sock_data);
                free(per_io_data);
                return 0;
            }
        }
        return 1;
    }

    auto send_data(SOCK_DATA *sock_data, PER_IO_DATA *per_io_data) noexcept -> int {
        ZeroMemory(&(per_io_data->overlapped), sizeof(OVERLAPPED));
        per_io_data->type = State::Write;

        auto message  = std::string_view(per_io_data->sdata, per_io_data->bytes_read);
        auto response = handler(message);
        WSABUF wsa_buf;
        wsa_buf.len = static_cast<ULONG>(response.size());
        wsa_buf.buf = response.data();

        unsigned int flags = 0;
        if (SOCKET_ERROR == WSASend(per_io_data->socket, &wsa_buf, 1, NULL, flags, &per_io_data->overlapped, NULL)) {
            if (WSAGetLastError() != WSA_IO_PENDING) {
                printf("Send failed with: %d\n", WSAGetLastError());
                if (per_io_data->sdata) {
                    free(per_io_data->sdata);
                }
                free(per_io_data);
                closesocket(sock_data->socket);
                free(sock_data);
                return 0;
            }
        }
        return 1;
    }

    uint32_t port;

    std::function<std::string(std::string_view)> handler;
};

auto start_server(uint32_t port, const std::function<std::string(std::string_view)> &handler) noexcept -> void {
    sender_reciever sender(port, handler);
}
