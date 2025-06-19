#include <iostream>
#include <vector>
#include <cmath>
#include "astar.h"
#include "Server.h"
#include <fstream>
#include <chrono>

using Grid = std::vector<std::vector<int>>;
using Path = std::vector<Pair>;

#define ROBOT_RADIUS 17

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

// Función para guardar rutas en binario
void savePaths(const std::vector<Path> &paths, const std::string &filename)
{
    std::ofstream ofs(filename, std::ios::binary);
    uint32_t r = ROW, c = COL;
    ofs.write(reinterpret_cast<const char *>(&r), sizeof(r));
    ofs.write(reinterpret_cast<const char *>(&c), sizeof(c));
    uint64_t total = paths.size();
    ofs.write(reinterpret_cast<const char *>(&total), sizeof(total));
    for (auto &path : paths)
    {
        uint32_t len = path.size();
        ofs.write(reinterpret_cast<const char *>(&len), sizeof(len));
        for (auto &p : path)
        {
            ofs.write(reinterpret_cast<const char *>(&p.first), sizeof(p.first));
            ofs.write(reinterpret_cast<const char *>(&p.second), sizeof(p.second));
        }
    }
}

// Función para cargar rutas previamente guardadas
bool loadPaths(std::vector<Path> &paths, const std::string &filename)
{
    std::ifstream ifs(filename, std::ios::binary);
    if (!ifs)
        return false;
    uint32_t r, c;
    ifs.read(reinterpret_cast<char *>(&r), sizeof(r));
    ifs.read(reinterpret_cast<char *>(&c), sizeof(c));
    if (r != ROW || c != COL)
        return false; // grid distinto
    uint64_t total;
    ifs.read(reinterpret_cast<char *>(&total), sizeof(total));
    paths.clear();
    paths.reserve(total);
    for (uint64_t i = 0; i < total; ++i)
    {
        uint32_t len;
        ifs.read(reinterpret_cast<char *>(&len), sizeof(len));
        Path path(len);
        for (uint32_t j = 0; j < len; ++j)
        {
            ifs.read(reinterpret_cast<char *>(&path[j].first), sizeof(path[j].first));
            ifs.read(reinterpret_cast<char *>(&path[j].second), sizeof(path[j].second));
        }
        paths.push_back(std::move(path));
    }
    return true;
}

