#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <chrono>
#include <thread>
#include <string>
#include <cstring>
#include <cstdlib> // Necesario para exit()

#include "astar.h"
#include "Server.h"
#include "ClientConnection.h"

using Grid = std::vector<std::vector<int>>;
using Path = std::vector<Pair>;

Server::~Server() {}

using Grid = std::vector<std::vector<int>>;
using Path = std::vector<Pair>;

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

    // Imprimir la IP del servidor
    std::cout << "Servidor inicializado con IP: " << ip << "\n";

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
        perror("accept() falló"); // Imprime el error específico
        std::cerr << "accept() falló.\n";
        return ClientConnection(INVALID_SOCKET);
    }

    char clientIp[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &cli.sin_addr, clientIp, sizeof(clientIp));
    u_short clientPort = ntohs(cli.sin_port);
    std::cout << "\n\033[33m=============Conexión aceptada de " << clientIp << ":" << clientPort << "=============\033[37m\n\n";

    return ClientConnection(clientSocket);
}

void sendMessage(ClientConnection &client, const std::string &msg)
{
    int len = msg.size();
    client.send(reinterpret_cast<const char *>(&len), sizeof(int));
    client.send(msg.c_str(), len);
}

bool receiveMessage(ClientConnection &client, std::string &msg)
{
    int len = 0;
    if (client.receive(reinterpret_cast<char *>(&len), sizeof(int)) <= 0)
    {
        std::cerr << "\033[31mError: Fallo al recibir el tamaño del mensaje.\033[0m\n";
        return false;
    }

    if (len <= 0 || len > 10000)
    {
        std::cerr << "\033[31mError: Tamaño del mensaje inválido (" << len << ").\033[0m\n";
        return false; // Sanity check
    }

    std::vector<char> buffer(len + 1, 0);
    if (client.receive(buffer.data(), len) <= 0)
    {
        std::cerr << "\033[31mError: Fallo al recibir el contenido del mensaje.\033[0m\n";
        return false;
    }

    msg = buffer.data();

    return true;
}

void Server::handleClient(ClientConnection client, const Grid &grid)
{
    std::string msg;

    while (true)
    {
        if (!receiveMessage(client, msg))
        {
            std::cerr << "\033[31mError: Conexión cerrada o fallo en la recepción del mensaje.\033[0m\n";
            std::cout << "\n\033[33m=============Conexión cerrada=============\033[37m\n\n";
            break;
        }

        if (msg == "pathfind")
        {
            sendMessage(client, "ok");
            Pair src, dest;

            if (!receiveMessage(client, msg))
            {
                std::cerr << "\033[31mError: Fallo al recibir la posición inicial.\033[0m\n";
                break;
            }
            src.second = round(atof(msg.c_str()) * QUALITY);
            sendMessage(client, "ok");

            if (!receiveMessage(client, msg))
            {
                std::cerr << "\033[31mError: Fallo al recibir la posición inicial (Y).\033[0m\n";
                break;
            }
            src.first = ROW - round(atof(msg.c_str()) * QUALITY) - 1;
            sendMessage(client, "ok");

            if (!receiveMessage(client, msg))
            {
                std::cerr << "\033[31mError: Fallo al recibir la posición final.\033[0m\n";
                break;
            }
            dest.second = round(atof(msg.c_str()) * QUALITY);
            sendMessage(client, "ok");

            if (!receiveMessage(client, msg))
            {
                std::cerr << "\033[31mError: Fallo al recibir la posición final (Y).\033[0m\n";
                break;
            }
            dest.first = ROW - round(atof(msg.c_str()) * QUALITY) - 1;
            sendMessage(client, "ok");

            auto start = std::chrono::system_clock::now().time_since_epoch().count();
            auto path = aStarSearch(grid, src, dest);

            if (!path.empty())
            {
                int count = path.size();
                int sizeInBytes = path.size() * sizeof(Pair);

                client.send(reinterpret_cast<const char *>(&count), sizeof(int));
                client.send(reinterpret_cast<const char *>(&sizeInBytes), sizeof(int));
                client.send(reinterpret_cast<const char *>(path.data()), sizeInBytes);

                do
                {
                    if (!receiveMessage(client, msg))
                        break;
                } while (msg != "end");

                std::cout << "Path enviado correctamente al cliente.\n";
            }
            else
            {
                std::cerr << "\033[31mError: No se encontró un camino.\033[0m\n";
                sendMessage(client, "no path found");
            }

            auto end = std::chrono::system_clock::now().time_since_epoch().count();
            std::string execMsg = "Tiempo de ejecución: " + std::to_string((end - start) / 1000000) + " ms";
            sendMessage(client, execMsg);
        }
        else if (msg == "test")
        {
            sendMessage(client, "ok");
            std::cout << "Test request received.\n";

            if (!receiveMessage(client, msg))
            {
                std::cerr << "\033[31mError: Fallo al recibir datos de prueba.\033[0m\n";
                continue;
            }
            while (msg != "ok")
            {
                if (!receiveMessage(client, msg))
                    break;
                static bool printed = false;
                if (!printed)
                {
                    std::cout << "Esperando confirmación del cliente...\n";
                    printed = true;
                }
            }
            std::cout << "Confirmación recibida.\n";

            sendMessage(client, "test ok");
            std::cout << "Test response sent.\n";
        }
        else if (msg == "close")
        {
            client.close();
            std::cout << "Conexión cerrada por el cliente.\n";
            exit(0);
        }
        else if (msg == "exit")
        {
            sendMessage(client, "ok");
        }
        else
        {
            sendMessage(client, "unknown command");
        }
        std::cout << "\n\033[33m=============Finishing command=============\n\n\033[37m";
    }

    client.close();
}
