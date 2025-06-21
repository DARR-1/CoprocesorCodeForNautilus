#pragma once
#include <vector>
#include <utility>
#include <cfloat>

#pragma pack(push, 1)
struct Pair
{
    int first;
    int second;

    // Constructor necesario
    Pair(int f, int s) : first(f), second(s) {}

    // Constructor por defecto (necesario para vector<>, etc.)
    Pair() : first(0), second(0) {}
};
#pragma pack(pop)

typedef std::pair<double, std::pair<int, int>> pPair;

struct cell
{
    // Row and Column index of its parent
    // Note that 0 <= i <= ROW-1 & 0 <= j <= COL-1
    int parent_i, parent_j;
    // f = g + h
    double f, g, h;
};

constexpr double QUALITY = 0.5;
constexpr int ROW = static_cast<int>((634.0 / 2.0) * QUALITY) + 1;
constexpr int COL = static_cast<int>((1381.0 / 2.0) * QUALITY) + 1;

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