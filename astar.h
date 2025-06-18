#pragma once
#include <vector>
#include <utility>
#include <cfloat>

typedef std::pair<int, int> Pair;
typedef std::pair<double, std::pair<int, int>> pPair;

struct cell
{
    // Row and Column index of its parent
    // Note that 0 <= i <= ROW-1 & 0 <= j <= COL-1
    int parent_i, parent_j;
    // f = g + h
    double f, g, h;
};
#define QUALITY 1 / 2
#define ROW ((634 / 2) * QUALITY) + 1
#define COL ((1381 / 2) * QUALITY) + 1

class AStar
{
public:
    AStar() = default;
    ~AStar() = default;

private:
    // Helper functions
    bool isValid(int row, int col);
    bool isUnBlocked(const std::vector<std::vector<int>> &grid, int row, int col);
    bool isDestination(int row, int col, Pair dest);
    double calculateHValue(int row, int col, Pair dest);
    std::vector<Pair> tracePath(const std::vector<std::vector<cell>> &cellDetails,
                                Pair dest);
};

std::vector<Pair> aStarSearch(const std::vector<std::vector<int>> &grid, Pair src, Pair dest);