// #include "bbc.h"
#include "evaluation.h"
// #include "board.h"

// extactly matches PNBRQKpnbrqk
// in order to index by the piece enum
const int material_score[2][12] = {
    // opening
    { 82, 337, 365, 477, 1025, 12000, -82, -337, -365, -477, -1025, -12000 },
    // endgame
    { 94, 281, 297, 512,  936, 12000, -94, -281, -297, -512,  -936, -12000 }
};

const int opening_phase_score = 6192;
const int endgame_phase_score = 518;

enum {opening, endgame, middlegame};
enum {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};

const int positional_score[2][6][64] = {
    {
    //pawn
    {
    0,   0,   0,   0,   0,   0,  0,   0,
    98, 134,  61,  95,  68, 126, 34, -11,
    -6,   7,  26,  31,  65,  56, 25, -20,
    -14,  13,   6,  21,  23,  12, 17, -23,
    -27,  -2,  -5,  12,  17,   6, 10, -25,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -35,  -1, -20, -23, -15,  24, 38, -22,
    0,   0,   0,   0,   0,   0,  0,   0,
    },

    // knight
    {
    -167, -89, -34, -49,  61, -97, -15, -107,
    -73, -41,  72,  36,  23,  62,   7,  -17,
    -47,  60,  37,  65,  84, 129,  73,   44,
    -9,  17,  19,  53,  37,  69,  18,   22,
    -13,   4,  16,  13,  28,  19,  21,   -8,
    -23,  -9,  12,  10,  19,  17,  25,  -16,
    -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23,
    },

    // bishop
    {
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
    -4,   5,  19,  50,  37,  37,   7,  -2,
    -6,  13,  13,  26,  34,  12,  10,   4,
    0,  15,  15,  15,  14,  27,  18,  10,
    4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
    },

    // rook
    {
    32,  42,  32,  51, 63,  9,  31,  43,
    27,  32,  58,  62, 80, 67,  26,  44,
    -5,  19,  26,  36, 17, 45,  61,  16,
    -24, -11,   7,  26, 24, 35,  -8, -20,
    -36, -26, -12,  -1,  9, -7,   6, -23,
    -45, -25, -16, -17,  3,  0,  -5, -33,
    -44, -16, -20,  -9, -1, 11,  -6, -71,
    -19, -13,   1,  17, 16,  7, -37, -26,
    },

    // queen
    {
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
    -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
    -1, -18,  -9,  10, -15, -25, -31, -50,
    },

    // king
    {
    -65,  23,  16, -15, -56, -34,   2,  13,
    29,  -1, -20,  -7,  -8,  -4, -38, -29,
    -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
    1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,
}

    },
    // Endgame positional piece scores //
    {
    //pawn
    {
    0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
    94, 100,  85,  67,  56,  53,  82,  84,
    32,  24,  13,   5,  -2,   4,  17,  17,
    13,   9,  -3,  -7,  -7,  -8,   3,  -1,
    4,   7,  -6,   1,   0,  -5,  -1,  -8,
    13,   8,   8,  10,  13,   0,   2,  -7,
    0,   0,   0,   0,   0,   0,   0,   0,
    },

    // knight
    {
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,
    },

    // bishop
    {
    -14, -21, -11,  -8, -7,  -9, -17, -24,
    -8,  -4,   7, -12, -3, -13,  -4, -14,
    2,  -8,   0,  -1, -2,   6,   0,   4,
    -3,   9,  12,   9, 14,  10,   3,   2,
    -6,   3,  13,  19,  7,  10,  -3,  -9,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -23,  -9, -23,  -5, -9, -16,  -5, -17,
    },

    // rook
    {
    13, 10, 18, 15, 12,  12,   8,   5,
    11, 13, 13, 11, -3,   3,   8,   3,
    7,  7,  7,  5,  4,  -3,  -5,  -3,
    4,  3, 13,  1,  2,   1,  -1,   2,
    3,  5,  8,  4, -5,  -6,  -8, -11,
    -4,  0, -5, -1, -7, -12,  -8, -16,
    -6, -6,  0,  2, -9,  -9, -11,  -3,
    -9,  2,  3, -1, -5, -13,   4, -20,
    },

    // queen
    {
    -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
    3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,
    },

    // king
    {
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
    10,  17,  23,  15,  20,  45,  44,  13,
    -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
    }
    }
};

