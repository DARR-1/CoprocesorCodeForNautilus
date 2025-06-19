#include <iostream>
#include <vector>
#include <cmath>
#include <fstream>
#include <chrono>
#include <thread>
#include <string>
#include <cstring>

#include "astar.h"
#include "Server.h"
#include "ClientConnection.h"

#define ROBOT_RADIUS 17

using Grid = std::vector<std::vector<int>>;
using Path = std::vector<Pair>;

int handleClient(ClientConnection client, const Grid &grid, Server &server)
{
    char buffer[200] = "";

    while (true)
    {
        int ret = client.receive(buffer, 200);
        if (ret <= 0)
        {
            std::cerr << "Conexión cerrada o error.\n";
            break;
        }

        std::cout << "Received from client: " << buffer << std::endl;

        if (strcmp(buffer, "pathfind") == 0)
        {
            client.send("ok", 200);
            Pair src, dest;

            client.receive(buffer, 200);
            src.second = round(atof(buffer) * QUALITY);
            client.send("ok", 200);

            client.receive(buffer, 200);
            src.first = ROW - round(atof(buffer) * QUALITY) - 1;
            client.send("ok", 200);

            client.receive(buffer, 200);
            dest.second = round(atof(buffer) * QUALITY);
            client.send("ok", 200);

            client.receive(buffer, 200);
            dest.first = ROW - round(atof(buffer) * QUALITY) - 1;
            client.send("ok", 200);

            auto start = std::chrono::system_clock::now().time_since_epoch().count();
            auto path = aStarSearch(grid, src, dest);

            if (!path.empty())
            {
                client.send(std::to_string(path.size()).c_str(), 200);
                client.send(std::to_string(sizeof(Pair) * path.size()).c_str(), 200);
                client.send(reinterpret_cast<const char *>(path.data()), path.size() * sizeof(Pair));

                client.receive(buffer, 200);
                while (strcmp(buffer, "end") != 0)
                    client.receive(buffer, 200);

                std::cout << "Path enviado correctamente al cliente.\n";
            }
            else
            {
                client.send("no path found", 200);
            }

            auto end = std::chrono::system_clock::now().time_since_epoch().count();
            client.send(("Tiempo de ejecución: " + std::to_string((end - start) / 1000000) + " ms\n").c_str(), 200);
        }
        else if (strcmp(buffer, "test") == 0)
        {
            client.send("ok", 200);
            std::cout << "Test request received.\n";

            buffer[0] = '\0'; // Limpiar buffer
            if (client.receive(buffer, 200) == -1)
            {
                std::cerr << "Error receiving test data.\n";
                continue;
            }
            while (strcmp(buffer, "ok") != 0)
            {
                client.receive(buffer, 200);
                static bool printed = false;
                if (!printed)
                {
                    std::cout << "Esperando confirmación del cliente...\n";
                    printed = true;
                }
            }
            std::cout << "Confirmación recibida.\n";

            client.send("test ok", 200);
            std::cout << "Test response sent.\n";
        }
        else if (strcmp(buffer, "close") == 0)
        {
            client.close();
            server.close();
            std::cout << "Conexión cerrada por el cliente.\n";
            return 0;
        }
        else if (strcmp(buffer, "exit") == 0)
        {
            break;
        }
        else
        {
            client.send("unknown command", 200);
        }

        buffer[0] = '\0';
    }

    client.close();
}

int main()
{
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
        std::cout << "Esperando nuevo cliente...\n";

        ClientConnection client = server.accept();
        if (client.getSocket() != -1)
        {
            std::thread clientThread(handleClient, std::move(client), std::ref(grid), std::ref(server));
            clientThread.detach();
        }
    }

    return 0;
}
