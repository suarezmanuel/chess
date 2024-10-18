#include "gtest/gtest.h"
#include "../chess.cpp"

int perft(std::string board[8][8], PositionsMap& whitePositions, PositionsMap& blackPositions, int depth, GameData& g, int initial_depth) {

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

        int childNodes = perft(board, whitePositions, blackPositions, depth - 1, g, initial_depth);

        nodes += childNodes;

        // if (depth == initial_depth) {
        //     char fromFile = 'a' + move.startX;
        //     char fromRank = '1' + (7 - move.startY);
        //     char toFile = 'a' + move.targetX;
        //     char toRank = '1' + (7 - move.targetY);

        //     std::string promotionStr;
        //     if (move.promotion != '\0') {
        //         promotionStr += move.promotion;
        //     }

        //     // printBoard(board, g);
        //     std::cout << fromFile << fromRank << toFile << toRank << promotionStr
        //               << ": " << childNodes << std::endl;
        // }

        // Undo move
        g = g_copy;
        // board = board_copy;
        undoMove(board, move.startX, move.startY, move.targetX, move.targetY,
                       whitePositions, blackPositions, g, eaten, pieceType);
    }

    return nodes;
}

void test_perft (std::string fen, int depth, int expected_val) {
    std::string board[8][8];

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            board[y][x] = "";
        }
    }

    GameData gg;

    setBoardFromFen(board, fen, gg);

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

    EXPECT_EQ(perft(board, whitePositions, blackPositions, depth, gg, depth), expected_val);
}


TEST(perft_board_1, depth_1) {
    test_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ", 1, 20);
}

TEST(perft_board_1, depth_2) {
    test_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ", 2, 400);
}

TEST(perft_board_1, depth_3) {
    test_perft("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 ", 3, 8902);
}


TEST(perft_board_2, depth_1) {
    test_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 1, 48);
}

TEST(perft_board_2, depth_2) {
    test_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 2, 2039);
}

TEST(perft_board_2, depth_3) {
    test_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 3, 97862);
}


TEST(perft_board_3, depth_1) {
    test_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 1, 14);
}

TEST(perft_board_3, depth_2) {
    test_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 2, 191);
    // test_perft("8/2p5/3p4/KP5r/1R2Pp1k/8/6P1/8 b - e3 0 1", 1, 16);
}

TEST(perft_board_3, depth_3) {
    test_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 1", 3, 2812);
}


TEST(perft_board_4, depth_1) {
    test_perft("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 1, 6);
}

TEST(perft_board_4, depth_2) {
    test_perft("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 2, 264);
}

TEST(perft_board_4, depth_3) {
    test_perft("r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1", 3, 9467);
}


TEST(perft_board_5, depth_1) {
    test_perft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 1, 44);
}

TEST(perft_board_5, depth_2) {
    test_perft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 2, 1486);
}

TEST(perft_board_5, depth_3) {
    test_perft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 3, 62379);
}


TEST(perft_board_6, depth_1) {
    test_perft("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 1, 46);
}

TEST(perft_board_6, depth_2) {
    test_perft("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 2, 2079);
}

TEST(perft_board_6, depth_3) {
    test_perft("r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10", 3, 89890);
}


int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // run only this test
    // ::testing::GTEST_FLAG(filter) = "perft_board_3.depth_2";
    return RUN_ALL_TESTS();
}