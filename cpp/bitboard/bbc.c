#include <stdio.h>

#define U64 unsigned long long

enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1,
};

enum {white, black};

const char* square_to_coordinates[] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) (bitboard ^= (1ULL << square))

static inline int bit_count (U64 bitboard) {
    int counter = 0;

    while (bitboard) {
        counter++;
        bitboard &= bitboard-1;
    }
    return counter;
}

static inline int get_lsb1_index (U64 bitboard) {
    int index = 0;
    if (bitboard == 0ULL) { return -1; }
    return bit_count((bitboard & -bitboard)-1);
}

/*
    not a file

8  0 1 1 1 1 1 1 1 
7  0 1 1 1 1 1 1 1 
6  0 1 1 1 1 1 1 1 
5  0 1 1 1 1 1 1 1 
4  0 1 1 1 1 1 1 1 
3  0 1 1 1 1 1 1 1 
2  0 1 1 1 1 1 1 1 
1  0 1 1 1 1 1 1 1 

   a b c d e f g h 


    not ab file

8  0 0 1 1 1 1 1 1 
7  0 0 1 1 1 1 1 1 
6  0 0 1 1 1 1 1 1 
5  0 0 1 1 1 1 1 1 
4  0 0 1 1 1 1 1 1 
3  0 0 1 1 1 1 1 1 
2  0 0 1 1 1 1 1 1 
1  0 0 1 1 1 1 1 1 

   a b c d e f g h 


    not h file

8  1 1 1 1 1 1 1 0 
7  1 1 1 1 1 1 1 0 
6  1 1 1 1 1 1 1 0 
5  1 1 1 1 1 1 1 0 
4  1 1 1 1 1 1 1 0 
3  1 1 1 1 1 1 1 0 
2  1 1 1 1 1 1 1 0 
1  1 1 1 1 1 1 1 0 

   a b c d e f g h 


    not gh file

8  1 1 1 1 1 1 0 0 
7  1 1 1 1 1 1 0 0 
6  1 1 1 1 1 1 0 0 
5  1 1 1 1 1 1 0 0 
4  1 1 1 1 1 1 0 0 
3  1 1 1 1 1 1 0 0 
2  1 1 1 1 1 1 0 0 
1  1 1 1 1 1 1 0 0 

   a b c d e f g h 

*/

const U64 not_a_file  = 18374403900871474942ULL;
const U64 not_h_file  = 9187201950435737471ULL;
const U64 not_ab_file = 18229723555195321596ULL;
const U64 not_gh_file = 4557430888798830399ULL;

const int bishop_revelant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6, 
};

const int rook_revelant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12, 
};

U64 pawn_attacks[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];
U64 bishop_attacks[64];
U64 rook_attacks[64];
U64 queen_attacks[64];


// can remove if
U64 mask_pawn_attacks(int side, int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    set_bit(bitboard, square);

    // if white
    if (!side) {
        attacks |= (not_h_file & (bitboard << 7));
        attacks |= (not_a_file & (bitboard << 9));
    } else {
        attacks |= (not_a_file & (bitboard >> 7));
        attacks |= (not_h_file & (bitboard >> 9));
    }

    return attacks;
}

U64 mask_knight_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    set_bit(bitboard, square);

    // top left
    attacks |= (not_gh_file & (bitboard >> 10));
    attacks |= (not_h_file & (bitboard >> (9+8)));
    // top right
    attacks |= (not_ab_file & (bitboard >> 6));
    attacks |= (not_a_file & (bitboard >> (7+8)));
    // bottom left
    attacks |= (not_h_file & (bitboard << (7+8)));
    attacks |= (not_gh_file & (bitboard << 6));
    // bottom right
    attacks |= (not_ab_file & (bitboard << (2+8)));
    attacks |= (not_a_file & (bitboard << (1+8+8)));
    return attacks;
}

U64 mask_king_attacks(int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    set_bit(bitboard, square);

    // down
    attacks |= bitboard << 8;
    // up
    attacks |= bitboard >> 8;
    // bottom right
    attacks |= (not_a_file & (bitboard << 9));
    // bottom left
    attacks |= (not_h_file & (bitboard << 7));
    // top right
    attacks |= (not_a_file & (bitboard >> 7));
    attacks |= (not_h_file & (bitboard >> 9));
    // right
    attacks |= (not_a_file & (bitboard << 1));
    // left
    attacks |= (not_h_file & (bitboard >> 1));
    return attacks;
}

