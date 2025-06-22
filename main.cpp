#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <chrono>
#include <thread>
#include <string>
#include <cstring>
#include <cstdlib> // exit(), system()
#include <cstdio>  // popen()
#ifdef __unix__
#include <unistd.h> // para getuid()
#endif

#include "astar.h"
#include "Server.h"
#include "ClientConnection.h"

#define ROBOT_RADIUS 17

using Grid = std::vector<std::vector<int>>;
using Path = std::vector<Pair>;

// Ejecutar figlet (popen variante cruzada)
void printFigletTitle(const std::string &title)
{
#ifdef _WIN32
    FILE *pipe = _popen(("figlet \"" + title + "\"").c_str(), "r");
#else
    system("sudo apt install figlet");
    FILE *pipe = popen(("figlet \"" + title + "\"").c_str(), "r");
#endif
    if (!pipe)
        return;

    char buffer[128];
    while (fgets(buffer, sizeof(buffer), pipe) != nullptr)
    {
        std::cout << "\033[1;35m" << buffer << "\033[0m";
    }

#ifdef _WIN32
    _pclose(pipe);
#else
    pclose(pipe);
#endif
}

int main(int argc, char *argv[])
{

    {
#ifdef _WIN32
        // Forzar consola UTF-8
        system("chcp 65001 > nul");
#else
        if (getuid() != 0)
        {
            std::cout << "Requiere permisos de superusuario. Intentando relanzar con sudo...\n";

            // Construir el comando: sudo + ruta del ejecutable + argumentos
            std::string cmd = "sudo ";
            for (int i = 0; i < argc; ++i)
            {
                cmd += "\"";
                cmd += argv[i];
                cmd += "\" ";
            }

            int result = system(cmd.c_str());
            return result;
        }

        std::cout << "Ejecutando con permisos de root.\n";
#endif

        const std::string reset = "\033[0m";
        const std::string bold = "\033[1m";
        const std::string cyan = "\033[36m";

        std::cout << bold << cyan;
        std::cout << "\n============================================================\n";
        std::cout << "\033[1;32m=                                                          =\n";
        std::cout << "\033[1;32m=              INICIANDO CODIGO COPROCESADOR               =\n";
        std::cout << "\033[1;32m=                       by: André 🍐                       =\n";
        std::cout << "\033[1;32m=                                                          =\n";
        std::cout << bold << cyan;
        std::cout << "============================================================\n\n";
        std::cout << reset;
        printFigletTitle("Nautilus 4010");
    }

    Server server(27015);
    if (server.initialize() != 0 || server.listen() != 0)
        return 1;

    Grid grid(ROW, std::vector<int>(COL));

    for (int i = 0; i < ROW; ++i)
    {
        for (int j = 0; j < COL; ++j)
        {
            if (
                (abs(-j + (176.745 * QUALITY)) <= (sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY)) / 2 &&
                 sqrt(3) * abs(i - (158.5 * QUALITY)) - j + (176.745 * QUALITY) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY) &&
                 sqrt(3) * abs(i - (158.5 * QUALITY)) + j - (176.745 * QUALITY) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY)) ||

                (abs(-j + (-176.745 * QUALITY + (COL - 1))) <= (sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY)) / 2 &&
                 sqrt(3) * abs(i - (158.5 * QUALITY)) - j + (-176.745 * QUALITY + (COL - 1)) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY) &&
                 sqrt(3) * abs(i - (158.5 * QUALITY)) + j - (-176.745 * QUALITY + (COL - 1)) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY)) ||

                (j > ((COL - 1) / 2) - (6 + ROBOT_RADIUS) * QUALITY &&
                 j < ((COL - 1) / 2) + (6 + ROBOT_RADIUS) * QUALITY &&
                 i > ((ROW - 1) / 2) - (6 + ROBOT_RADIUS) * QUALITY &&
                 i < ((ROW - 1) / 2) + (6 + ROBOT_RADIUS) * QUALITY))
            {
                grid[i][j] = 0;
            }
            else
            {
                grid[i][j] = 1;
            }
        }
    }

    while (true)
    {
        std::cout << "\n🟢 \033[1;32mEsperando nuevo cliente...\033[0m\n";

        ClientConnection client = server.accept();
        if (client.getSocket() != -1)
        {
            std::thread clientThread(&Server::handleClient, &server, std::move(client), std::ref(grid));
            clientThread.detach();
        }
    }

    return 0;
}
