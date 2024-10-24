#include "gtest/gtest.h"
#include "../chess.cpp" // Ensure this path is correct or replace with the appropriate header
#include <chrono>
#include <iostream>
#include <filesystem>
#include <fstream> // Added for std::ofstream
#include <vector>
#include <string>
#include <map>

namespace fs = std::filesystem;
#define ITERATIONS 5
typedef std::chrono::high_resolution_clock::time_point timePoint;
int board_count = 0;
timePoint start; 
std::ofstream file; 

void ensure_temp_dir_exists() {
    const std::string tempDir = "temp";
    if (!fs::exists(tempDir)) {
        fs::create_directory(tempDir);
        std::cout << "Created directory: " << tempDir << std::endl;
    }
}

void open_perft_record_file(int initial_depth) {
    ensure_temp_dir_exists();
    std::string filename = "temp/perft_depth" + std::to_string(initial_depth) + "_temp.csv";
    
    bool file_exists = fs::exists(filename);
    
    // open in apped mode
    file.open(filename, std::ios::app);
    
    if (!file.is_open()) {
        std::cerr << "Failed to open file: " << filename << std::endl;
        return;
    }
    
    // if just created, add the header
    if (!file_exists) {
        file << "BoardIndex,TimePerBoard_microseconds\n";
    }
}

void perft_record(std::string board[8][8], 
                PositionsMap& whitePositions, 
                PositionsMap& blackPositions, 
                int depth, 
                GameData& g, 
                int initial_depth) {
    
    if (board_count >= 2000 || depth == 0) return;

    int loop_count = 1;
    double avg = 0;
    if (depth == 1) { loop_count = ITERATIONS; }

    for (int i=0; i < loop_count; i++) {

        if (depth == 1) {
            if (!file.is_open()) {
                open_perft_record_file(initial_depth);
            }
            start = std::chrono::high_resolution_clock::now();
        }

        std::vector<Move> possibleMoves;
        PositionsMap positions = (g.turn == 'w') ? whitePositions : blackPositions;
        possibleMoves = getValidMovesOfCurrentTurn(board, blackPositions, g);


        if (possibleMoves.empty()) return;

        for (const auto& move : possibleMoves) {

            std::string eaten = board[move.targetY][move.targetX];
            GameData g_copy = g;

            char pieceType = board[move.startY][move.startX][1];

            movePiece(board, move, whitePositions, blackPositions, g);

            perft_record(board, whitePositions, blackPositions, depth - 1, g, initial_depth);

            g = g_copy;
            undoMove(board, move.startX, move.startY, move.targetX, move.targetY,
                    whitePositions, blackPositions, g, eaten, pieceType);
        }

        if (depth == 1) {
            timePoint end = std::chrono::high_resolution_clock::now();
            auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end-start).count();
            avg += static_cast<double>(duration);
        }
    }

    if (depth == 1) {
        file << board_count << "," << (avg/ITERATIONS) << "\n";
        board_count++;
    }
}

std::string fileName;
char response = '`';

void write_perft_record (const std::string& fen, int depth) {
    std::string board[8][8];

    if (file.is_open()) {
        file.close();
        std::cout << "Closed previously open file." << std::endl;
    }

    for (int y = 0; y < 8; ++y) {
        for (int x = 0; x < 8; ++x) {
            board[y][x] = "";
        }
    }

    GameData gg;

    setBoardFromFen(board, fen, gg);

    PositionsMap whitePositions;
    PositionsMap blackPositions;

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

    board_count = 0;
    perft_record(board, whitePositions, blackPositions, depth, gg, depth);

    if (response == '`') {
        std::cout << "Do you want to record performance for these runs? (y/n): ";
        std::cin >> response;
    }

    std::string source_file = "temp/perft_depth" + std::to_string(depth) + "_temp.csv";

    // if y is pressed move the file to records
    if (response == 'y' || response == 'Y') {

        if (fileName.empty()) {
            std::cout << "New record name: ";
            std::cin >> fileName;
        }

        const std::string recordsDir = "records";
        if (!fs::exists(recordsDir)) {
            fs::create_directory(recordsDir);
            std::cout << "Created directory: " << recordsDir << std::endl;
        }

        if (file.is_open()) {
            file.close();
            std::cout << "Closed file before moving." << std::endl;
        }

        auto now = std::chrono::system_clock::now();
        std::time_t now_time = std::chrono::system_clock::to_time_t(now);
        std::tm now_tm;
        #ifdef _WIN32
            localtime_s(&now_tm, &now_time);
        #else
             localtime_r(&now_time, &now_tm);
        #endif

        char time_buffer[20];
        std::strftime(time_buffer, sizeof(time_buffer), "%Y-%m-%d_%H-%M-%S", &now_tm);
        std::string timestamp(time_buffer);

        std::string new_filename = "records/" + fileName + "_" + std::to_string(depth) + "_" + timestamp + ".csv";

        std::string dest_file = new_filename;

        try {
            fs::rename(source_file, dest_file);
            std::cout << "Performance data moved to " << dest_file << std::endl;
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error moving file: " << e.what() << std::endl;
        }
    } else {
        // if n is pressed delete temp file
        try {
            if (fs::exists(source_file)) {
                fs::remove(source_file);
                std::cout << "Deleted temporary file: " << source_file << std::endl;
            } else {
                std::cout << "Temporary file does not exist: " << source_file << std::endl;
            }
        } catch (const fs::filesystem_error& e) {
            std::cerr << "Error deleting temporary file: " << e.what() << std::endl;
        }

        std::cout << "Performance data not saved and temporary file deleted." << std::endl;
    }
}

int perft(std::string board[8][8], PositionsMap& whitePositions, PositionsMap& blackPositions, int depth, GameData& g, int initial_depth) {

    std::vector<Move> possibleMoves;
    PositionsMap positions = (g.turn == 'w') ? whitePositions : blackPositions;
    possibleMoves = getValidMovesOfCurrentTurn(board, positions, g);

    if (depth == 0) return 1;
    if (possibleMoves.size() == 0) return 0;
    
    int nodes = 0;

    for (const auto& move : possibleMoves) {
 
        std::string eaten = board[move.targetY][move.targetX];
        GameData g_copy = g;

        char pieceType = board[move.startY][move.startX][1];

        movePiece(board, move, whitePositions, blackPositions, g);

        int childNodes = perft(board, whitePositions, blackPositions, depth - 1, g, initial_depth);

        nodes += childNodes;
        
        g = g_copy;
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


// TEST(perft_record_2, depth_2) {
//     write_perft_record("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 2);
// }

// TEST(perft_record_2, depth_3) {
//     write_perft_record("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 3);
// }

// TEST(perft_record_2, depth_4) {
//     write_perft_record("r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1", 4);
// }

int main (int argc, char **argv) {
    ::testing::InitGoogleTest(&argc, argv);
    // run only this test
    // ::testing::GTEST_FLAG(filter) = "perft_board_3.depth_2";
    return RUN_ALL_TESTS();
}