U64 mask_bishop_attacks(int square) {
    U64 attacks = 0ULL;
    int r = square/8;
    int c = square%8;
    for (int i=r+1, j=c+1; i < 7 && j < 7; i++, j++) attacks |= (1ULL << (i*8 + j));
    for (int i=r+1, j=c-1; i < 7 && j > 0; i++, j--) attacks |= (1ULL << (i*8 + j));
    for (int i=r-1, j=c+1; i > 0 && j < 7; i--, j++) attacks |= (1ULL << (i*8 + j));
    for (int i=r-1, j=c-1; i > 0 && j > 0; i--, j--) attacks |= (1ULL << (i*8 + j));
    return attacks;
}

U64 mask_rook_attacks(int square) {
    U64 attacks = 0ULL;
    int r = square/8;
    int c = square%8;
    // go right
    for (int i=r+1; i < 7; i++) attacks |= (1ULL << (i*8 + c));
    for (int i=r-1; i > 0; i--) attacks |= (1ULL << (i*8 + c));
    for (int i=c+1; i < 7; i++) attacks |= (1ULL << (r*8 + i));
    for (int i=c-1; i > 0; i--) attacks |= (1ULL << (r*8 + i));
    return attacks;
}

U64 bishop_attacks_on_the_fly(int square, U64 block) {
    U64 attacks = 0ULL;
    int r = square/8;
    int c = square%8;
    U64 t;
    for (int i=r+1, j=c+1; i < 7 && j < 7; i++, j++) 
    { 
        t = 1ULL << (i*8 + j);
        attacks |= t;
        if (block & t) break;
    }
    for (int i=r+1, j=c-1; i < 7 && j > 0; i++, j--) 
    { 
        t = 1ULL << (i*8 + j);
        attacks |= t;
        if (block & t) break;
    }
    for (int i=r-1, j=c+1; i > 0 && j < 7; i--, j++) 
    { 
        t = 1ULL << (i*8 + j);
        attacks |= t;
        if (block & t) break;
    }
    for (int i=r-1, j=c-1; i > 0 && j > 0; i--, j--) 
    { 
        t = 1ULL << (i*8 + j);
        attacks |= t;
        if (block & t) break;
    }
    return attacks;
}

U64 rook_attacks_on_the_fly(int square, U64 block) {
    U64 attacks = 0ULL;
    int r = square/8;
    int c = square%8;
    U64 t;
    // go right
    for (int i=r+1; i < 7; i++) 
    { 
        t = 1ULL << (i*8 + c);
        attacks |= t;
        if (block & t) break;
    }
    // go left
    for (int i=r-1; i > 0; i--) 
    { 
        t = 1ULL << (i*8 + c);
        attacks |= t;
        if (block & t) break;
    }
    // go down
    for (int i=c+1; i < 7; i++) 
    { 
        t = 1ULL << (r*8 + i);
        attacks |= t;
        if (block & t) break;
    }
    // go up
    for (int i=c-1; i > 0; i--) 
    { 
        t = 1ULL << (r*8 + i);
        attacks |= t;
        if (block & t) break;
    }
    return attacks;
}

void print_bitboard(U64 board) {
    printf("\n");
    for (int i=0; i < 8; i++) {
        printf("%d  ", 8-i);
        for (int j=0; j < 8; j++) {
            int e = i*8 + j;
            printf("%d ", get_bit(board,e) ? 1 : 0);
        }
        printf("\n");
    }
    printf("\n   a b c d e f g h \n");
    printf("\n    Bitboard: %llud\n\n", board);
}

void init_leapers_attacks() {
    for (int i=0; i < 64; i++) {
        pawn_attacks[white][i] = mask_pawn_attacks(white, i);
        pawn_attacks[black][i] = mask_pawn_attacks(black, i);
        knight_attacks[i] = mask_knight_attacks(i);
        king_attacks[i] = mask_king_attacks(i);
        bishop_attacks[i] = mask_bishop_attacks(i);
        rook_attacks[i] = mask_rook_attacks(i);
    }
}

U64 set_occupancy(int index, int bits_in_mask, U64 attack_mask) {
    U64 occupancy = 0ULL;

    for (int i=0; i < bits_in_mask; i++) {
        int square = get_lsb1_index(attack_mask);
        pop_bit(attack_mask, square);
        if (index & (1 << i)) {
            occupancy |= (1ULL << square);
        }
    }

    return occupancy;
}

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

int main () {
    init_leapers_attacks();
   
}