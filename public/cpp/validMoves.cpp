#include <iostream>
#include <vector>
#include <map>
#include <string>
#include <algorithm>

struct Position {
    int x;
    int y;

    bool operator<(const Position& other) const {
        return std::tie(x, y) < std::tie(other.x, other.y);
    }
};

// Initial positions for white pieces
std::map<Position, char> whitePositions = {
    {{0, 6}, 'p'}, {{1, 6}, 'p'}, {{2, 6}, 'p'}, {{3, 6}, 'p'},
    {{4, 6}, 'p'}, {{5, 6}, 'p'}, {{6, 6}, 'p'}, {{7, 6}, 'p'},
    {{0, 7}, 'r'}, {{1, 7}, 'n'}, {{2, 7}, 'b'}, {{3, 7}, 'q'},
    {{4, 7}, 'k'}, {{5, 7}, 'b'}, {{6, 7}, 'n'}, {{7, 7}, 'r'}
};

// Initial positions for black pieces
std::map<Position, char> blackPositions = {
    {{0, 1}, 'p'}, {{1, 1}, 'p'}, {{2, 1}, 'p'}, {{3, 1}, 'p'},
    {{4, 1}, 'p'}, {{5, 1}, 'p'}, {{6, 1}, 'p'}, {{7, 1}, 'p'},
    {{0, 0}, 'r'}, {{1, 0}, 'n'}, {{2, 0}, 'b'}, {{3, 0}, 'q'},
    {{4, 0}, 'k'}, {{5, 0}, 'b'}, {{6, 0}, 'n'}, {{7, 0}, 'r'}
};

struct GameState {
    bool hasWhiteKingMoved = false;
    bool hasWhiteRightRookMoved = false;
    bool hasWhiteLeftRookMoved = false;

    bool hasBlackKingMoved = false;
    bool hasBlackRightRookMoved = false;
    bool hasBlackLeftRookMoved = false;

    int prevX = -1;
    int prevY = -1;
};

GameState g;

// Direction vectors for different pieces
std::vector<std::pair<int, int>> rookDirections = {{0, -1}, {1, 0}, {0, 1}, {-1, 0}};
std::vector<std::pair<int, int>> bishopDirections = {{1, -1}, {1, 1}, {-1, 1}, {-1, -1}};
std::vector<std::pair<int, int>> knightDirections = {{1, -2}, {2, -1}, {2, 1}, {1, 2},
                                                     {-1, 2}, {-2, 1}, {-2, -1}, {-1, -2}};
std::vector<std::pair<int, int>> kingDirections = {{0, -1}, {1, 0}, {0, 1}, {-1, 0},
                                                   {1, -1}, {1, 1}, {-1, 1}, {-1, -1}};

// Function declarations
std::vector<std::vector<int>> getValidMovesWhite(std::vector<std::vector<std::string>>& board);
std::vector<std::vector<int>> getValidMovesBlack(std::vector<std::vector<std::string>>& board);
std::vector<std::vector<int>> validMovesFromArray(std::vector<std::vector<std::string>>& board, int startX, int startY, const std::vector<std::vector<int>>& array);
bool isBad(const std::string& a);
std::vector<std::vector<int>> getValidMoves(std::vector<std::vector<std::string>>& board, const std::string& piece, int xPos, int yPos);
bool checkColor(const std::string& piece, char color);
bool checkPiece(const std::string& piece, char pieceType);
std::vector<std::vector<int>> pawnMoves(std::vector<std::vector<std::string>>& board, char color, int xPos, int yPos);
std::vector<std::vector<int>> rookMoves(std::vector<std::vector<std::string>>& board, char color, int xPos, int yPos);
std::vector<std::vector<int>> knightMoves(std::vector<std::vector<std::string>>& board, char color, int xPos, int yPos);
std::vector<std::vector<int>> bishopMoves(std::vector<std::vector<std::string>>& board, char color, int xPos, int yPos);
std::vector<std::vector<int>> queenMoves(std::vector<std::vector<std::string>>& board, char color, int xPos, int yPos);
std::vector<std::vector<int>> kingMoves(std::vector<std::vector<std::string>>& board, char color, int xPos, int yPos);
std::vector<std::vector<int>> castleMoves(std::vector<std::vector<std::string>>& board, char color);
bool isInCheck(std::vector<std::vector<std::string>>& board, char color, Position kingPos);

bool isBad(const std::string& a) {
    return a.empty();
}

bool checkColor(const std::string& piece, char color) {
    if (isBad(piece)) return false;
    return piece[0] == color;
}

