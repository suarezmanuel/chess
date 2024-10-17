#include "main.h"
#include "validMoves.cpp"
using namespace std;

// only restores the board, not g
void undoMoveServer(string boardArray[8][8], int startX, int startY, int targetX, int targetY, PositionsMap& whitePositions, PositionsMap& blackPositions, GameData& g, const std::string& eaten, char pieceType) {
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

void movePieceServer(string boardArray[8][8], const Move& move, PositionsMap& whitePositions, PositionsMap& blackPositions, GameData& g) {
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
            g.prevX = -1;
            g.prevY = -1;
        }
    } else {
        g.prevX = -1;
        g.prevY = -1;
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

void printBoard(string board[8][8], GameData& g) {
    std::cout << "Current Board:" << std::endl;
    for (int y = 0; y < 8; ++y) {
        std::cout << 8 - y << " ";
        for (int x = 0; x < 8; ++x) {
            string piece = board[y][x];
            if (piece != "") {
                std::cout << piece << " ";
            } else {
                std::cout << ".. ";
            }
        }
        std::cout << std::endl;
    }
    std::cout << "   a  b  c  d  e  f  g  h" << std::endl;

    std::cout << "white king moved: " << g.hasWhiteKingMoved << std::endl;
    std::cout << "white right rook moved: " << g.hasWhiteRightRookMoved << std::endl;
    std::cout << "white left rook moved: " << g.hasWhiteLeftRookMoved << std::endl;

    std::cout << "black king moved: " << g.hasBlackKingMoved << std::endl;
    std::cout << "black right rook moved: " << g.hasBlackRightRookMoved << std::endl;
    std::cout << "black left rook moved: " << g.hasBlackLeftRookMoved << std::endl;

    std::cout << "prev x: " << g.prevX << std::endl;
    std::cout << "prev y: " << g.prevY << std::endl; 
    std::cout << "turn: " << g.turn << std::endl;
}

// int base_depth = 3;
int help(string board[8][8], PositionsMap& whitePositions, PositionsMap& blackPositions, int depth, GameData& g, int initial_depth) {

    std::vector<Move> possibleMoves;
    if (g.turn == 'w') {
        possibleMoves = getValidMovesWhite(board, whitePositions, g);
    } else {
        possibleMoves = getValidMovesBlack(board, blackPositions, g);
    }

    if (depth == 0) {
        return 1;
    }
    if (possibleMoves.size() == 0) {
        return 0;
    }
    

    int nodes = 0;

    for (const auto& move : possibleMoves) {
        // if (depth == initial_depth && (move.startX != 1 || move.startY != 1 || move.targetX != 1 || move.targetY != 3)) continue;
        // printBoard(board);
        std::string eaten = board[move.targetY][move.targetX];
        GameData g_copy = g;
        // BoardType board_copy = board;

        char pieceType = board[move.startY][move.startX][1];

        movePieceServer(board, move, whitePositions, blackPositions, g);

        int childNodes = help(board, whitePositions, blackPositions, depth - 1, g, initial_depth);

        nodes += childNodes;

        if (depth == initial_depth) {
            char fromFile = 'a' + move.startX;
            char fromRank = '1' + (7 - move.startY);
            char toFile = 'a' + move.targetX;
            char toRank = '1' + (7 - move.targetY);

            std::string promotionStr;
            if (move.promotion != '\0') {
                promotionStr += move.promotion;
            }

            // printBoard(board, g);
            std::cout << fromFile << fromRank << toFile << toRank << promotionStr
                      << ": " << childNodes << std::endl;
        }

        // Undo move
        g = g_copy;
        // board = board_copy;
        undoMoveServer(board, move.startX, move.startY, move.targetX, move.targetY,
                       whitePositions, blackPositions, g, eaten, pieceType);
    }

    return nodes;
}

extern "C" {

    // Function to change the board at a specific position
    void change_board(char* board[8][8]) {
        // Place "wk" at position [4][4]
        if (board[4][4]) {
            // free(board[4][4]);
        }
        board[4][4] = strdup("wk");

        // Print the board (optional)
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                std::cout << (board[i][j] ? board[i][j] : "empty") << " ";
            }
            std::cout << std::endl;
        }
    }
    // Function to set the board from FEN notation
    void setBoardFromFenPrint(char* board[8][8], const char* fen, char* turn, char* castling, char* enPassant, char* halfMoveClock, char* fullMoveNumber, GameData gg) {
        
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

        // used for printing to screen
        strcpy(turn, turnStr.c_str());
        strcpy(castling, castlingStr.c_str());
        strcpy(enPassant, enPassantStr.c_str());
        strcpy(halfMoveClock, halfMoveClockStr.c_str());
        strcpy(fullMoveNumber, fullMoveNumberStr.c_str());

        // used for actual game logic
        gg.hasWhiteKingMoved = (castlingStr.find('K') == std::string::npos);
        gg.hasWhiteRightRookMoved = (castlingStr.find('K') == std::string::npos);
        gg.hasWhiteLeftRookMoved = (castlingStr.find('Q') == std::string::npos);

        gg.hasBlackKingMoved = (castlingStr.find('k') == std::string::npos);
        gg.hasBlackRightRookMoved = (castlingStr.find('k') == std::string::npos);
        gg.hasBlackLeftRookMoved = (castlingStr.find('q') == std::string::npos);

        if (enPassantStr != "-") {
            gg.prevX = enPassantStr[0] - 'a';
            gg.prevY = 8 - (enPassantStr[1] - '0');
        } else {
            gg.prevX = -1;
            gg.prevY = -1;
        }

        // take first character of the turn string
        gg.turn = turnStr.c_str()[0];

        // Clear the board
        for (int i = 0; i < 8; i++) {
            for (int j = 0; j < 8; j++) {
                if (board[i][j]) {
                    // free(board[i][j]);
                    board[i][j] = "";
                }
            }
        }

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
                board[row][col] = strdup(piece.c_str());
                col++;
            }
        }
    }

    void setBoardFromFen(string board[8][8], const string fen, GameData& gg) {
        
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
            gg.prevX = -1;
            gg.prevY = -1;
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

    extern "C" void perft(string board[8][8], int depth, GameData gg) {

        int initial_depth = depth;

        PositionsMap whitePositions;
        PositionsMap blackPositions;

        // Initialize positions from the board
        for (int y = 0; y < 8; ++y) {
            for (int x = 0; x < 8; ++x) {
                const std::string& pieceStr = board[y][x];
                if (!pieceStr.empty()) {
                    char color = pieceStr[0];
                    char piece = pieceStr[1];
                    if (color == 'w') {
                        whitePositions[{x, y}] = piece;
                    } else if (color == 'b') {
                        blackPositions[{x, y}] = piece;
                    }
                }
            }
        }

        int count = help(board, whitePositions, blackPositions, depth, gg, initial_depth);

        std::cout << "Perft result: " << count << " nodes at depth " << depth << std::endl;
    }

} // extern "C"

int main () {
    
    string board[8][8];

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            board[y][x] = "";
        }
    }

    GameData gg;

    setBoardFromFen(board, "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", gg);
    printBoard(board, gg);
    perft(board, 4, gg);
}