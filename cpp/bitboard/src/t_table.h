#pragma once

#ifndef T_TABLE_H
#define T_TABLE_H

#include "bbc.h"
#include "board.h"

// hash table size
int hash_entries = 0;

U64 generate_hash_key();

void clear_hash_table();

void init_hash_table(int mb);

inline void write_hash_entry(int score, int best_move, int depth, int flag) {
    tt *hash_entry = &hash_table[hash_key % hash_entries];

    if (score < -mate_score) score -= ply;
    if (score > mate_score) score += ply;

    hash_entry->best_move = best_move;
    hash_entry->hash_key = hash_key;
    hash_entry->score = score;
    hash_entry->flag = flag;
    hash_entry->depth = depth;
}

inline int read_hash_entry(int alpha, int beta, int* best_move, int depth) {
    tt *hash_entry = &hash_table[hash_key % hash_entries];

    if (hash_entry->hash_key == hash_key) {
        if (hash_entry->depth >= depth) {

            int score = hash_entry->score;

            if (score < -mate_score) score += ply;
            if (score > mate_score) score -= ply;

            if (hash_entry->flag == hash_flag_exact) {
                return score;
            }
            if (hash_entry->flag == hash_flag_alpha 
            &&  score <= alpha) {
                return alpha;
            }

            if (hash_entry->flag == hash_flag_beta 
            &&  score >= beta) {
                return beta;
            }
        }
        *best_move = hash_entry->best_move;
    }
    return no_hash_entry;
}


#endif // T_TABLE_H
