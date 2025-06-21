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

#define ROBOT_RADIUS 17

using Grid = std::vector<std::vector<int>>;
using Path = std::vector<Pair>;

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
            std::thread clientThread(&Server::handleClient, &server, std::move(client), std::ref(grid));
            clientThread.detach();
        }
    }

    return 0;
}
