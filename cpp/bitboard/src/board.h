#pragma once

#ifndef BOARD_H
#define BOARD_H

#include "bbc.h"

inline void add_move(moves *move_list, int move) {
    move_list->moves[move_list->count] = move;
    move_list->count++;
}

void generate_moves(moves* move_list);

void reset_board();

int make_move(int move, int move_flag);

void print_board();

void init_leapers_attacks();

void init_sliders_attacks(int bishop);

#define copy_board()                                                       \
    U64 bitboards_copy[12], occupancies_copy[3];                           \
    int side_copy, enpassant_copy, castle_copy;                            \
    memcpy(bitboards_copy, bitboards, 96);                                 \
    memcpy(occupancies_copy, occupancies, 24);                             \
    side_copy = side, enpassant_copy = enpassant, castle_copy = castle;    \
    U64 hash_key_copy = hash_key;                                          \
    int fifty_copy = fifty;                                                \

#define take_back()                                                        \
    memcpy(bitboards, bitboards_copy, 96);                                 \
    memcpy(occupancies, occupancies_copy, 24);                             \
    side = side_copy, enpassant = enpassant_copy, castle = castle_copy;    \
    hash_key = hash_key_copy;                                              \
    fifty = fifty_copy;                                                    \

const char* square_to_coordinates[64];

inline int count_bits (U64 bitboard) {
    int counter = 0;

    while (bitboard) {
        counter++;
        bitboard &= bitboard-1;
    }
    return counter;
}

inline int get_lsb1_index (U64 bitboard) {
    if (bitboard == 0ULL) { return -1; }
    return count_bits((bitboard & -bitboard)-1);
}

inline U64 get_bishop_attacks(int square, U64 occupancy) {
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64 - bishop_relevant_bits[square];

    return bishop_attacks[square][occupancy];
}

inline U64 get_queen_attacks(int square, U64 occupancy) {
    U64 queen_attacks = 0ULL;

    U64 bishop_occupancies = occupancy;
    U64 rook_occupancies = occupancy;
    
    bishop_occupancies &= bishop_masks[square];
    bishop_occupancies *= bishop_magic_numbers[square];
    bishop_occupancies >>= 64 - bishop_relevant_bits[square];

    queen_attacks |= bishop_attacks[square][bishop_occupancies];

    rook_occupancies &= rook_masks[square];
    rook_occupancies *= rook_magic_numbers[square];
    rook_occupancies >>= 64 - rook_relevant_bits[square];

    queen_attacks |= rook_attacks[square][rook_occupancies];

    return queen_attacks;
}

inline U64 get_rook_attacks(int square, U64 occupancy) {
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_numbers[square];
    occupancy >>= 64 - rook_relevant_bits[square];

    return rook_attacks[square][occupancy];
}

U64 mask_bishop_attacks(int square);

U64 mask_rook_attacks(int square);

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask);

U64 bishop_attacks_on_the_fly(int square, U64 block);

U64 rook_attacks_on_the_fly(int square, U64 block);

inline int is_square_attacked(int square, int side) {
    if (side == white && (pawn_attacks[black][square] & bitboards[P])) return 1;
    if (side == black && (pawn_attacks[white][square] & bitboards[p])) return 1;
    if (knight_attacks[square] & (side == white ? bitboards[N] : bitboards[n])) return 1;
    if (king_attacks[square] & (side == white ? bitboards[K] : bitboards[k])) return 1;
    
    if (get_bishop_attacks(square, occupancies[both]) & (side == white ? bitboards[B] : bitboards[b])) return 1;
    if (get_rook_attacks(square, occupancies[both]) & (side == white ? bitboards[R] : bitboards[r])) return 1;
    if (get_queen_attacks(square, occupancies[both]) & (side == white ? bitboards[Q] : bitboards[q])) return 1;
    
    return 0;
}

void print_move(int move);

#endif // BOARD_H
