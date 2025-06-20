#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <vector>
#include <cstdlib>

#pragma comment(lib, "ws2_32.lib")

#pragma pack(push, 1)
struct Pair
{
    int first;
    int second;
};
#pragma pack(pop)

int main(int, char **)
{
    SOCKET clientSocket;
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    std::string ip;
    int port;

    // Solicitar IP y puerto al usuario
    std::cout << "Ingrese la dirección IP del servidor: ";
    std::getline(std::cin, ip);

    std::cout << "Ingrese el puerto del servidor: ";
    std::cin >> port;
    std::cin.ignore(); // Limpiar el buffer de entrada

    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        std::cerr << "WSAStartup failed\n";
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "Socket creation failed: " << WSAGetLastError() << "\n";
        WSACleanup();
        return 1;
    }

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    InetPton(AF_INET, ip.c_str(), &serv.sin_addr);

    if (connect(clientSocket, (SOCKADDR *)&serv, sizeof(serv)) == SOCKET_ERROR)
    {
        std::cerr << "Connect failed: " << WSAGetLastError() << "\n";
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    std::cout << "Conectado al servidor\n";

    char buffer[200] = {};
    std::string cmd;
    bool running = true;
    bool onCmd = false;

    while (running)
    {
        if (!onCmd)
        {
            std::cout << "\nComando (pathfind, test, exit, close): ";
            std::getline(std::cin, cmd);
            std::cout << "Enviando comando: " << cmd << "\n";
            memset(buffer, 0, sizeof(buffer));
            send(clientSocket, cmd.c_str(), 200, 0);
            onCmd = true;
        }

        if (cmd == "pathfind")
        {
            // 1) Confirmación inicial
            recv(clientSocket, buffer, sizeof(buffer), 0);
            std::cout << "[srv] " << buffer << "\n";

            // 2) Enviar source x, y y destination x, y
            std::string input;

            std::cout << "Ingrese source x: ";
            std::getline(std::cin, input);
            send(clientSocket, input.c_str(), 200, 0);
            recv(clientSocket, buffer, sizeof(buffer), 0);
            std::cout << "[srv] " << buffer << "\n";

            std::cout << "Ingrese source y: ";
            std::getline(std::cin, input);
            send(clientSocket, input.c_str(), 200, 0);
            recv(clientSocket, buffer, sizeof(buffer), 0);
            std::cout << "[srv] " << buffer << "\n";

            std::cout << "Ingrese destination x: ";
            std::getline(std::cin, input);
            send(clientSocket, input.c_str(), 200, 0);
            recv(clientSocket, buffer, sizeof(buffer), 0);
            std::cout << "[srv] " << buffer << "\n";

            std::cout << "Ingrese destination y: ";
            std::getline(std::cin, input);
            send(clientSocket, input.c_str(), 200, 0);
            recv(clientSocket, buffer, sizeof(buffer), 0);
            std::cout << "[srv] " << buffer << "\n";

            // 3) Recibir path
            std::cout << "Recibiendo path...\n";

            recv(clientSocket, buffer, sizeof(buffer), 0);
            if (strcmp(buffer, "no path found") == 0)
            {
                std::cout << "[srv] " << buffer << "\n";
                onCmd = false;
                continue;
            }

            std::cout << "[srv] " << buffer << "\n";
            int count = atoi(buffer);

            recv(clientSocket, buffer, sizeof(buffer), 0);
            int size = atoi(buffer); // opcional

            std::cout << "Recibiendo " << count << " pares (" << count * sizeof(Pair) << " bytes)\n";

            // Recibir los datos binarios del path
            int totalBytes = count * sizeof(Pair);
            char *rawPath = new char[totalBytes];
            int received = 0;

            while (received < totalBytes)
            {
                int bytes = recv(clientSocket, rawPath + received, totalBytes - received, 0);
                if (bytes <= 0)
                {
                    std::cerr << "Error al recibir los datos binarios.\n";
                    delete[] rawPath;
                    break;
                }
                received += bytes;
            }

            std::vector<Pair> path(count);
            memcpy(path.data(), rawPath, totalBytes);
            delete[] rawPath;

            std::cout << "Path recibido con " << path.size() << " puntos:\n";
            for (const auto &point : path)
            {
                std::cout << "(" << point.first << ", " << point.second << ")\n";
            }

            send(clientSocket, "end", 200, 0);
            recv(clientSocket, buffer, sizeof(buffer), 0);
            std::cout << "[srv] " << buffer << "\n";

            onCmd = false;
            continue;
        }
        else if (cmd == "test")
        {
            recv(clientSocket, buffer, sizeof(buffer), 0);
            if (strcmp(buffer, "ok") != 0)
            {
                std::cout << "Esperando confirmación del servidor...\n";
                continue;
            }
            std::cout << "Confirmación recibida.\n";
            std::cout << "[srv] " << buffer << "\n";

            send(clientSocket, "ok", 200, 0);
            recv(clientSocket, buffer, sizeof(buffer), 0);
            std::cout << "[srv] " << buffer << "\n";

            if (strcmp(buffer, "test ok") == 0)
            {
                std::cout << "Prueba completada con éxito.\n";
            }
            onCmd = false;
            continue;
        }
        else if (cmd == "exit")
        {
            std::cout << "Cerrando conexión...\n";
            running = false;
        }
        else if (cmd == "close")
        {
            break;
        }
        else
        {
            recv(clientSocket, buffer, sizeof(buffer), 0);
            std::cout << "[srv] " << buffer << "\n";
            onCmd = false;
            continue;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    std::cout << "Cliente finalizado.\n";
    return 0;
}