bool checkPiece(const std::string& piece, char pieceType) {
    if (isBad(piece)) return false;
    return piece[1] == pieceType;
}

// Implementations
std::vector<std::vector<int>> getValidMovesWhite(std::vector<std::vector<std::string>>& board) {
    std::vector<std::vector<int>> moves;
    for (const auto& entry : whitePositions) {
        Position pos = entry.first;
        char value = entry.second;
        std::string piece = "w";
        piece += value;
        std::vector<std::vector<int>> a = getValidMoves(board, piece, pos.x, pos.y);
        std::vector<std::vector<int>> validMoves = validMovesFromArray(board, pos.x, pos.y, a);
        moves.insert(moves.end(), validMoves.begin(), validMoves.end());
    }
    return moves;
}

std::vector<std::vector<int>> getValidMovesBlack(std::vector<std::vector<std::string>>& board) {
    std::vector<std::vector<int>> moves;
    for (const auto& entry : blackPositions) {
        Position pos = entry.first;
        char value = entry.second;
        std::string piece = "b";
        piece += value;
        std::vector<std::vector<int>> a = getValidMoves(board, piece, pos.x, pos.y);
        std::vector<std::vector<int>> validMoves = validMovesFromArray(board, pos.x, pos.y, a);
        moves.insert(moves.end(), validMoves.begin(), validMoves.end());
    }
    return moves;
}

std::vector<std::vector<int>> validMovesFromArray(std::vector<std::vector<std::string>>& board, int startX, int startY, const std::vector<std::vector<int>>& array) {
    char color = board[startY][startX][0];
    char piece = board[startY][startX][1];

    std::map<Position, char>& positions = (color == 'w') ? whitePositions : blackPositions;

    Position kingPos;

    for (const auto& entry : positions) {
        if (entry.second == 'k') {
            kingPos = entry.first;
            break;
        }
    }

    std::vector<std::vector<int>> moves;

    for (const auto& move : array) {
        std::string temp = board[move[3]][move[2]];

        // start
        board[startY][startX] = "";

        // target
        board[move[3]][move[2]] = std::string(1, color) + piece;

        if (piece == 'k') {
            kingPos.x = move[2];
            kingPos.y = move[3];
        }

        if (!isInCheck(board, color, kingPos)) {
            // Check if move is already in moves
            bool alreadyExists = false;
            for (const auto& m : moves) {
                if (m[0] == move[2] && m[1] == move[3]) {
                    alreadyExists = true;
                    break;
                }
            }
            if (!alreadyExists) {
                moves.push_back(move);
            }
        }

        // Undo the move
        board[startY][startX] = std::string(1, color) + piece;
        if (piece == 'k') {
            kingPos.x = startX;
            kingPos.y = startY;
        }
        board[move[3]][move[2]] = temp;
    }
    return moves;
}

std::vector<std::vector<int>> getValidMoves(std::vector<std::vector<std::string>>& board, const std::string& piece, int xPos, int yPos) {
    char pieceType = piece[1]; // e.g., 'p' for pawn, 'r' for rook, etc.
    std::vector<std::vector<int>> moves;
    switch (pieceType) {
        case 'p': // Pawn move
            moves = pawnMoves(board, piece[0], xPos, yPos);
            break;
        case 'r': // Rook move
            moves = rookMoves(board, piece[0], xPos, yPos);
            break;
        case 'n': // Knight move
            moves = knightMoves(board, piece[0], xPos, yPos);
            break;
        case 'b': // Bishop move
            moves = bishopMoves(board, piece[0], xPos, yPos);
            break;
        case 'q': // Queen move
            moves = queenMoves(board, piece[0], xPos, yPos);
            break;
        case 'k': // King move
            {
                std::vector<std::vector<int>> king_moves = kingMoves(board, piece[0], xPos, yPos);
                std::vector<std::vector<int>> castle_moves = castleMoves(board, piece[0]);
                king_moves.insert(king_moves.end(), castle_moves.begin(), castle_moves.end());
                moves = king_moves;
            }
            break;
    }
    return moves;
}

