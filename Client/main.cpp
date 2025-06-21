#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm> // <-- necesario para std::all_of

#pragma comment(lib, "ws2_32.lib")

bool sendMessage(SOCKET sock, const std::string &msg)
{
    int len = msg.size();
    if (send(sock, reinterpret_cast<const char *>(&len), sizeof(int), 0) == SOCKET_ERROR)
        return false;

    std::cout << "\033[32mEnviando mensaje: " << len << " (" << sizeof(int) << " bytes)\033[37m\n";

    std::cout << "\033[32mEnviando mensaje: " << msg << " (" << len << " bytes)\033[37m\n";
    return send(sock, msg.c_str(), len, 0) != SOCKET_ERROR;
}

bool receiveMessage(SOCKET sock, std::string &msg)
{
    int len = 0;
    if (recv(sock, reinterpret_cast<char *>(&len), sizeof(int), 0) <= 0)
        return false;
    if (len <= 0 || len > 10000)
        return false;

    std::cout << "\033[34m[server] " << len << " (" << sizeof(int) << " bytes)\033[37m\n";

    std::vector<char> buffer(len + 1, 0);
    if (recv(sock, buffer.data(), len, 0) <= 0)
        return false;

    msg = buffer.data();
    std::cout << "\033[34m[server] " << msg << " (" << len << " bytes)\033[37m\n";
    return true;
}

int main(int, char **)
{
    SOCKET clientSocket;
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    std::string ip;
    int port;

    std::cout << "Ingrese la dirección IP del servidor: ";
    std::getline(std::cin, ip);

    std::cout << "Ingrese el puerto del servidor: ";
    std::cin >> port;
    std::cin.ignore();

    if (WSAStartup(wVersionRequested, &wsaData) != 0)
    {
        std::cerr << "\033[31mWSAStartup failed\033[37m\n"; // Mensaje en rojo
        return 1;
    }

    clientSocket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (clientSocket == INVALID_SOCKET)
    {
        std::cerr << "\033[31mSocket creation failed: " << WSAGetLastError() << "\033[37m\n"; // Mensaje en rojo
        WSACleanup();
        return 1;
    }

    sockaddr_in serv{};
    serv.sin_family = AF_INET;
    serv.sin_port = htons(port);
    InetPton(AF_INET, ip.c_str(), &serv.sin_addr);

    if (connect(clientSocket, (SOCKADDR *)&serv, sizeof(serv)) == SOCKET_ERROR)
    {
        std::cerr << "\033[31mConnect failed: " << WSAGetLastError() << "\033[37m\n"; // Mensaje en rojo
        closesocket(clientSocket);
        WSACleanup();
        return 1;
    }

    sockaddr_in localAddr;
    int addrLen = sizeof(localAddr);
    if (getsockname(clientSocket, (sockaddr *)&localAddr, &addrLen) != 0)
    {
        std::cerr << "\033[31mError al obtener la información del cliente: " << WSAGetLastError() << "\033[37m\n"; // Mensaje en rojo
    }

    std::string cmd, msg;
    bool running = true, onCmd = false;

    while (running)
    {
        if (!onCmd)
        {
            std::cout << "\nComando (pathfind, test, exit, close): ";
            std::getline(std::cin, cmd);
            sendMessage(clientSocket, cmd);
            onCmd = true;
        }

        if (cmd == "pathfind")
        {
            receiveMessage(clientSocket, msg);

            std::string input;

            std::cout << "Ingrese source x: ";
            std::getline(std::cin, input);
            sendMessage(clientSocket, input);
            receiveMessage(clientSocket, msg);

            std::cout << "Ingrese source y: ";
            std::getline(std::cin, input);
            sendMessage(clientSocket, input);
            receiveMessage(clientSocket, msg);

            std::cout << "Ingrese destination x: ";
            std::getline(std::cin, input);
            sendMessage(clientSocket, input);
            receiveMessage(clientSocket, msg);

            std::cout << "Ingrese destination y: ";
            std::getline(std::cin, input);
            sendMessage(clientSocket, input);
            receiveMessage(clientSocket, msg);

            while (true)
            {
                receiveMessage(clientSocket, msg);
                if (msg == "end")
                    break;

                if (!std::all_of(msg.begin(), msg.end(), ::isdigit) && !(msg[0] == '-' && std::isdigit(msg[1])))
                {
                    std::cerr << "Mensaje inesperado recibido: " << msg << "\n";
                    break; // o continúa, dependiendo de tu lógica
                }

                int x = std::stoi(msg);

                receiveMessage(clientSocket, msg);
                if (!std::all_of(msg.begin(), msg.end(), ::isdigit) && !(msg[0] == '-' && std::isdigit(msg[1])))
                {
                    std::cerr << "Mensaje inesperado recibido: " << msg << "\n";
                    break;
                }

                int y = std::stoi(msg);

                std::cout << "Path: x" << x << " y" << y << "\n";
            }

            sendMessage(clientSocket, "end");
            receiveMessage(clientSocket, msg);

            onCmd = false;
        }
        else if (cmd == "test")
        {
            receiveMessage(clientSocket, msg);

            if (msg != "ok")
                continue;

            sendMessage(clientSocket, "ok");
            receiveMessage(clientSocket, msg);

            if (msg == "test ok")
            {
                std::cout << "Prueba completada con éxito.\n";
            }
            onCmd = false;
        }
        else if (cmd == "exit")
        {
            running = false;
        }
        else if (cmd == "close")
        {
            break;
        }
        else
        {
            receiveMessage(clientSocket, msg);
            onCmd = false;
        }
    }

    closesocket(clientSocket);
    WSACleanup();
    std::cout << "Cliente finalizado.\n";
    return 0;
}
