#include "Server.h"
#include <iostream>
#include <cstring>
#include <thread>
#include <vector>
#include <chrono>
#include <cstdlib>
#include <cmath>
#include "astar.h"
#include "ClientConnection.h"

Server::~Server() {}

Server::Server(u_short port)
{
    this->port = port;
    this->ip = getLocalIP();  // Detecta IP automáticamente
}

std::string Server::getLocalIP()
{
    std::string localIP = "127.0.0.1";
    int sock = socket(AF_INET, SOCK_DGRAM, 0);
    if (sock == -1) return localIP;

    sockaddr_in remoteAddr{};
    remoteAddr.sin_family = AF_INET;
    remoteAddr.sin_port = htons(80);
    inet_pton(AF_INET, "8.8.8.8", &remoteAddr.sin_addr);

    connect(sock, (sockaddr*)&remoteAddr, sizeof(remoteAddr));

    sockaddr_in localAddr{};
    socklen_t addrLen = sizeof(localAddr);
    getsockname(sock, (sockaddr*)&localAddr, &addrLen);

    char buffer[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &localAddr.sin_addr, buffer, sizeof(buffer));

    close(sock);
    return std::string(buffer);
}

int Server::initialize()
{
#ifdef _WIN32
    error = WSAStartup(MAKEWORD(2, 2), &wsaData);
    if (error)
    {
        std::cerr << "WSAStartup() failed: " << error << "\n";
        return 1;
    }
#endif

    ListenSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (ListenSocket == INVALID_SOCKET)
    {
        std::cerr << "socket() failed.\n";
        return 1;
    }

    service.sin_family = AF_INET;
    service.sin_port = htons(port);

    if (inet_pton(AF_INET, ip.c_str(), &service.sin_addr) <= 0)
    {
        std::cerr << "inet_pton() failed. IP: " << ip << "\n";
        return 1;
    }

    if (bind(ListenSocket, (sockaddr *)&service, sizeof(service)) == SOCKET_ERROR)
    {
        std::cerr << "bind() failed.\n";
        return 1;
    }

    std::cout << "\033[1;32m🚦 Servidor inicializado con IP: " << ip \033[0m\n";
    return 0;
}

int Server::listen()
{
    if (::listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "\033[1;31mlisten() failed.\033[0m\n";
        return 1;
    }
    std::cout << "\033[1;32m👂 Servidor escuchando en puerto " << port << "\033[0m\n";
    return 0;
}

ClientConnection Server::accept()
{
    sockaddr_in cli{};
    socklen_t cliSize = sizeof(cli);
    SOCKET clientSocket = ::accept(ListenSocket, (sockaddr *)&cli, &cliSize);
    if (clientSocket == INVALID_SOCKET)
    {
        perror("accept() falló");
        std::cerr << "accept() falló.\n";
        return ClientConnection(INVALID_SOCKET);
    }

    char clientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cli.sin_addr, clientIp, sizeof(clientIp));
    u_short clientPort = ntohs(cli.sin_port);
    std::cout << "\n\033[33m=============🛜 Conexión aceptada de " << clientIp << ":" << clientPort << "=============\033[37m\n\n";

    return ClientConnection(clientSocket);
}

// Aquí seguiría el resto del código de handleClient(), etc.
