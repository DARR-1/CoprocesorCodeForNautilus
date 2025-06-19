#include "Server.h"
#include <stdio.h>

#ifdef _WIN32
#include <winsock2.h>
#include <Ws2tcpip.h>
#pragma comment(lib, "Ws2_32.lib")
#else
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <netdb.h>
#define INVALID_SOCKET (-1)
#define SOCKET_ERROR (-1)
#define closesocket close
typedef int SOCKET;
#endif

Server::~Server() {}

Server::Server(u_short port, std::string hostname)
{
    this->port = port;
    this->hostname = hostname;
}

Server::Server(u_short port)
{
    this->port = port;
    this->hostname = "0.0.0.0";
}

/* Initialize Winsock (solo en Windows) */
int Server::initialize()
{
#ifdef _WIN32
    error = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (error)
    {
        printf("WSAStartup() failed con error: %d\n", error);
        return 1;
    }
#endif

    ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ListenSocket == INVALID_SOCKET)
    {
        printf("socket() failed con error: %d\n",
#ifdef _WIN32
               WSAGetLastError()
#else
               errno
#endif
        );
#ifdef _WIN32
        WSACleanup();
#endif
        return 1;
    }

    service.sin_family = AF_INET;
    service.sin_port = htons(port);

    struct addrinfo hints, *result = nullptr;
    #ifdef _WIN32
ZeroMemory(&hints, sizeof(hints));
#else
memset(&hints, 0, sizeof(hints));
#endif
    hints.ai_family = AF_INET;

    error = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
    if (error != 0)
    {
        printf("getaddrinfo() failed con error: %d\n", error);
        close();
        return 1;
    }

    sockaddr_in *sock = (sockaddr_in *)result->ai_addr;
    char ipBuffer[INET_ADDRSTRLEN];
    if (
#ifdef _WIN32
        InetNtop(AF_INET, &sock->sin_addr, ipBuffer, sizeof(ipBuffer)) == nullptr
#else
        !inet_ntop(AF_INET, &sock->sin_addr, ipBuffer, sizeof(ipBuffer))
#endif
    )
    {
        printf("InetNtop() failed con error: %d\n",
#ifdef _WIN32
               WSAGetLastError()
#else
               errno
#endif
        );
        freeaddrinfo(result);
        close();
        return 1;
    }
    ip = ipBuffer;
    freeaddrinfo(result);

    if (
#ifdef _WIN32
        inet_pton(AF_INET, ip.c_str(), &service.sin_addr) <= 0
#else
        inet_pton(AF_INET, ip.c_str(), &service.sin_addr) != 1
#endif
    )
    {
        printf("inet_pton() failed con error: %d\n",
#ifdef _WIN32
               WSAGetLastError()
#else
               errno
#endif
        );
        close();
        return 1;
    }

    error = bind(ListenSocket, (sockaddr *)&service, sizeof(service));
    if (error == SOCKET_ERROR)
    {
        printf("bind() failed con error: %d\n",
#ifdef _WIN32
               WSAGetLastError()
#else
               errno
#endif
        );
        close();
        return 1;
    }
    return 0;
}

int Server::listen()
{
    error = ::listen(ListenSocket, SOMAXCONN);
    if (error == SOCKET_ERROR)
    {
        printf("listen() failed con error: %d\n",
#ifdef _WIN32
               WSAGetLastError()
#else
               errno
#endif
        );
        close();
        return 1;
    }
    printf("Listening...\n");
    return 0;
}

int Server::accept()
{
    sockaddr_in cli;
    socklen_t cliSize = sizeof(cli);
    AcceptSocket = ::accept(ListenSocket, (sockaddr *)&cli, &cliSize);
    if (AcceptSocket == INVALID_SOCKET)
    {
        printf("accept() failed con error: %d\n",
#ifdef _WIN32
               WSAGetLastError()
#else
               errno
#endif
        );
        close();
        return -1;
    }

    char clientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cli.sin_addr, clientIp, sizeof(clientIp));
    u_short clientPort = ntohs(cli.sin_port);

    printf("Accepted connection from %s:%d\n", clientIp, clientPort);
    return 0;
}

int Server::send(const char *buffer, int length)
{
    int bytesSent = ::send(AcceptSocket, buffer, length, 0);
    if (bytesSent == SOCKET_ERROR)
    {
        printf("send() failed con error: %d\n",
#ifdef _WIN32
               WSAGetLastError()
#else
               errno
#endif
        );
        return -1;
    }
    printf("Sent %d bytes to client.\n", bytesSent);
    return bytesSent;
}

int Server::receive(char *buffer, int length)
{
    int bytesReceived = ::recv(AcceptSocket, buffer, length, 0);
    if (bytesReceived == SOCKET_ERROR)
    {
        printf("recv() failed con error: %d\n",
#ifdef _WIN32
               WSAGetLastError()
#else
               errno
#endif
        );
        return -1;
    }
    printf("Received %d bytes from client.\n", bytesReceived);
    return bytesReceived;
}

int Server::close()
{
#ifdef _WIN32
    closesocket(AcceptSocket);
    closesocket(ListenSocket);
    WSACleanup();
#else
    ::close(AcceptSocket);
    ::close(ListenSocket);
#endif
    printf("Server closed.\n");
    return 0;
}