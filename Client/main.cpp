#include <iostream>
#include <winsock2.h>
#include <Ws2tcpip.h>
#include <string>
#include <vector>
#include <cstdlib>
#include <algorithm> // <-- necesario para std::all_of
#include <fstream>   // <-- necesario para std::ifstream

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

void printLogoFromFile()
{
    std::string logoPath = "bin\\logo.txt";
    std::ifstream logoFile(logoPath);

    if (!logoFile.is_open())
    {
        std::cerr << "\033[31mNo se pudo abrir el archivo: " << logoPath << "\033[37m\n";
        return;
    }

    std::string line;
    while (std::getline(logoFile, line))
    {
        std::string filteredLine = "";
        bool inEscapeSequence = false;

        for (size_t i = 0; i < line.length(); ++i)
        {
            if (line[i] == '\033' || (inEscapeSequence && line[i] == '['))
            {
                inEscapeSequence = true;
                continue;
            }
            else if (inEscapeSequence && line[i] == 'm')
            {
                inEscapeSequence = false;
                continue;
            }
            else if (inEscapeSequence)
            {
                // Saltar caracteres de la secuencia de escape
                continue;
            }
            else if (line[i] == '.')
            {
                // Reemplazar puntos por espacios
                filteredLine += ' ';
            }
            else if (line[i] == ' ')
            {
                // Mantener espacios
                filteredLine += ' ';
            }
            else
            {
                // Para cualquier otro caracter visible, usar color amarillo
                filteredLine += "\033[1;33m" + std::string(1, line[i]) + "\033[0m";
            }
        }

        std::cout << filteredLine << "\n";
    }

    logoFile.close();
}

int main(int, char **)
{
#ifdef _WIN32
    // Forzar consola UTF-8
    system("chcp 65001 > nul");
#endif
    // Título del programa
    const std::string reset = "\033[0m";
    const std::string bold = "\033[1m";
    const std::string cyan = "\033[36m";

    std::cout << bold << cyan;
    std::cout << "\n╔══════════════════════════════════════════════════════════╗\n";
    std::cout << "║                                                          ║\n";
    std::cout << "║              \033[1;32mINICIANDO CODIGO COPROCESADOR\033[36m               ║\n";
    std::cout << "║                       \033[1;32mby: André 🍐\033[36m                       ║\n";
    std::cout << "║                                                          ║\n";
    std::cout << "╚══════════════════════════════════════════════════════════╝\n\n";
    std::cout << reset;

    printLogoFromFile();

    std::cout << "\033[1;33m";
    std::cout <<
        R"(
███╗   ██╗ █████╗ ██╗   ██╗████████╗██╗██╗     ██╗   ██╗███████╗    ██╗  ██╗ ██████╗  ██╗ ██████╗        ██████╗ 
████╗  ██║██╔══██╗██║   ██║╚══██╔══╝██║██║     ██║   ██║██╔════╝    ██║  ██║██╔═████╗███║██╔═████╗    ██╗██╔══██╗
██╔██╗ ██║███████║██║   ██║   ██║   ██║██║     ██║   ██║███████╗    ███████║██║██╔██║╚██║██║██╔██║    ╚═╝██║  ██║
██║╚██╗██║██╔══██║██║   ██║   ██║   ██║██║     ██║   ██║╚════██║    ╚════██║████╔╝██║ ██║████╔╝██║    ██╗██║  ██║
██║ ╚████║██║  ██║╚██████╔╝   ██║   ██║███████╗╚██████╔╝███████║         ██║╚██████╔╝ ██║╚██████╔╝    ╚═╝██████╔╝
╚═╝  ╚═══╝╚═╝  ╚═╝ ╚═════╝    ╚═╝   ╚═╝╚══════╝ ╚═════╝ ╚══════╝         ╚═╝ ╚═════╝  ╚═╝ ╚═════╝        ╚═════╝ 
)" << std::endl;

    SOCKET clientSocket;
    WSADATA wsaData;
    WORD wVersionRequested = MAKEWORD(2, 2);

    std::string ip;
    int port;

    std::cout << "\033[36m\nIngrese la dirección IP del servidor: \033[37m";
    std::getline(std::cin, ip);

    std::cout << "\033[36mIngrese el puerto del servidor: \033[37m";
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
                {
                    sendMessage(clientSocket, "end");
                    break;
                }

                // Función auxiliar para validar si es un número válido (entero o decimal)
                auto isValidNumber = [](const std::string &str) -> bool
                {
                    if (str.empty())
                        return false;

                    size_t start = 0;
                    if (str[0] == '-')
                    {
                        if (str.length() == 1)
                            return false;
                        start = 1;
                    }

                    bool hasDot = false;
                    for (size_t i = start; i < str.length(); ++i)
                    {
                        if (str[i] == '.')
                        {
                            if (hasDot)
                                return false; // Más de un punto
                            hasDot = true;
                        }
                        else if (!std::isdigit(str[i]))
                        {
                            return false;
                        }
                    }
                    return true;
                };

                if (!isValidNumber(msg))
                {
                    std::cerr << "\033[31mMensaje inesperado recibido: " << msg << "\033[37m\n"; // Mensaje en rojo
                    break;
                }

                double x = std::stod(msg);

                receiveMessage(clientSocket, msg);
                if (!isValidNumber(msg))
                {
                    std::cerr << "\033[31mMensaje inesperado recibido: " << msg << "\033[37m\n"; // Mensaje en rojo
                    break;
                }

                double y = std::stod(msg);

                std::cout << "\033[33mPath: x" << x << " y" << y << "\033[37m\n"; // Mensaje en amarillo
            }
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
                std::cout << "\033[32mPrueba completada con éxito.\033[37m\n";
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
    std::cout << "\033[36mCliente finalizado.\033[37m\n"; // Mensaje en cian
    return 0;
}