int main()
{

    std::cout << "A* Pathfinding Algorithm\n";

    Grid grid(ROW, std::vector<int>(COL));

    for (int i = 0; i < ROW; ++i)
    {
        for (int j = 0; j < COL; ++j)
        {
            if ((abs(-j + (176.745 * QUALITY)) <= (sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY)) / 2 && sqrt(3) * abs(i - (158.5 * QUALITY)) - j + (176.745 * QUALITY) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY) && sqrt(3) * abs(i - (158.5 * QUALITY)) + j - (176.745 * QUALITY) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY)) ||
                (abs(-j + (-176.745 * QUALITY + (COL - 1))) <= (sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY)) / 2 && sqrt(3) * abs(i - (158.5 * QUALITY)) - j + (-176.745 * QUALITY + (COL - 1)) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY) && sqrt(3) * abs(i - (158.5 * QUALITY)) + j - (-176.745 * QUALITY + (COL - 1)) <= sqrt(3) * ((37.81 + ROBOT_RADIUS) * QUALITY)) ||
                (j > ((COL - 1) / 2) - (6 + ROBOT_RADIUS) * QUALITY && j < ((COL - 1) / 2) + (6 + ROBOT_RADIUS) * QUALITY && i > ((ROW - 1) / 2) - (6 + ROBOT_RADIUS) * QUALITY && i < ((ROW - 1) / 2) + (6 + ROBOT_RADIUS) * QUALITY))
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

    Pair src, dest;

    std::vector<std::vector<Pair>> paths;
    const std::string cacheFile = "paths.dat";
    /*
    if (!loadPaths(paths, cacheFile))
    {
        std::cout << "Generating paths for all pairs of source and destination...\n";
        // Calculamos el total de pares y marcamos el inicio del bucle
        long long totalPairs = static_cast<long long>(ROW) * COL * ROW * COL;
        auto loopStart = std::chrono::high_resolution_clock::now();

        // 1. Destino:
        //  COL
        //  ROW
        // 2. Origen:
        //  COL
        //  ROW
        //    source_index = OrigenRow * COL + OrigenCol
        //    dest_index   = DestinoRow * COL + DestinoCol
        //    id = source_index * (ROW * COL) + dest_index
        for (int i = 0; i < ROW; ++i)
        {
            for (int j = 0; j < COL; ++j)
            {
                for (int id = 0; id < ROW; ++id)
                {
                    for (int jd = 0; jd < COL; ++jd)
                    {
                        std::vector<Pair> path = aStarSearch(grid, {i, j}, {id, jd});

                        if (!path.empty())
                        {
                            paths.push_back(path);
                        }
                        else
                        {
                            paths.push_back({});
                        }
                        auto now = std::chrono::high_resolution_clock::now();
                        auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(now - loopStart).count();
                        double avgPerPath = double(elapsed) / uint64_t(paths.size());
                        uint64_t remMs = (uint64_t(COL) * uint64_t(COL) * uint64_t(ROW) * uint64_t(ROW) - uint64_t(paths.size())) * avgPerPath;

                        system("cls"); // Limpiar pantalla
                        std::cout << "Generating paths for all pairs of source and destination...\n";
                        std::cout << "Completed path " << uint64_t(paths.size()) << " of " << uint64_t(COL) * uint64_t(COL) * uint64_t(ROW) * uint64_t(ROW) << ". Estimated remaining: " << remMs / 3600000 << " hrs\n";
                    }
                }
            }
        }
        std::cout << "Paths generated for all pairs of source and destination.\n";
        std::cout << "Total paths generated: " << paths.size() << "\n";
    auto end = std::chrono::high_resolution_clock::now();
    auto pathElapsed = std::chrono::duration_cast<std::chrono::milliseconds>(end - loopStart).count();
    std::cout << "Time taken to generate paths: " << pathElapsed << " ms\n";

    // Guardar rutas en archivo binario
    savePaths(paths, cacheFile);
    std::cout << "Paths guardados en " << cacheFile << "\n";
}
else
{
    std::cout << "Paths cargados desde " << cacheFile << "\n";
}
*/
    // auto t_start = std::chrono::high_resolution_clock::now();

    src = {0, 0};
    dest = {ROW - 1, (COL - 1) / 2};
    /*
    // índices a 64 bits
    uint64_t src_idx = uint64_t(src.first) * uint64_t(COL) + uint64_t(src.second);
    uint64_t dest_idx = uint64_t(dest.first) * uint64_t(COL) + uint64_t(dest.second);
    uint64_t total_rc = uint64_t(ROW) * uint64_t(COL);
    uint64_t flat_index = src_idx * total_rc + dest_idx;

    std::cout << "src_idx=" << src_idx
              << ", dest_idx=" << dest_idx
              << ", ROW*COL=" << total_rc
              << ", flat_index=" << flat_index
              << "\n";

    if (flat_index >= paths.size())
    {
        std::cerr << "Error: índice fuera de rango: " << flat_index << "\n";
        return 1;
    }
    */

    /*
    std::vector<Pair> path = aStarSearch(grid, src, dest);

    if (!path.empty())
    {
        std::cout << "Path found from source to destination:\n";
        for (const auto &p : path)
        {
            std::cout << "(" << p.first << ", " << p.second << ") ";
        }
        std::cout << "\n";

        // Imprimir vista parcial del grid
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
    */

    Server server(27015);
    server.initialize();
    server.listen();
    server.accept();

    char buffer[200] = "";
    server.receive(buffer, 200);
    std::cout << "Received from client: " << buffer << std::endl;

    auto t_start = std::chrono::high_resolution_clock::now();

    if (strcmp(buffer, "pathfind") == 0)
    {
        server.send("ok", 200);

        std::cout << "Pathfinding request received.\n";
        server.receive(buffer, 200);
        std::cout << "Received source x: " << buffer << std::endl;
        src = {0, round(atof(buffer) * 2)};
        server.send("ok", 200);

        server.receive(buffer, 200);
        std::cout << "Received source y: " << buffer << std::endl;
        src = {ROW - round(atof(buffer) * 2), 0};
        server.send("ok", 200);

        server.receive(buffer, 200);
        std::cout << "Received destination x: " << buffer << std::endl;
        dest = {0, round(atof(buffer) * 2)};
        server.send("ok", 200);

        server.receive(buffer, 200);
        std::cout << "Received destination y: " << buffer << std::endl;
        dest = {ROW - round(atof(buffer) * 2), 0};
        server.send("ok", 200);

        auto path = aStarSearch(grid, src, dest);
        std::cout << "Pathfinding completed.\n";

        if (!path.empty())
        {
            std::cout << "Path from source to destination:\n";
            for (const auto &p : path)
            {
                // std::cout << "(" << p.first << ", " << p.second << ") ";
                server.send(std::to_string(p.first).c_str(), 200);

                std::cout << "Esperando confirmación del cliente...\n";
                while (strcmp(buffer, "ok") != 0)
                {
                    server.receive(buffer, 200);
                }
                std::cout << "Confirmación recibida.\n";
                buffer[0] = '\0'; // Limpiar buffer

                server.send(std::to_string(p.second).c_str(), 200);

                std::cout << "Esperando confirmación del cliente...\n";
                while (strcmp(buffer, "ok") != 0)
                {
                    server.receive(buffer, 200);
                }
                std::cout << "Confirmación recibida.\n";
            }
            server.send("end", 200);

            std::cout << "Esperando confirmación del cliente...\n";
            while (strcmp(buffer, "end") != 0)
            {
                server.receive(buffer, 200);
            }
            std::cout << "Path sent to client successfully.\n";

            // std::cout << "Imprimiendo vista parcial del grid:\n";
            // printGrid(grid, path, src, dest);
        }
        else
        {
            std::cout << "No path found from source to destination.\n";
        }
    }
    else
    {
        // Handle other cases if needed
    }
    auto t_end = std::chrono::high_resolution_clock::now();
    auto elapsed = std::chrono::duration_cast<std::chrono::milliseconds>(t_end - t_start).count();

    std::cout << "Tiempo de ejecución total: " << elapsed << " ms\n";
    return server.close();
}
