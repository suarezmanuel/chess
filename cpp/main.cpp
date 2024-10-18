#include "chess.cpp"

// int base_depth = 3;
int help(std::string board[8][8], PositionsMap& whitePositions, PositionsMap& blackPositions, int depth, GameData& g, int initial_depth) {

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
 
        std::string eaten = board[move.targetY][move.targetX];
        GameData g_copy = g;

        char pieceType = board[move.startY][move.startX][1];

        movePiece(board, move, whitePositions, blackPositions, g);

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
            // std::cout << fromFile << fromRank << toFile << toRank << promotionStr
            //           << ": " << childNodes << std::endl;
        }

        // Undo move
        g = g_copy;
        // board = board_copy;
        undoMove(board, move.startX, move.startY, move.targetX, move.targetY,
                       whitePositions, blackPositions, g, eaten, pieceType);
    }

    return nodes;
}

int perft(std::string board[8][8], int depth, GameData gg) {

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

    return help(board, whitePositions, blackPositions, depth, gg, initial_depth);

    // std::cout << "Perft result: " << count << " nodes at depth " << depth << std::endl;
}

int main () {
    
    std::string board[8][8];

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            board[y][x] = "";
        }
    }

    GameData gg;

    setBoardFromFen(board, "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", gg);
    // printBoard(board, gg);
    // std::cout << "rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8" << std::endl << std::endl;
    std::cout << perft(board, 1, gg) << std::endl;
    //48
}