#include "gtest/gtest.h"
#include "main.cpp"

void test_perft (std::string fen, int depth, int expected_val) {
    std::string board[8][8];

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            board[y][x] = "";
        }
    }

    GameData gg;

    setBoardFromFen(board, fen, gg);
    EXPECT_EQ(perft(board, depth, gg), expected_val);
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
    test_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 1, 48);
}

TEST(perft_board_2, depth_2) {
    test_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 2, 2039);
}

TEST(perft_board_2, depth_3) {
    test_perft("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - ", 3, 97862);
}


TEST(perft_board_3, depth_1) {
    test_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 1, 14);
}

TEST(perft_board_3, depth_2) {
    test_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 2, 191);
}

TEST(perft_board_3, depth_3) {
    test_perft("8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - ", 3, 2812);
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
    test_perft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 1, 44);
}

TEST(perft_board_5, depth_3) {
    test_perft("rnbq1k1r/pp1Pbppp/2p5/8/2B5/8/PPP1NnPP/RNBQK2R w KQ - 1 8", 1, 44);
}

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}