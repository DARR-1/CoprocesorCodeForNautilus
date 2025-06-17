// A C++ Program to implement A* Search Algorithm
#include <cmath>
#include <algorithm>
#include <set>
#include "astar.h"

using namespace std;

bool isUnBlocked(const vector<vector<int>> &grid, int row, int col)
{
    return grid[row][col] == 1;
}

// A Utility Function to check whether given cell (row, col)
// is a valid cell or not.
bool isValid(int row, int col)
{
    // Returns true if row number and column number
    // is in range
    return (row >= 0) && (row < ROW) && (col >= 0) && (col < COL);
}

// A Utility Function to check whether destination cell has
// been reached or not
bool isDestination(int row, int col, Pair dest)
{
    if (row == dest.first && col == dest.second)
        return (true);
    else
        return (false);
}

// A Utility Function to calculate the 'h' heuristics.
double calculateHValue(int row, int col, Pair dest)
{
    // Return using the distance formula
    return ((double)sqrt(
        (row - dest.first) * (row - dest.first) + (col - dest.second) * (col - dest.second)));
}

// Nuevo tracePath que devuelve vector<Pair>
vector<Pair> tracePath(const vector<vector<cell>> &cellDetails, Pair dest)
{
    vector<Pair> Path;
    int row = dest.first, col = dest.second;
    // Retrocedemos padres hasta llegar al origen
    while (!(cellDetails[row][col].parent_i == row && cellDetails[row][col].parent_j == col))
    {
        Path.emplace_back(row, col);
        int pi = cellDetails[row][col].parent_i;
        int pj = cellDetails[row][col].parent_j;
        row = pi;
        col = pj;
    }
    Path.emplace_back(row, col);
    reverse(Path.begin(), Path.end());
    return Path;
}

// A Function to find the shortest path between
// a given source cell to a destination cell according
// to A* Search Algorithm
vector<Pair> aStarSearch(const vector<vector<int>> &grid, Pair src, Pair dest)
{
    if (!isValid(src.first, src.second) || !isValid(dest.first, dest.second))
        return {};

    if (!isUnBlocked(grid, src.first, src.second) || !isUnBlocked(grid, dest.first, dest.second))
        return {};

    if (isDestination(src.first, src.second, dest))
        return {src};

    // Reemplazamos el closedList estático por un vector dinámico
    vector<vector<bool>> closedList(ROW, vector<bool>(COL, false));
    // Reemplazamos cellDetails estático por vector dinámico
    vector<vector<cell>> cellDetails(ROW, vector<cell>(COL));

    // Inicializar cellDetails
    for (int i = 0; i < ROW; ++i)
        for (int j = 0; j < COL; ++j)
        {
            cellDetails[i][j].f = FLT_MAX;
            cellDetails[i][j].g = FLT_MAX;
            cellDetails[i][j].h = FLT_MAX;
            cellDetails[i][j].parent_i = -1;
            cellDetails[i][j].parent_j = -1;
        }

    // Parámetros del nodo de inicio
    int i = src.first, j = src.second;
    cellDetails[i][j].f = cellDetails[i][j].g = cellDetails[i][j].h = 0.0f;
    cellDetails[i][j].parent_i = i;
    cellDetails[i][j].parent_j = j;

    set<pPair> openList;
    openList.insert({0.0, {i, j}});

    while (!openList.empty())
    {
        auto p = *openList.begin();
        openList.erase(openList.begin());
        i = p.second.first;
        j = p.second.second;
        closedList[i][j] = true;

        // Repetir para cada sucesor (norte, sur, ..., sur-oeste)...
        // Cuando lleguemos a destino, llamamos a tracePath:
        if (isDestination(i, j, dest))
        {
            return tracePath(cellDetails, dest);
        }

        static const vector<pair<int, int>> directions = {
            {-1, 0}, {1, 0}, {0, -1}, {0, 1}, {-1, -1}, {-1, 1}, {1, -1}, {1, 1}};

        for (auto [dr, dc] : directions)
        {
            int nr = i + dr, nc = j + dc;
            if (!isValid(nr, nc) || closedList[nr][nc] || !isUnBlocked(grid, nr, nc))
                continue;

            if (isDestination(nr, nc, dest))
            {
                cellDetails[nr][nc].parent_i = i;
                cellDetails[nr][nc].parent_j = j;
                return tracePath(cellDetails, dest);
            }

            double gNew = cellDetails[i][j].g + ((dr == 0 || dc == 0) ? 1.0 : 1.414);
            double hNew = calculateHValue(nr, nc, dest);
            double fNew = gNew + hNew;

            if (cellDetails[nr][nc].f > fNew)
            {
                openList.insert({fNew, {nr, nc}});
                cellDetails[nr][nc].f = fNew;
                cellDetails[nr][nc].g = gNew;
                cellDetails[nr][nc].h = hNew;
                cellDetails[nr][nc].parent_i = i;
                cellDetails[nr][nc].parent_j = j;
            }
        }
    }

    // No se encontró camino
    return {};
}

// Driver program to test above function