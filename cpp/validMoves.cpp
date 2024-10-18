#include "chess.h"

inline bool isBad(const std::string& piece) {
    return piece.empty();
}

bool checkColor(const std::string& piece, char color) {
    if (isBad(piece)) return false;
    return piece[0] == color;
}

bool checkPiece(const std::string& piece, char pieceType) {
    if (isBad(piece)) return false;
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
    char oppColor = (color == 'w') ? 'b' : 'w';
    int direction = (color == 'w') ? -1 : 1;
    int startRow = (color == 'w') ? 6 : 1;

    // Single forward move
    if (yPos + direction >= 0 && yPos + direction < 8 && isBad(board[yPos + direction][xPos])) {
        moves.push_back({xPos, yPos, xPos, yPos + direction});

        // Double forward move from starting position
        if (yPos == startRow && isBad(board[yPos + 2 * direction][xPos])) {
            moves.push_back({xPos, yPos, xPos, yPos + 2 * direction});
        }
    }

    // Captures
    for (int dx = -1; dx <= 1; dx += 2) {
        int x = xPos + dx;
        int y = yPos + direction;
        if (x >= 0 && x < 8 && y >= 0 && y < 8) {
            if (checkColor(board[y][x], oppColor)) {
                moves.push_back({xPos, yPos, x, y});
            }
        }
    }

    // En passant
    // we only update en passant for opponent so no color check needed
    if (g.prevX != -1 && g.prevY != -1) {
        if (abs(g.prevX - xPos) == 1 && abs(g.prevY - yPos) == 1) {
            // if (yPos == (color == 'w' ? 3 : 4)) {
            //     int targetY = yPos + direction;
            //     moves.push_back({xPos, yPos, g.prevX, targetY});
            // }
            moves.push_back({xPos, yPos, g.prevX, g.prevY});
        }
    }

    // Pawn promotion
    std::vector<Move> promotionMoves;
    for (auto& move : moves) {
        if (move.targetY == 0 || move.targetY == 7) {
            for (char promo : {'q', 'r', 'b', 'n'}) {
                Move promoMove = move;
                promoMove.promotion = promo;
                promotionMoves.push_back(promoMove);
            }
        } else {
            move.promotion = '\0';
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
            if (board[yPos][0] == "wr" && isBad(board[yPos][1]) && isBad(board[yPos][2]) && isBad(board[yPos][3]) && !isInCheck(board, color, {3, yPos})) {
                moves.push_back({4, yPos, 2, yPos});
            }
        }
        // Kingside castling
        if (!g.hasWhiteRightRookMoved) {
            if (board[yPos][7] == "wr" && isBad(board[yPos][5]) && isBad(board[yPos][6]) && !isInCheck(board, color, {5, yPos})) {
                moves.push_back({4, yPos, 6, yPos});
            }
        }
    } else {
        // Queenside castling
        if (!g.hasBlackLeftRookMoved) {
            if (board[yPos][0] == "br" && isBad(board[yPos][1]) && isBad(board[yPos][2]) && isBad(board[yPos][3]) && !isInCheck(board, color, {3, yPos})) {
                moves.push_back({4, yPos, 2, yPos});
            }
        }
        // Kingside castling
        if (!g.hasBlackRightRookMoved) {
            if (board[yPos][7] == "br" && isBad(board[yPos][5]) && isBad(board[yPos][6]) && !isInCheck(board, color, {5, yPos})) {
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

std::vector<Move> getValidMovesWhite(std::string board[8][8], PositionsMap& whitePositions, GameData& g) {
    std::vector<Move> moves;
    for (const auto& [key, value] : whitePositions) {
        int x = key.first;
        int y = key.second;
        std::string pieceStr = "w" + std::string(1, value);
        std::vector<Move> pieceMoves = getValidMoves(board, pieceStr, x, y, whitePositions, g);
        moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
    }
    return moves;
}

std::vector<Move> getValidMovesBlack(std::string board[8][8], PositionsMap& blackPositions, GameData& g) {
    std::vector<Move> moves;
    for (const auto& [key, value] : blackPositions) {
        int x = key.first;
        int y = key.second;
        std::string pieceStr = "b" + std::string(1, value);
        std::vector<Move> pieceMoves = getValidMoves(board, pieceStr, x, y, blackPositions, g);
        moves.insert(moves.end(), pieceMoves.begin(), pieceMoves.end());
    }
    return moves;
}