// Pawn moves including en passant
std::vector<std::vector<int>> pawnMoves(std::vector<std::vector<std::string>>& board, char color, int xPos, int yPos) {
    std::vector<std::vector<int>> moves;
    char oppColor = (color == 'w') ? 'b' : 'w';

    int direction = (color == 'w') ? -1 : 1;
    int startRow = (color == 'w') ? 6 : 1;

    // Forward moves
    int newY = yPos + direction;
    if (newY >= 0 && newY < 8 && isBad(board[newY][xPos])) {
        moves.push_back({xPos, yPos, xPos, newY});
        // Double move from starting position
        if (yPos == startRow && isBad(board[yPos + 2 * direction][xPos])) {
            moves.push_back({xPos, yPos, xPos, yPos + 2 * direction});
        }
    }

    // Captures
    for (int dx = -1; dx <= 1; dx += 2) {
        int newX = xPos + dx;
        if (newX >= 0 && newX < 8 && newY >= 0 && newY < 8) {
            if (checkColor(board[newY][newX], oppColor)) {
                moves.push_back({xPos, yPos, newX, newY});
            }
        }
    }

    // En passant
    if (g.prevX != -1 && g.prevY != -1 && checkPiece(board[g.prevY][g.prevX], 'p')) {
        if (abs(g.prevY - yPos) == 0 && abs(g.prevX - xPos) == 1) {
            int enPassantRow = (color == 'w') ? 3 : 4;
            if (yPos == enPassantRow) {
                moves.push_back({xPos, yPos, g.prevX, yPos + direction});
            }
        }
    }

    return moves;
}

// Rook moves
std::vector<std::vector<int>> rookMoves(std::vector<std::vector<std::string>>& board, char color, int xPos, int yPos) {
    std::vector<std::vector<int>> moves;
    char oppColor = (color == 'w') ? 'b' : 'w';

    for (const auto& dir : rookDirections) {
        int xTemp = xPos;
        int yTemp = yPos;
        while (true) {
            xTemp += dir.first;
            yTemp += dir.second;
            if (xTemp < 0 || xTemp >= 8 || yTemp < 0 || yTemp >= 8) break;
            if (checkColor(board[yTemp][xTemp], color)) break;
            moves.push_back({xPos, yPos, xTemp, yTemp});
            if (checkColor(board[yTemp][xTemp], oppColor)) break;
        }
    }

    return moves;
}

// Knight moves
std::vector<std::vector<int>> knightMoves(std::vector<std::vector<std::string>>& board, char color, int xPos, int yPos) {
    std::vector<std::vector<int>> moves;
    char oppColor = (color == 'w') ? 'b' : 'w';

    for (const auto& dir : knightDirections) {
        int xTemp = xPos + dir.first;
        int yTemp = yPos + dir.second;
        if (xTemp < 0 || xTemp >= 8 || yTemp < 0 || yTemp >= 8) continue;
        if (checkColor(board[yTemp][xTemp], color)) continue;
        moves.push_back({xPos, yPos, xTemp, yTemp});
    }

    return moves;
}

// Bishop moves
std::vector<std::vector<int>> bishopMoves(std::vector<std::vector<std::string>>& board, char color, int xPos, int yPos) {
    std::vector<std::vector<int>> moves;
    char oppColor = (color == 'w') ? 'b' : 'w';

    for (const auto& dir : bishopDirections) {
        int xTemp = xPos;
        int yTemp = yPos;
        while (true) {
            xTemp += dir.first;
            yTemp += dir.second;
            if (xTemp < 0 || xTemp >= 8 || yTemp < 0 || yTemp >= 8) break;
            if (checkColor(board[yTemp][xTemp], color)) break;
            moves.push_back({xPos, yPos, xTemp, yTemp});
            if (checkColor(board[yTemp][xTemp], oppColor)) break;
        }
    }

    return moves;
}

// Queen moves (combination of rook and bishop)
std::vector<std::vector<int>> queenMoves(std::vector<std::vector<std::string>>& board, char color, int xPos, int yPos) {
    std::vector<std::vector<int>> moves = rookMoves(board, color, xPos, yPos);
    std::vector<std::vector<int>> bishop_moves = bishopMoves(board, color, xPos, yPos);
    moves.insert(moves.end(), bishop_moves.begin(), bishop_moves.end());
    return moves;
}

// King moves
std::vector<std::vector<int>> kingMoves(std::vector<std::vector<std::string>>& board, char color, int xPos, int yPos) {
    std::vector<std::vector<int>> moves;
    char oppColor = (color == 'w') ? 'b' : 'w';

    for (const auto& dir : kingDirections) {
        int xTemp = xPos + dir.first;
        int yTemp = yPos + dir.second;
        if (xTemp < 0 || xTemp >= 8 || yTemp < 0 || yTemp >= 8) continue;
        if (checkColor(board[yTemp][xTemp], color)) continue;
        moves.push_back({xPos, yPos, xTemp, yTemp});
    }

    return moves;
}

