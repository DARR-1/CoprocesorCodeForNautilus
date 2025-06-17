#include "Server.h"
#include <stdio.h>
#include <Ws2tcpip.h>

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

/* Initialize Winsock */
int Server::initialize()
{
    error = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (error)
    {
        printf("WSAStartup() failed with error: %d\n", error);
        return 1;
    }

    ListenSocket = socket(AF_INET, SOCK_STREAM, 0);

    if (ListenSocket == INVALID_SOCKET)
    {
        printf("socket() failed with error: %d\n", WSAGetLastError());
        WSACleanup();
        return 1;
    }

    service.sin_family = AF_INET;
    service.sin_port = htons(port);
    struct addrinfo hints, *result = NULL;
    ZeroMemory(&hints, sizeof(hints));
    hints.ai_family = AF_INET;
    error = getaddrinfo(hostname.c_str(), NULL, &hints, &result);
    if (error != 0)
    {
        printf("getaddrinfo() failed with error: %d\n", error);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }

    struct sockaddr_in *sock = (struct sockaddr_in *)result->ai_addr;
    char ipBuffer[INET_ADDRSTRLEN];
    if (InetNtop(AF_INET, &sock->sin_addr, ipBuffer, sizeof(ipBuffer)) == NULL)
    {
        printf("InetNtop() failed with error: %ld\n", WSAGetLastError());
        freeaddrinfo(result);
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    ip = ipBuffer; // std::string hace copia
    freeaddrinfo(result);

    if (inet_pton(AF_INET, ip.c_str(), &service.sin_addr) <= 0)
    {
        printf("inet_pton() failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    error = bind(ListenSocket, (SOCKADDR *)&service, sizeof(SOCKADDR));
    if (error == SOCKET_ERROR)
    {
        printf("bind() failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    return 0;
}

int Server::listen()
{
    error = ::listen(ListenSocket, SOMAXCONN);
    if (error == SOCKET_ERROR)
    {
        printf("listen() failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return 1;
    }
    printf("Listening...\n");
    return 0;
}

int Server::accept()
{
    struct sockaddr_in cli;
    int cliSize = sizeof(cli);
    AcceptSocket = ::accept(ListenSocket, (sockaddr *)&cli, &cliSize);
    if (AcceptSocket == INVALID_SOCKET)
    {
        printf("accept() failed with error: %d\n", WSAGetLastError());
        closesocket(ListenSocket);
        WSACleanup();
        return -1;
    }

    char clientIp[INET_ADDRSTRLEN];
    InetNtop(AF_INET, &cli.sin_addr, clientIp, sizeof(clientIp));
    u_short clientPort = ntohs(cli.sin_port);

    printf("Accepted connection from %s:%d\n", clientIp, clientPort);
    return 0;
}

int Server::close()
{
    closesocket(AcceptSocket);
    closesocket(ListenSocket);
    WSACleanup();
    printf("Server closed.\n");

    return 0;
}