#include "Server.h"
#include <iostream>

Server::~Server() {}

Server::Server(u_short port, std::string hostname)
{
    this->port = port;
    this->hostname = hostname;
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

    struct addrinfo hints{}, *result = nullptr;
    hints.ai_family = AF_INET;

    error = getaddrinfo(hostname.c_str(), nullptr, &hints, &result);
    if (error != 0)
    {
        std::cerr << "getaddrinfo() failed: " << error << "\n";
        return 1;
    }

    sockaddr_in *sock = (sockaddr_in *)result->ai_addr;
    char ipBuffer[INET_ADDRSTRLEN];
#ifdef _WIN32
    InetNtop(AF_INET, &sock->sin_addr, ipBuffer, sizeof(ipBuffer));
#else
    inet_ntop(AF_INET, &sock->sin_addr, ipBuffer, sizeof(ipBuffer));
#endif
    ip = ipBuffer;
    freeaddrinfo(result);

#ifdef _WIN32
    inet_pton(AF_INET, ip.c_str(), &service.sin_addr);
#else
    inet_pton(AF_INET, ip.c_str(), &service.sin_addr);
#endif

    if (bind(ListenSocket, (sockaddr *)&service, sizeof(service)) == SOCKET_ERROR)
    {
        std::cerr << "bind() failed.\n";
        return 1;
    }

    return 0;
}

int Server::listen()
{
    if (::listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "listen() failed.\n";
        return 1;
    }
    std::cout << "Servidor escuchando en puerto " << port << "\n";
    return 0;
}

ClientConnection Server::accept()
{
    sockaddr_in cli{};
    socklen_t cliSize = sizeof(cli);
    SOCKET clientSocket = ::accept(ListenSocket, (sockaddr *)&cli, &cliSize);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "accept() falló.\n";
        return ClientConnection(INVALID_SOCKET);
    }

    char clientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cli.sin_addr, clientIp, sizeof(clientIp));
    u_short clientPort = ntohs(cli.sin_port);
    std::cout << "Conexión aceptada de " << clientIp << ":" << clientPort << "\n";

    return ClientConnection(clientSocket);
}

int Server::close()
{
#ifdef _WIN32
    closesocket(ListenSocket);
    WSACleanup();
#else
    ::close(ListenSocket);
#endif
    printf("Server closed.\n");
    return 0;
}