// Castling moves
std::vector<std::vector<int>> castleMoves(std::vector<std::vector<std::string>>& board, char color) {
    std::vector<std::vector<int>> moves;
    bool hasKingMoved = (color == 'w') ? g.hasWhiteKingMoved : g.hasBlackKingMoved;
    int row = (color == 'w') ? 7 : 0;

    if (hasKingMoved || isInCheck(board, color, {4, row})) return moves;

    if (color == 'w') {
        if (!g.hasWhiteLeftRookMoved) {
            if (board[row][0] == "wr" && isBad(board[row][1]) && isBad(board[row][2]) && isBad(board[row][3]) && !isInCheck(board, color, {3, row}))
                moves.push_back({4, row, 2, row});
        }
        if (!g.hasWhiteRightRookMoved) {
            if (board[row][7] == "wr" && isBad(board[row][5]) && isBad(board[row][6]) && !isInCheck(board, color, {5, row}))
                moves.push_back({4, row, 6, row});
        }
    } else {
        if (!g.hasBlackLeftRookMoved) {
            if (board[row][0] == "br" && isBad(board[row][1]) && isBad(board[row][2]) && isBad(board[row][3]) && !isInCheck(board, color, {3, row}))
                moves.push_back({4, row, 2, row});
        }
        if (!g.hasBlackRightRookMoved) {
            if (board[row][7] == "br" && isBad(board[row][5]) && isBad(board[row][6]) && !isInCheck(board, color, {5, row}))
                moves.push_back({4, row, 6, row});
        }
    }

    return moves;
}

// Check if the king is in check
bool isInCheck(std::vector<std::vector<std::string>>& board, char color, Position kingPos) {
    int xPos = kingPos.x;
    int yPos = kingPos.y;

    char oppColor = (color == 'w') ? 'b' : 'w';

    // Knight threats
    std::vector<std::vector<int>> knight_moves = knightMoves(board, color, xPos, yPos);
    for (const auto& move : knight_moves) {
        if (checkColor(board[move[3]][move[2]], oppColor) && checkPiece(board[move[3]][move[2]], 'n')) {
            return true;
        }
    }

    // Rook and Queen threats
    std::vector<std::vector<int>> rook_moves = rookMoves(board, color, xPos, yPos);
    for (const auto& move : rook_moves) {
        if (checkColor(board[move[3]][move[2]], oppColor) &&
            (checkPiece(board[move[3]][move[2]], 'r') || checkPiece(board[move[3]][move[2]], 'q'))) {
            return true;
        }
    }

    // Bishop and Queen threats
    std::vector<std::vector<int>> bishop_moves = bishopMoves(board, color, xPos, yPos);
    for (const auto& move : bishop_moves) {
        if (checkColor(board[move[3]][move[2]], oppColor) &&
            (checkPiece(board[move[3]][move[2]], 'b') || checkPiece(board[move[3]][move[2]], 'q'))) {
            return true;
        }
    }

    // King threats
    std::vector<std::vector<int>> king_moves = kingMoves(board, color, xPos, yPos);
    for (const auto& move : king_moves) {
        if (checkColor(board[move[3]][move[2]], oppColor) && checkPiece(board[move[3]][move[2]], 'k')) {
            return true;
        }
    }

    // Pawn threats
    int direction = (color == 'w') ? -1 : 1;
    for (int dx = -1; dx <= 1; dx += 2) {
        int x = xPos + dx;
        int y = yPos + direction;
        if (x >= 0 && x < 8 && y >= 0 && y < 8) {
            if (checkColor(board[y][x], oppColor) && checkPiece(board[y][x], 'p')) {
                return true;
            }
        }
    }

    return false;
}

int main() {
    // Initialize the board as an 8x8 vector of strings
    std::vector<std::vector<std::string>> board(8, std::vector<std::string>(8, ""));

    // Place white pieces
    for (const auto& entry : whitePositions) {
        Position pos = entry.first;
        char piece = entry.second;
        board[pos.y][pos.x] = "w";
        board[pos.y][pos.x] += piece;
    }

    // Place black pieces
    for (const auto& entry : blackPositions) {
        Position pos = entry.first;
        char piece = entry.second;
        board[pos.y][pos.x] = "b";
        board[pos.y][pos.x] += piece;
    }

    // Example usage: get valid moves for white
    std::vector<std::vector<int>> whiteMoves = getValidMovesWhite(board);

    // Output the moves
    for (const auto& move : whiteMoves) {
        std::cout << "From (" << move[0] << ", " << move[1] << ") to (" << move[2] << ", " << move[3] << ")\n";
    }

    return 0;
}