// mirror for opponent
const int mirror_score[128] ={
	a1, b1, c1, d1, e1, f1, g1, h1,
	a2, b2, c2, d2, e2, f2, g2, h2,
	a3, b3, c3, d3, e3, f3, g3, h3,
	a4, b4, c4, d4, e4, f4, g4, h4,
	a5, b5, c5, d5, e5, f5, g5, h5,
	a6, b6, c6, d6, e6, f6, g6, h6,
	a7, b7, c7, d7, e7, f7, g7, h7,
	a8, b8, c8, d8, e8, f8, g8, h8
};


U64 file_masks[64];
U64 rank_masks[64];
U64 isolated_masks[64];
U64 white_passed_masks[64];
U64 black_passed_masks[64];

const int get_rank[64] = {
    7, 7, 7, 7, 7, 7, 7, 7,
    6, 6, 6, 6, 6, 6, 6, 6,
    5, 5, 5, 5, 5, 5, 5, 5,
    4, 4, 4, 4, 4, 4, 4, 4,
    3, 3, 3, 3, 3, 3, 3, 3,
    2, 2, 2, 2, 2, 2, 2, 2,
    1, 1, 1, 1, 1, 1, 1, 1,
    0, 0, 0, 0, 0, 0, 0, 0,
};

const int double_pawn_penalty_opening = -5;
const int double_pawn_penalty_endgame = -10;

const int isolated_pawn_penalty_opening = -5;
const int isolated_pawn_penalty_endgame = -10;

const int passed_pawn_bonus[8] = { 0, 10, 30, 50, 75, 100, 150, 200 }; 

const int semi_open_file_score = 10;

const int open_file_score = 15;

const int bishop_unit = 4;
const int queen_unit = 9;

const int bishop_mobility_opening = 5;
const int bishop_mobility_endgame = 5;
const int queen_mobility_opening = 1;
const int queen_mobility_endgame = 2;

const int king_shield_bonus = 5;

U64 set_file_rank(int file_number, int rank_number) {
    // file or rank mask
    U64 mask = 0ULL;

    for (int i=0; i < 8; i++) {
        for (int j=0; j < 8; j++) {
            int square = i*8 + j;

            if (file_number != -1) {
                if (j == file_number)
                    set_bit(mask, square);     
            } else if (rank_number != -1) {
                if (i == rank_number)
                    set_bit(mask, square);
            }
        }
    }

    return mask;
}

void initialize_evaluation_masks() {
    for (int i=0; i < 8; i++) {
        for (int j=0; j < 8; j++) {
            file_masks[i*8+j] = set_file_rank(j, -1);
            rank_masks[i*8+j] = set_file_rank(-1, i);
        }   
    }     

    for (int i=0; i < 8; i++) {
        for (int j=0; j < 8; j++) {
            isolated_masks[i*8+j] |= set_file_rank(j-1, -1);
            isolated_masks[i*8+j] |= set_file_rank(j+1, -1);
        }   
    }       

  for (int i=0; i < 8; i++) {
        for (int j=0; j < 8; j++) {
            white_passed_masks[i*8+j] |= set_file_rank(j-1, -1);
            white_passed_masks[i*8+j] |= set_file_rank(j, -1);
            white_passed_masks[i*8+j] |= set_file_rank(j+1, -1);

            white_passed_masks[i*8+j] >>= 8*(8-i);
        }   
    }      

    for (int i=0; i < 8; i++) {
        for (int j=0; j < 8; j++) {
            black_passed_masks[i*8+j] |= set_file_rank(j-1, -1);
            black_passed_masks[i*8+j] |= set_file_rank(j, -1);
            black_passed_masks[i*8+j] |= set_file_rank(j+1, -1);

            black_passed_masks[i*8+j] <<= 8*(i+1);
        }   
    }      
}       

