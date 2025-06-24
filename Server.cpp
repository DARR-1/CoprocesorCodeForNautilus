#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <chrono>
#include <thread>
#include <string>
#include <cstring>
#include <cstdlib> // Necesario para exit()

#ifdef _WIN32
#include <iphlpapi.h>
#pragma comment(lib, "iphlpapi.lib")
#endif

#include "astar.h"
#include "Server.h"
#include "ClientConnection.h"

using Grid = std::vector<std::vector<int>>;
using Path = std::vector<Pair>;

Server::~Server() {}

Server::Server(u_short port, std::string hostname)
{
    this->port = port;
    if (hostname.empty())
    {
        this->hostname = getLocalIP();
        std::cout << "\033[1;33m🔍 IP no especificada, detectando automáticamente: " << this->hostname << "\033[0m\n";
    }
    else
    {
        this->hostname = hostname;
    }
}

// Función para obtener la IP local automáticamente
std::string Server::getLocalIP()
{
#ifdef _WIN32
    // Implementación para Windows usando GetAdaptersInfo
    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = nullptr;
    DWORD dwRetVal = 0;
    UINT i;

    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    pAdapterInfo = (IP_ADAPTER_INFO *)malloc(sizeof(IP_ADAPTER_INFO));
    if (pAdapterInfo == nullptr)
    {
        return "127.0.0.1"; // Fallback a localhost
    }

    // Primer llamada para obtener el tamaño necesario
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW)
    {
        free(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO *)malloc(ulOutBufLen);
        if (pAdapterInfo == nullptr)
        {
            return "127.0.0.1";
        }
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR)
    {
        pAdapter = pAdapterInfo;
        while (pAdapter)
        {
            std::string adapterName = pAdapter->AdapterName;
            std::string ip = pAdapter->IpAddressList.IpAddress.String;

            // Evitar IPs de loopback, auto-asignadas y vacías
            if (ip != "0.0.0.0" &&
                ip != "127.0.0.1" &&
                !ip.empty() &&
                ip.substr(0, 7) != "169.254")
            { // Evitar IPs auto-asignadas

                free(pAdapterInfo);
                return ip;
            }
            pAdapter = pAdapter->Next;
        }
    }

    free(pAdapterInfo);
    return "127.0.0.1"; // Fallback a localhost

#else
    // Implementación para Linux/Unix
    struct ifaddrs *ifaddr, *ifa;
    int family;
    char host[NI_MAXHOST];

    if (getifaddrs(&ifaddr) == -1)
    {
        return "127.0.0.1";
    }

    for (ifa = ifaddr; ifa != nullptr; ifa = ifa->ifa_next)
    {
        if (ifa->ifa_addr == nullptr)
            continue;

        family = ifa->ifa_addr->sa_family;

        if (family == AF_INET)
        {
            int s = getnameinfo(ifa->ifa_addr, sizeof(struct sockaddr_in),
                                host, NI_MAXHOST, nullptr, 0, NI_NUMERICHOST);

            if (s == 0)
            {
                std::string ip = host;
                std::string ifName = ifa->ifa_name;

                // Evitar interfaces de loopback y direcciones auto-asignadas
                if (ip != "127.0.0.1" &&
                    ifName != "lo" &&
                    ip.substr(0, 7) != "169.254")
                {

                    freeifaddrs(ifaddr);
                    return ip;
                }
            }
        }
    }

    freeifaddrs(ifaddr);
    return "127.0.0.1"; // Fallback a localhost
#endif
}

void printSlow(const std::string &text, int delay_ms = 20)
{
    for (char c : text)
    {
        std::cout << c << std::flush;
        std::this_thread::sleep_for(std::chrono::milliseconds(delay_ms));
    }
    std::cout << std::endl;
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

    printSlow("\033[1;32m🚦 Servidor inicializado con IP: " + ip + "\033[0m\n");

    return 0;
}

