#include "chess.h"
#include "validMoves.cpp"

// only restores the board, not g
void undoMove(std::string boardArray[8][8], int startX, int startY, int targetX, int targetY, PositionsMap& whitePositions, PositionsMap& blackPositions, GameData& g, const std::string& eaten, char pieceType) {
    std::string pieceStr = boardArray[targetY][targetX];
    char piece = pieceStr[1];
    char color = pieceStr[0];

    // Pawn promotion
    if (pieceType == 'p' && piece != 'p') {
        piece = 'p';
        pieceStr = std::string(1, color) + piece;
    }

    // En passant
    if (piece == 'p') {
        int y = targetY + (color == 'w' ? 1 : -1);
        if (abs(targetX - startX) == 1 && eaten.empty()) {
            // Restore the captured pawn
            std::string capturedPawn = std::string(1, (color == 'w' ? 'b' : 'w')) + 'p';
            boardArray[y][targetX] = capturedPawn;
            PositionsMap& positions = (color == 'b') ? whitePositions : blackPositions;
            positions[{targetX, y}] = 'p';
        }
    }

    // Check for castle
    if (piece == 'k' && abs(targetX - startX) > 1) {
        // Handle undo castling
        int rookStartX, rookStartY, rookTargetX, rookTargetY;
        std::string rookStr;
        if (color == 'w') {
            if (targetX == 6) {
                rookStartX = 7; rookStartY = 7; rookTargetX = 5; rookTargetY = 7;
            } else {
                rookStartX = 0; rookStartY = 7; rookTargetX = 3; rookTargetY = 7;
            }
            rookStr = "wr";
            deleteFromMap(whitePositions, {rookTargetX, rookTargetY});
            whitePositions[{rookStartX, rookStartY}] = 'r';
        } else {
            if (targetX == 6) {
                rookStartX = 7; rookStartY = 0; rookTargetX = 5; rookTargetY = 0;
            } else {
                rookStartX = 0; rookStartY = 0; rookTargetX = 3; rookTargetY = 0;
            }
            rookStr = "br";
            deleteFromMap(blackPositions, {rookTargetX, rookTargetY});
            blackPositions[{rookStartX, rookStartY}] = 'r';
        }
        boardArray[rookStartY][rookStartX] = rookStr;
        boardArray[rookTargetY][rookTargetX] = "";
    }

    // Update positions
    if (color == 'w') {
        whitePositions[{startX, startY}] = piece;
        deleteFromMap(whitePositions, {targetX, targetY});
        if (!eaten.empty()) {
            blackPositions[{targetX, targetY}] = eaten[1];
        }
    } else {
        blackPositions[{startX, startY}] = piece;
        deleteFromMap(blackPositions, {targetX, targetY});
        if (!eaten.empty()) {
            whitePositions[{targetX, targetY}] = eaten[1];
        }
    }

    // Update the board array
    boardArray[startY][startX] = pieceStr;
    boardArray[targetY][targetX] = eaten;
}

