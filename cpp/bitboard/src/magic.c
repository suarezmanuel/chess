// #include "bbc.h"
#include "magic.h"
// #include "board.h"

unsigned int state = 1804289383;

// generate U32 number
unsigned int get_random_U32_number() {

    state ^= state << 13;
    state ^= state >> 17;
    state ^= state << 5;

    return state;
}

U64 get_random_U64_number() {
    U64 n1, n2, n3, n4;
    // get bottom 16 bits of number
    n1 = (U64)(get_random_U32_number() & 0xFFFF);
    n2 = (U64)(get_random_U32_number() & 0xFFFF);
    n3 = (U64)(get_random_U32_number() & 0xFFFF);
    n4 = (U64)(get_random_U32_number() & 0xFFFF);

    return n1 | (n2 << 16) | (n3 << 32) | (n4 << 48);
}

// generate magic number candidate
U64 generate_magic_number() {
    return get_random_U64_number() & get_random_U64_number() & get_random_U64_number();
}

U64 find_magic_number(int square, int relevant_bits, int bishop) {
    U64 occupancies[4096];
    U64 attacks[4096];
    U64 used_attacks[4096];

    U64 attack_mask = bishop ? mask_bishop_attacks(square) : mask_rook_attacks(square);

    int occupancy_indicies = 1 << relevant_bits;
    for (int i = 0; i < occupancy_indicies; i++) {
        occupancies[i] = set_occupancy(i, relevant_bits, attack_mask);

        attacks[i] = bishop ? bishop_attacks_on_the_fly(square, occupancies[i]) : rook_attacks_on_the_fly(square, occupancies[i]);
    }

    for (int random_count = 0; random_count < 100000000; random_count++) {
        U64 magic_number = generate_magic_number();

        if (count_bits((attack_mask * magic_number) & 0xFF00000000000000) < 6) continue;

        memset(used_attacks, 0ULL, sizeof(used_attacks));

        int i, fail;

        for (i = 0, fail = 0; !fail && i < occupancy_indicies; i++) {
            int magic_i = (int)((occupancies[i] * magic_number) >> (64 - relevant_bits));
        
            if (used_attacks[magic_i] == 0ULL) 
                used_attacks[magic_i] = attacks[i];

            else if (used_attacks[magic_i] != attacks[i])
                fail = 1;
        }

        if (!fail)
            return magic_number;
    }

    printf("Magic number fails!\n");
    return 0ULL;
}

void init_magic_numbers() {
    for (int square = 0; square < 64; square++) {
        rook_magic_numbers[square] = find_magic_number(square, rook_relevant_bits[square], rook); 
    }

    for (int square = 0; square < 64; square++) {
        bishop_magic_numbers[square] = find_magic_number(square, bishop_relevant_bits[square], bishop); 
    }

    // for (int square = 0; square < 64; square++) {
    //     printf(" 0x%llxULL,\n", find_magic_number(square, rook_relevant_bits[square], rook)); 
    // }
    // printf("\n\n\n");

    // for (int square = 0; square < 64; square++) {
    //     printf(" 0x%llxULL,\n", find_magic_number(square, bishop_relevant_bits[square], bishop)); 
    // }
}

void init_random_keys() {
    
    state = 1804289383;

    for (int pp = P; pp <= k; pp++) {
        for (int square = 0; square < 64; square++) {
            piece_keys[pp][square] = get_random_U64_number();
        }
    } 

    for (int square = 0; square < 64; square++) {
        enpassant_keys[square] = get_random_U64_number();
    }

    for (int i = 0; i < 16; i++) {
        castle_keys[i] = get_random_U64_number();
    }   

    side_key = get_random_U64_number();
} 