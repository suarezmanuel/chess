#include "chess.h"


bool checkColor(const std::string& piece, char color) {
    if (piece.empty()) return false;
    return piece[0] == color;
}

bool checkPiece(const std::string& piece, char pieceType) {
    if (piece.empty()) return false;
    return piece[1] == pieceType;
}

void deleteFromMap(PositionsMap& positions, const std::pair<int, int>& key) {
    positions.erase(key);
}

bool isInCheck(std::string board[8][8], char color, const std::pair<int, int>& kingPos) {
    int xPos = kingPos.first;
    int yPos = kingPos.second;
    char oppColor = (color == 'w') ? 'b' : 'w';

    // Knight attacks
    std::vector<Move> knightAttacks;
    for (const auto& dir : knightDirections) {
        int x = xPos + dir.first;
        int y = yPos + dir.second;
        if (x >= 0 && x < 8 && y >= 0 && y < 8) {
            if (checkColor(board[y][x], oppColor) && checkPiece(board[y][x], 'n')) {
                return true;
            }
        }
    }

    // Rook and Queen attacks (horizontal and vertical)
    for (const auto& dir : rookDirections) {
        int x = xPos;
        int y = yPos;
        while (true) {
            x += dir.first;
            y += dir.second;
            if (x < 0 || x >= 8 || y < 0 || y >= 8) break;
            if (checkColor(board[y][x], color)) break;
            if (checkColor(board[y][x], oppColor)) {
                if (checkPiece(board[y][x], 'r') || checkPiece(board[y][x], 'q')) {
                    return true;
                } else {
                    break;
                }
            }
        }
    }

    // Bishop and Queen attacks (diagonals)
    for (const auto& dir : bishopDirections) {
        int x = xPos;
        int y = yPos;
        while (true) {
            x += dir.first;
            y += dir.second;
            if (x < 0 || x >= 8 || y < 0 || y >= 8) break;
            if (checkColor(board[y][x], color)) break;
            if (checkColor(board[y][x], oppColor)) {
                if (checkPiece(board[y][x], 'b') || checkPiece(board[y][x], 'q')) {
                    return true;
                } else {
                    break;
                }
            }
        }
    }

    // King attacks
    for (const auto& dir : kingDirections) {
        int x = xPos + dir.first;
        int y = yPos + dir.second;
        if (x >= 0 && x < 8 && y >= 0 && y < 8) {
            if (checkColor(board[y][x], oppColor) && checkPiece(board[y][x], 'k')) {
                return true;
            }
        }
    }

    // Pawn attacks
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

std::vector<Move> validMovesFromArray(std::string board[8][8], int startX, int startY, const std::vector<Move>& movesArray, PositionsMap& positions, char color, GameData& g) {
    std::string pieceStr = board[startY][startX];
    // char color = pieceStr[0];
    char piece = pieceStr[1];
    std::pair<int, int> kingPos;
    for (const auto& [key, value] : positions) {
        if (value == 'k') {
            kingPos = key;
            break;
        }
    }

    std::vector<Move> validMoves;

    for (const auto& move : movesArray) {

        // handle en passant
        int flag = move.targetX == g.prevX && move.targetY == g.prevY && piece == 'p';
        int diffX = move.targetX - startX;

        int eatenY = move.targetY*(1-flag) + startY*flag;
        int eatenX = move.targetX*(1-flag) + (startX + diffX)*flag;

        std::string temp = board[move.targetY][move.targetX];
        std::string temp2 = board[eatenY][eatenX];
        board[eatenY][eatenX] = "";

        // Make the move
        board[startY][startX] = "";
        board[move.targetY][move.targetX] = pieceStr;

        std::pair<int, int> newKingPos = kingPos;
        if (piece == 'k') {
            newKingPos = {move.targetX, move.targetY};
        }

        if (!isInCheck(board, color, newKingPos)) {
            validMoves.push_back(move);
        }

        // Undo the move
        board[startY][startX] = std::string(1, color) + piece;
        board[move.targetY][move.targetX] = temp;
        board[eatenY][eatenX] = temp2;
    }

    return validMoves;
}

std::vector<Move> pawnMoves(std::string board[8][8], char color, int xPos, int yPos, GameData& g) {
    std::vector<Move> moves;
    bool isWhite = color == 'w';
    char oppColor; int direction; int startRow;
    
    if (isWhite) {
        oppColor = 'b';
        direction = -1;
        startRow = 6;
    } else {
        oppColor = 'w';
        direction = 1;
        startRow = 1;
    }
    
    int yNew = yPos+direction;
    // Single forward move
    if (yNew >= 0 && yNew < 8 && board[yNew][xPos].empty()) {
        moves.push_back({xPos, yPos, xPos, yNew});

        // if you can't move once you can't move twice
        yNew += direction;
        // it will be in bounds because its on a starting row
        if (yPos == startRow && board[yNew][xPos].empty()) {
            moves.push_back({xPos, yPos, xPos, yNew});
        }
    }

    // eating pieces normally
    int x = xPos - 1;
    int y = yPos + direction;
    if (y >= 0 && y < 8) {
        if (x >= 0 && x < 8 && checkColor(board[y][x], oppColor)) {
            moves.push_back({xPos, yPos, x, y});
        }
        x = xPos + 1;
        if (x >= 0 && x < 8 && checkColor(board[y][x], oppColor)) {
            moves.push_back({xPos, yPos, x, y});
        }
    }

    // eating pieces en passant
    // prevX, prevY are -2 if uninitialized so we need no check
    if (abs(g.prevX - xPos) == 1 && abs(g.prevY - yPos) == 1) {
        moves.push_back({xPos, yPos, g.prevX, g.prevY});
    }

    // Pawn promotion
    std::vector<Move> promotionMoves;
    for (auto& move : moves) {
        if (move.targetY == 0 || move.targetY == 7) {
            promotionMoves.push_back({move.startX, move.startY, move.targetX, move.targetY, 'q'});
            promotionMoves.push_back({move.startX, move.startY, move.targetX, move.targetY, 'r'});
            promotionMoves.push_back({move.startX, move.startY, move.targetX, move.targetY, 'b'});
            promotionMoves.push_back({move.startX, move.startY, move.targetX, move.targetY, 'n'});
        } else {
            promotionMoves.push_back(move);
        }
    }

    return promotionMoves;
}

std::vector<Move> rookMoves(std::string board[8][8], char color, int xPos, int yPos) {
    std::vector<Move> moves;
    char oppColor = (color == 'w') ? 'b' : 'w';

    for (const auto& dir : rookDirections) {
        int x = xPos;
        int y = yPos;
        while (true) {
            x += dir.first;
            y += dir.second;
            if (x < 0 || x >= 8 || y < 0 || y >= 8) break;
            if (checkColor(board[y][x], color)) break;
            moves.push_back({xPos, yPos, x, y});
            if (checkColor(board[y][x], oppColor)) break;
        }
    }
    return moves;
}

std::vector<Move> knightMoves(std::string board[8][8], char color, int xPos, int yPos) {
    std::vector<Move> moves;
    char oppColor = (color == 'w') ? 'b' : 'w';

    for (const auto& dir : knightDirections) {
        int x = xPos + dir.first;
        int y = yPos + dir.second;
        if (x >= 0 && x < 8 && y >= 0 && y < 8) {
            if (!checkColor(board[y][x], color)) {
                moves.push_back({xPos, yPos, x, y});
            }
        }
    }
    return moves;
}

std::vector<Move> bishopMoves(std::string board[8][8], char color, int xPos, int yPos) {
    std::vector<Move> moves;
    char oppColor = (color == 'w') ? 'b' : 'w';

    for (const auto& dir : bishopDirections) {
        int x = xPos;
        int y = yPos;
        while (true) {
            x += dir.first;
            y += dir.second;
            if (x < 0 || x >= 8 || y < 0 || y >= 8) break;
            if (checkColor(board[y][x], color)) break;
            moves.push_back({xPos, yPos, x, y});
            if (checkColor(board[y][x], oppColor)) break;
        }
    }
    return moves;
}

std::vector<Move> queenMoves(std::string board[8][8], char color, int xPos, int yPos) {
    std::vector<Move> moves = rookMoves(board, color, xPos, yPos);
    std::vector<Move> bishopMovesList = bishopMoves(board, color, xPos, yPos);
    moves.insert(moves.end(), bishopMovesList.begin(), bishopMovesList.end());
    return moves;
}

std::vector<Move> castleMoves(std::string board[8][8], char color, GameData& g) {
    std::vector<Move> moves;
    int yPos = (color == 'w') ? 7 : 0;
    bool hasKingMoved = (color == 'w') ? g.hasWhiteKingMoved : g.hasBlackKingMoved;
    if (hasKingMoved || isInCheck(board, color, {4, yPos})) return moves;

    if (color == 'w') {
        // Queenside castling
        if (!g.hasWhiteLeftRookMoved) {
            if (board[yPos][0] == "wr" && board[yPos][1].empty() && board[yPos][2].empty() && board[yPos][3].empty() && !isInCheck(board, color, {3, yPos})) {
                moves.push_back({4, yPos, 2, yPos});
            }
        }
        // Kingside castling
        if (!g.hasWhiteRightRookMoved) {
            if (board[yPos][7] == "wr" && board[yPos][5].empty() && board[yPos][6].empty() && !isInCheck(board, color, {5, yPos})) {
                moves.push_back({4, yPos, 6, yPos});
            }
        }
    } else {
        // Queenside castling
        if (!g.hasBlackLeftRookMoved) {
            if (board[yPos][0] == "br" && board[yPos][1].empty() && board[yPos][2].empty() && board[yPos][3].empty() && !isInCheck(board, color, {3, yPos})) {
                moves.push_back({4, yPos, 2, yPos});
            }
        }
        // Kingside castling
        if (!g.hasBlackRightRookMoved) {
            if (board[yPos][7] == "br" && board[yPos][5].empty() && board[yPos][6].empty() && !isInCheck(board, color, {5, yPos})) {
                moves.push_back({4, yPos, 6, yPos});
            }
        }
    }
    return moves;
}

std::vector<Move> kingMoves(std::string board[8][8], char color, int xPos, int yPos, GameData& g) {
    std::vector<Move> moves;
    char oppColor = (color == 'w') ? 'b' : 'w';

    // Normal king moves
    for (const auto& dir : kingDirections) {
        int x = xPos + dir.first;
        int y = yPos + dir.second;
        if (x >= 0 && x < 8 && y >= 0 && y < 8) {
            if (!checkColor(board[y][x], color)) {
                moves.push_back({xPos, yPos, x, y});
            }
        }
    }

    // Castling
    std::vector<Move> castlingMoves = castleMoves(board, color, g);
    moves.insert(moves.end(), castlingMoves.begin(), castlingMoves.end());

    return moves;
}

std::vector<Move> getValidMoves(std::string board[8][8], const std::string& piece, int xPos, int yPos, PositionsMap& positions, GameData& g) {
    char pieceType = piece[1];
    char color = piece[0];
    std::vector<Move> moves;

    switch (pieceType) {
        case 'p':
            moves = pawnMoves(board, color, xPos, yPos, g);
            break;
        case 'r':
            moves = rookMoves(board, color, xPos, yPos);
            break;
        case 'n':
            moves = knightMoves(board, color, xPos, yPos);
            break;
        case 'b':
            moves = bishopMoves(board, color, xPos, yPos);
            break;
        case 'q':
            moves = queenMoves(board, color, xPos, yPos);
            break;
        case 'k':
            moves = kingMoves(board, color, xPos, yPos, g);
            break;
    }

    // Filter moves that don't put the king in check
    moves = validMovesFromArray(board, xPos, yPos, moves, positions, color, g);

    return moves;
}


std::vector<Move> getValidMovesOfCurrentTurn(std::string board[8][8], PositionsMap& positions, GameData& g) {
    std::vector<Move> moves;
    for (const auto& [key, value] : positions) {
        int x = key.first;
        int y = key.second;
        std::string pieceStr = g.turn + std::string(1, value);
        std::vector<Move> pieceMoves = getValidMoves(board, pieceStr, x, y, positions, g);
        moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
    }
    return moves;
}