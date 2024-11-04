// #include "bbc.h"
#include "t_table.h"
// #include "board.h"

void clear_hash_table() {
    tt *hash_entry;

    for (hash_entry = hash_table; hash_entry < hash_table + hash_entries; hash_entry ++) {
            hash_entry->hash_key = 0;
            hash_entry->depth = 0;
            hash_entry->flag = 0;
            hash_entry->score = 0;
            hash_entry->best_move = 0;
    }
}

void init_hash_table(int mb) {
    int hash_size = 0x100000 * mb;
    hash_entries = hash_size / sizeof(tt);

    if (hash_table != NULL) free(hash_table);

    hash_table = (tt*) malloc(hash_entries * sizeof(tt));

    if (hash_table == NULL) {
            printf(" Couldn't allocate memory for hash table, trying %dMB\n", mb/2);
            init_hash_table(mb/2);
    } else {
        clear_hash_table();
        printf(" Hash table is initialized with %d entries\n", hash_entries);
    }
}

U64 generate_hash_key() {
    U64 final_key = 0ULL;

    U64 bitboard;    
    for (int pp = P; pp <= k; pp++) {
        bitboard = bitboards[pp];

        while(bitboard) {
            int square = get_lsb1_index(bitboard);
            
            final_key ^= piece_keys[pp][square];
            
            pop_bit(bitboard, square);
        }
    }

    if (enpassant != no_sq) {
        // hash enpassant
        final_key ^= enpassant_keys[enpassant];
    }
    
    final_key ^= castle_keys[castle];

    if (side == black) {
        final_key ^= side_key;
    }
    
    return final_key;
}