inline int get_game_phase_score() {
    int white_piece_scores = 0;
    int black_piece_scores = 0;

    for (int pp = N; pp <= Q; pp++) {
        white_piece_scores += count_bits(bitboards[pp]) * material_score[opening][pp];
    }
    for (int pp = n; pp <= q; pp++) {
        black_piece_scores += count_bits(bitboards[pp]) * -material_score[opening][pp];
    }

    return white_piece_scores + black_piece_scores;
}

int evaluate() {
    int game_phase_score = get_game_phase_score();
    
    int game_phase;
    if (game_phase_score > opening_phase_score) game_phase = opening;
    else if (game_phase_score < endgame_phase_score) game_phase = endgame;
    else game_phase = middlegame;

    int score = 0, score_opening = 0, score_endgame = 0;

    U64 bitboard;

    int piece, square, double_pawns = 0;

    for (int pp = P; pp <= k; pp++) {

        // get all of the bits with the piece turned on
        bitboard = bitboards[pp];

        while(bitboard) {
            piece = pp;

            square = get_lsb1_index(bitboard);

            score_opening += material_score[opening][piece];
            score_endgame += material_score[endgame][piece];
            
            
            switch(piece) {
                case P: 
                    score_opening += positional_score[opening][PAWN][square];
                    score_endgame += positional_score[endgame][PAWN][square];

                    // double pawn penalty
                    double_pawns = count_bits(bitboards[P] & file_masks[square]);
                    if (double_pawns > 1) {
                        // penalty is negative
                        score_opening += (double_pawns - 1) * double_pawn_penalty_opening;
                        score_endgame += (double_pawns - 1) * double_pawn_penalty_endgame;
                    }
                    if ((bitboards[P] & isolated_masks[square]) == 0) {
                        score_opening += isolated_pawn_penalty_opening;
                        score_endgame += isolated_pawn_penalty_endgame;
                    }

                    if ((white_passed_masks[square] & bitboards[p]) == 0) {
                        score_opening += passed_pawn_bonus[get_rank[square]];
                        score_endgame += passed_pawn_bonus[get_rank[square]];
                    }

                    break;
                case N: 
                    score_opening += positional_score[opening][KNIGHT][square];
                    score_endgame += positional_score[endgame][KNIGHT][square];
                    break;
                case B: 
                    score_opening += positional_score[opening][BISHOP][square];
                    score_endgame += positional_score[endgame][BISHOP][square];
                    
                    score_opening += (count_bits(get_bishop_attacks(square, occupancies[both])) - bishop_unit) * bishop_mobility_opening;
                    score_endgame += (count_bits(get_bishop_attacks(square, occupancies[both])) - bishop_unit) * bishop_mobility_endgame;

                    break;
                case R: 
                    score_opening += positional_score[opening][ROOK][square];
                    score_endgame += positional_score[endgame][ROOK][square];
                    
                    // if no pawns of our color
                    if ((bitboards[P] & file_masks[square]) == 0) {
                        score_opening += semi_open_file_score;
                        score_endgame += semi_open_file_score;
                    }

                    // if no pawns at all
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0) {
                        score_opening += open_file_score;
                        score_endgame += open_file_score;
                    }

                    break;

                case Q:
                    score_opening += positional_score[opening][QUEEN][square];
                    score_endgame += positional_score[endgame][QUEEN][square];
                    
                    score_opening += (count_bits(get_queen_attacks(square, occupancies[both])) - queen_unit) * queen_mobility_opening;
                    score_endgame += (count_bits(get_queen_attacks(square, occupancies[both])) - queen_unit) * queen_mobility_endgame;

                    break;
                    
                case K: 
                    // // positional score
                    score_opening += positional_score[opening][KING][square];
                    score_endgame += positional_score[endgame][KING][square];

                    // if no pawns of our color
                    if ((bitboards[P] & file_masks[square]) == 0) {
                        score_opening -= semi_open_file_score;
                        score_endgame -= semi_open_file_score;
                    }

                    // if no pawns at all
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0) {
                        score_opening -= open_file_score;
                        score_endgame -= open_file_score;
                    }

                    score_opening += count_bits(king_attacks[square] & occupancies[white]) * king_shield_bonus;
                    score_endgame += count_bits(king_attacks[square] & occupancies[white]) * king_shield_bonus;
                    
                    break;

                case p: 
                    score_opening -= positional_score[opening][PAWN][mirror_score[square]];
                    score_endgame -= positional_score[endgame][PAWN][mirror_score[square]];
                    
                    // double pawn penalty
                    double_pawns = count_bits(bitboards[p] & file_masks[square]);
                    if (double_pawns > 1) {
                        // penalty is negative
                        score_opening -= (double_pawns - 1) * double_pawn_penalty_opening;
                        score_endgame -= (double_pawns - 1) * double_pawn_penalty_endgame;
                    }
                    if ((bitboards[p] & isolated_masks[square]) == 0) {
                        score_opening -= isolated_pawn_penalty_opening;
                        score_endgame -= isolated_pawn_penalty_endgame;
                    }

                    if ((black_passed_masks[square] & bitboards[P]) == 0) {
                        score_opening -= passed_pawn_bonus[get_rank[mirror_score[square]]];
                        score_endgame -= passed_pawn_bonus[get_rank[mirror_score[square]]];
                    }

                    break;
                case n: 
                    score_opening -= positional_score[opening][KNIGHT][mirror_score[square]];
                    score_endgame -= positional_score[endgame][KNIGHT][mirror_score[square]];
                    break;
                case b: 
                    score_opening -= positional_score[opening][BISHOP][mirror_score[square]];
                    score_endgame -= positional_score[endgame][BISHOP][mirror_score[square]];
                    
                    score_opening -= (count_bits(get_bishop_attacks(square, occupancies[both])) - bishop_unit) * bishop_mobility_opening;
                    score_endgame -= (count_bits(get_bishop_attacks(square, occupancies[both])) - bishop_unit) * bishop_mobility_endgame;

                    break;
                case r:
                    score_opening -= positional_score[opening][ROOK][mirror_score[square]];
                    score_endgame -= positional_score[endgame][ROOK][mirror_score[square]];

                    // if no pawns of our color
                    if ((bitboards[p] & file_masks[square]) == 0) {
                        score_opening -= semi_open_file_score;
                        score_endgame -= semi_open_file_score;
                    }

                    // if no pawns at all
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0) {
                        score_opening -= open_file_score;
                        score_endgame -= open_file_score;
                    }

                    break;

                case q:
                    score_opening -= positional_score[opening][QUEEN][mirror_score[square]];
                    score_endgame -= positional_score[endgame][QUEEN][mirror_score[square]];
                    
                    score_opening -= (count_bits(get_queen_attacks(square, occupancies[both])) - queen_unit) * queen_mobility_opening;
                    score_endgame -= (count_bits(get_queen_attacks(square, occupancies[both])) - queen_unit) * queen_mobility_endgame;

                    break;
                case k: 
                    score_opening -= positional_score[opening][KING][mirror_score[square]];
                    score_endgame -= positional_score[endgame][KING][mirror_score[square]];

                    // if no pawns of our color
                    if ((bitboards[p] & file_masks[square]) == 0) {
                        score_opening += semi_open_file_score;
                        score_endgame += semi_open_file_score;
                    }

                    // if no pawns at all
                    if (((bitboards[P] | bitboards[p]) & file_masks[square]) == 0) {
                        score_opening += open_file_score;
                        score_endgame += open_file_score;
                    }

                    score_opening += count_bits(king_attacks[square] & occupancies[black]) * king_shield_bonus;
                    score_endgame += count_bits(king_attacks[square] & occupancies[black]) * king_shield_bonus;
                    
                    break;
            }
            
            pop_bit(bitboard, square);
        }
    }
    // interpolation
    if (game_phase == middlegame) {
        score = (
            score_opening * game_phase_score +
            score_endgame * (opening_phase_score - game_phase_score)
        ) / opening_phase_score;
    }
    else if (game_phase == opening) score = score_opening;
    else if (game_phase == endgame) score = score_endgame;

    if (side == black) score = -score;
    return score * (100 - fifty) / 100;
}

void print_move_scores(moves moves) {
    
     printf("     move scores:\n\n");
    
    for (int i = 0; i < moves.count; i++) {
        // print_move(moves.moves[i]);
        printf("     move: ");
        print_move(moves.moves[i]);
        printf("     score: %d\n", score_move(moves.moves[i]));
    }
}