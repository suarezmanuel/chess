#ifndef HLLO 

#define HLLO

#include <cstring>
#include <cstdlib>
#include <iostream>
#include <vector>
#include <string>
#include <algorithm>
#include <sstream>
#include <map>

struct GameData {
    bool hasWhiteKingMoved = false;
    bool hasWhiteRightRookMoved = false;
    bool hasWhiteLeftRookMoved = false;

    bool hasBlackKingMoved = false;
    bool hasBlackRightRookMoved = false;
    bool hasBlackLeftRookMoved = false;

    int prevX = -2;
    int prevY = -2;

    char turn = 'w';
};

// typedef std::vector<std::vector<std::string>> BoardType;
typedef std::map<std::pair<int, int>, char> PositionsMap;

struct Move {
    int startX;
    int startY;
    int targetX;
    int targetY;
    char promotion ='\0';  // '\0' if no promotion
};

PositionsMap whitePositions = {
    {{0, 6}, 'p'}, {{1, 6}, 'p'}, {{2, 6}, 'p'}, {{3, 6}, 'p'},
    {{4, 6}, 'p'}, {{5, 6}, 'p'}, {{6, 6}, 'p'}, {{7, 6}, 'p'},
    {{0, 7}, 'r'}, {{1, 7}, 'n'}, {{2, 7}, 'b'}, {{3, 7}, 'q'},
    {{4, 7}, 'k'}, {{5, 7}, 'b'}, {{6, 7}, 'n'}, {{7, 7}, 'r'}
};

PositionsMap blackPositions = {
    {{0, 1}, 'p'}, {{1, 1}, 'p'}, {{2, 1}, 'p'}, {{3, 1}, 'p'},
    {{4, 1}, 'p'}, {{5, 1}, 'p'}, {{6, 1}, 'p'}, {{7, 1}, 'p'},
    {{0, 0}, 'r'}, {{1, 0}, 'n'}, {{2, 0}, 'b'}, {{3, 0}, 'q'},
    {{4, 0}, 'k'}, {{5, 0}, 'b'}, {{6, 0}, 'n'}, {{7, 0}, 'r'}
};

const std::vector<std::pair<int, int>> rookDirections = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
const std::vector<std::pair<int, int>> bishopDirections = {{1, -1}, {1, 1}, {-1, 1}, {-1, -1}};
const std::vector<std::pair<int, int>> knightDirections = {{1, -2}, {2, -1}, {2, 1}, {1, 2}, {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}};
const std::vector<std::pair<int, int>> kingDirections = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}, {1, -1}, {1, 1}, {-1, 1}, {-1, -1}};

#endif