int Server::listen()
{
    if (::listen(ListenSocket, SOMAXCONN) == SOCKET_ERROR)
    {
        std::cerr << "\033[1;31mlisten() failed.\033[0m\n";
        return 1;
    }
    printSlow("\033[1;34m👂 Servidor escuchando en puerto " + std::to_string(port) + "\033[0m\n");
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
    std::cout << "\n\033[33m=============🛜 Conexión aceptada de " << clientIp << ":" << clientPort << "=============\033[37m\n\n";

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
        std::cerr << "\033[31m❌ Error: Fallo al recibir el tamaño del mensaje.\033[0m\n";
        return false;
    }

    if (len <= 0 || len > 10000)
    {
        std::cerr << "\033[31m❌ Error: Tamaño del mensaje inválido (" << len << ").\033[0m\n";
        return false; // Sanity check
    }

    std::vector<char> buffer(len + 1, 0);
    if (client.receive(buffer.data(), len) <= 0)
    {
        std::cerr << "\033[31m❌ Error: Fallo al recibir el contenido del mensaje.\033[0m\n";
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
            std::cerr << "\033[31m❌ Error: Conexión cerrada o fallo en la recepción del mensaje.\033[0m\n";
            std::cout << "\n\033[33m=============🛜 Conexión cerrada con " << client.getName() << "=============\033[37m\n\n";
            break;
        }

        std::cout << "\n\033[33m=============💻 Iniciando comando: " << msg << "=============\033[37m\n\n";

        if (msg == "pathfind")
        {
            sendMessage(client, "ok");
            Pair src, dest;

            if (!receiveMessage(client, msg))
            {
                std::cerr << "\033[31m❌ Error: Fallo al recibir la posición inicial.\033[0m\n";
                break;
            }
            src.second = round((std::stod(msg.c_str()) / 0.0254) * QUALITY);
            sendMessage(client, "ok");

            if (!receiveMessage(client, msg))
            {
                std::cerr << "\033[31m❌ Error: Fallo al recibir la posición inicial (Y).\033[0m\n";
                break;
            }
            src.first = ROW - round((std::stod(msg.c_str()) / 0.0254) * QUALITY) - 1;
            sendMessage(client, "ok");

            if (!receiveMessage(client, msg))
            {
                std::cerr << "\033[31m❌ Error: Fallo al recibir la posición final.\033[0m\n";
                break;
            }
            dest.second = round((std::stod(msg.c_str()) / 0.0254) * QUALITY);
            sendMessage(client, "ok");

            if (!receiveMessage(client, msg))
            {
                std::cerr << "\033[31m❌ Error: Fallo al recibir la posición final (Y).\033[0m\n";
                break;
            }
            dest.first = ROW - round((std::stod(msg.c_str()) / 0.0254) * QUALITY) - 1;
            std::cout << "Posición inicial: (" << src.first << ", " << src.second << ")\n";
            std::cout << "Posición final: (" << dest.first << ", " << dest.second << ")\n";
            sendMessage(client, "ok");

            auto start = std::chrono::system_clock::now().time_since_epoch().count();
            auto path = aStarSearch(grid, src, dest);

            if (!path.empty())
            {

                for (const auto &point : path)
                {
                    sendMessage(client, std::to_string((0.0254 * point.second) / QUALITY));
                    sendMessage(client, std::to_string(-((0.0254 * (point.first + 1 - ROW)) / QUALITY)));
                }
                sendMessage(client, "end");

                do
                {
                    if (!receiveMessage(client, msg))
                        break;
                } while (msg != "end");

                std::cout << "Path enviado correctamente al cliente.\n";
            }
            else
            {
                std::cerr << "\033[31m❌ Error: No se encontró un camino.\033[0m\n";
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
                std::cerr << "\033[31m❌ Error: Fallo al recibir datos de prueba.\033[0m\n";
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
        std::cout << "\n\033[33m=============🛑 Finishing command=============\n\n\033[37m";
    }

    client.close();
}