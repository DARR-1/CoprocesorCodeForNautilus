#include <iostream>
#include <vector>
#include <cmath>
#include "astar.h"
#include "Server.h"
#include <fstream>
#include <chrono>
#include <opencv2/opencv.hpp> // <-- nueva dependencia

using Grid = std::vector<std::vector<int>>;
using Path = std::vector<Pair>;

#define ROBOT_RADIUS 12

// Imprime el grid marcando obstáculos '#', libres '.', camino '*', origen 'S' y destino 'D'
void printGrid(const Grid &grid, const Path &path, Pair src, Pair dest)
{
    std::vector<std::string> vis(ROW, std::string(COL, '.'));

    for (int i = 0; i < ROW; ++i)
        for (int j = 0; j < COL; ++j)
            if (grid[i][j] == 0)
                vis[i][j] = '#';

    for (auto &p : path)
    {
        vis[p.first][p.second] = '*';
    }
    vis[src.first][src.second] = 'S';
    vis[dest.first][dest.second] = 'D';

    int maxR = ROW, maxC = COL;
    for (int i = 0; i < maxR; ++i)
    {
        std::cout << vis[i].substr(0, maxC) << "\n";
    }
    std::cout << "...\n\n";

    // --- guardamos el grid en grid.txt ---
    std::ofstream ofs("grid.txt");
    if (!ofs)
    {
        std::cerr << "Error al crear grid.txt\n";
    }
    else
    {
        for (int i = 0; i < maxR; ++i)
        {
            ofs << vis[i].substr(0, maxC) << "\n";
        }
        ofs.close();
        std::cout << "Grid guardado en grid.txt\n";
    }
    // ---------------------------------------
}

int main()
{
    auto t_start = std::chrono::high_resolution_clock::now();

    std::cout << "A* Pathfinding Algorithm\n";

    Grid grid(ROW, std::vector<int>(COL));

    for (int i = 0; i < ROW; ++i)
    {
        for (int j = 0; j < COL; ++j)
        {
            if ((abs(-j + (176.745 * 2)) <= (sqrt(3) * ((37.81 + ROBOT_RADIUS) * 2)) / 2 && sqrt(3) * abs(i - (158.5 * 2)) - j + (176.745 * 2) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * 2) && sqrt(3) * abs(i - (158.5 * 2)) + j - (176.745 * 2) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * 2)) ||
                (abs(-j + (-176.745 * 2 + 1381)) <= (sqrt(3) * ((37.81 + ROBOT_RADIUS) * 2)) / 2 && sqrt(3) * abs(i - (158.5 * 2)) - j + (-176.745 * 2 + 1381) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * 2) && sqrt(3) * abs(i - (158.5 * 2)) + j - (-176.745 * 2 + 1381) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * 2)))
            {
                grid[i][j] = 0; // Bloqueado
            }
            else
            {
                grid[i][j] = 1; // Libre
            }
        }
    }

    std::cout << "Grid initialized with dimensions: " << ROW << " x " << COL << std::endl;

    Pair src = {0, 0}, dest = {0, 0};
    auto path = aStarSearch(grid, src, dest);
    std::cout << "Pathfinding completed.\n";

    if (!path.empty())
    {
        std::cout << "Path from source to destination:\n";
        for (const auto &p : path)
        {
            // std::cout << "(" << p.first << ", " << p.second << ") ";
        }
        std::cout << std::endl;

        std::cout << "Imprimiendo vista parcial del grid:\n";
        printGrid(grid, path, src, dest);
    }
    else
    {
        std::cout << "No path found from source to destination.\n";
    }

    auto t_end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();
    std::cout << "Tiempo de ejecución total: " << elapsed << " ms\n";

    return 0;

    Server server(27015);
    server.initialize();
    server.listen();
    server.accept();
    system("pause");
    return server.close();
}