void movePiece(std::string boardArray[8][8], const Move& move, PositionsMap& whitePositions, PositionsMap& blackPositions, GameData& g) {
    int startX = move.startX;
    int startY = move.startY;
    int targetX = move.targetX;
    int targetY = move.targetY;

    std::string pieceStr = boardArray[startY][startX];
    char piece = pieceStr[1];
    char color = pieceStr[0];

    // Pawn promotion
    if (piece == 'p' && (targetY == 0 || targetY == 7)) {
        if (move.promotion != '\0') {
            piece = move.promotion;
            pieceStr = std::string(1, color) + piece;
        }
    }

    // En passant
    if (piece == 'p') {
        int y = targetY + (color == 'w' ? 1 : -1);
        if (abs(targetX - startX) == 1 && boardArray[targetY][targetX].empty()) {
            // Remove the captured pawn
            boardArray[y][targetX] = "";
            PositionsMap& positions = (color == 'b') ? whitePositions : blackPositions;
            deleteFromMap(positions, {targetX, y});
        }
        
        if (abs(targetY-startY) == 2) {
            g.prevX = targetX;
            // get the middle y
            g.prevY = (startY + targetY) / 2;
        } else {
            // clear en passant
            g.prevX = -2;
            g.prevY = -2;
        }
    } else {
        g.prevX = -2;
        g.prevY = -2;
    }

    // Update castling parameters
    if (piece == 'k') {
        if (color == 'w') {
            g.hasWhiteKingMoved = true;
        } else {
            g.hasBlackKingMoved = true;
        }
    } else if (piece == 'r') {
        if (startX == 0 && startY == 0) {
            g.hasBlackLeftRookMoved = true;
        } else if (startX == 0 && startY == 7) {
            g.hasWhiteLeftRookMoved = true;
        } else if (startX == 7 && startY == 0) {
            g.hasBlackRightRookMoved = true;
        } else if (startX == 7 && startY == 7) {
            g.hasWhiteRightRookMoved = true;
        }
    }

    // Check for castle
    if (piece == 'k' && abs(targetX - startX) > 1) {
        // Handle castling move
        int rookStartX, rookStartY, rookTargetX, rookTargetY;
        std::string rookStr;
        if (color == 'w') {
            if (targetX == 6) {
                rookStartX = 7; rookStartY = 7; rookTargetX = 5; rookTargetY = 7;
            } else {
                rookStartX = 0; rookStartY = 7; rookTargetX = 3; rookTargetY = 7;
            }
            rookStr = "wr";
            deleteFromMap(whitePositions, {rookStartX, rookStartY});
            whitePositions[{rookTargetX, rookTargetY}] = 'r';
        } else {
            if (targetX == 6) {
                rookStartX = 7; rookStartY = 0; rookTargetX = 5; rookTargetY = 0;
            } else {
                rookStartX = 0; rookStartY = 0; rookTargetX = 3; rookTargetY = 0;
            }
            rookStr = "br";
            deleteFromMap(blackPositions, {rookStartX, rookStartY});
            blackPositions[{rookTargetX, rookTargetY}] = 'r';
        }
        boardArray[rookTargetY][rookTargetX] = rookStr;
        boardArray[rookStartY][rookStartX] = "";
    }

    // Update positions
    if (color == 'w') {
        deleteFromMap(whitePositions, {startX, startY});
        whitePositions[{targetX, targetY}] = piece;
        deleteFromMap(blackPositions, {targetX, targetY});
    } else {
        deleteFromMap(blackPositions, {startX, startY});
        blackPositions[{targetX, targetY}] = piece;
        deleteFromMap(whitePositions, {targetX, targetY});
    }

    // Update the board array
    boardArray[targetY][targetX] = pieceStr;
    boardArray[startY][startX] = "";

    g.turn = (g.turn == 'w') ? 'b' : 'w';
}

void printBoard(std::string board[8][8], GameData& g) {
    std::cout << "Current Board:" << std::endl;
    for (int y = 0; y < 8; ++y) {
        std::cout << 8 - y << " ";
        for (int x = 0; x < 8; ++x) {
            std::string piece = board[y][x];
            if (piece != "") {
                std::cout << piece << " ";
            } else {
                std::cout << ".. ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "   a  b  c  d  e  f  g  h" << std::endl;
}

void setBoardFromFen(std::string board[8][8], const std::string fen, GameData& gg) {
    
    std::string fenStr(fen);
    std::vector<std::string> fenParts;
    std::istringstream iss(fenStr);
    std::string part;

    // Split the FEN string
    while (iss >> part) {
        fenParts.push_back(part);
    }

    if (fenParts.size() < 6) {
        // Handle error: FEN string does not have enough parts
        return;
    }

    const std::string& position = fenParts[0];
    const std::string& turnStr = fenParts[1];
    const std::string& castlingStr = fenParts[2];
    const std::string& enPassantStr = fenParts[3];
    const std::string& halfMoveClockStr = fenParts[4];
    const std::string& fullMoveNumberStr = fenParts[5];

    // The king has moved if neither kingside nor queenside castling is available
    gg.hasWhiteKingMoved = (castlingStr.find('K') == std::string::npos && castlingStr.find('Q') == std::string::npos);
    gg.hasWhiteRightRookMoved = (castlingStr.find('K') == std::string::npos);
    gg.hasWhiteLeftRookMoved = (castlingStr.find('Q') == std::string::npos);

    gg.hasBlackKingMoved = (castlingStr.find('k') == std::string::npos && castlingStr.find('q') == std::string::npos);
    gg.hasBlackRightRookMoved = (castlingStr.find('k') == std::string::npos);
    gg.hasBlackLeftRookMoved = (castlingStr.find('q') == std::string::npos);

    if (enPassantStr != "-") {
        gg.prevX = enPassantStr[0] - 'a';
        gg.prevY = 8 - (enPassantStr[1] - '0');
    } else {
        gg.prevX = -2;
        gg.prevY = -2;
    }

    // take first character of the turn string
    gg.turn = turnStr.c_str()[0];

    // Parse the board position from FEN
    int row = 0, col = 0;
        for (char c : position) {
        if (c == '/') {
            row++;
            col = 0;
        } else if (isdigit(c)) {
            col += c - '0';
        } else {
            std::string piece;
            if (isupper(c)) {
                // White piece
                piece = "w";
                piece += tolower(c);
            } else {
                // Black piece
                piece = "b";
                piece += c;
            }
            board[row][col] = piece;
            col++;
        }
    }
}