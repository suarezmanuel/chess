#define NOMINMAX
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#ifdef WIN64
    #include <windows.h>
#else
    # include <sys/time.h>
#endif

#define min(a,b) ((a) < (b) ? (a) : (b))

#define U64 unsigned long long

enum {
    a8, b8, c8, d8, e8, f8, g8, h8,
    a7, b7, c7, d7, e7, f7, g7, h7,
    a6, b6, c6, d6, e6, f6, g6, h6,
    a5, b5, c5, d5, e5, f5, g5, h5,
    a4, b4, c4, d4, e4, f4, g4, h4,
    a3, b3, c3, d3, e3, f3, g3, h3,
    a2, b2, c2, d2, e2, f2, g2, h2,
    a1, b1, c1, d1, e1, f1, g1, h1, no_sq,
};

enum { white, black, both };
enum { rook, bishop };

// represent castling rights using 4 bits
enum { wk = 1, wq = 2, bk = 4, bq = 8 };

enum { P, N, B, R, Q, K, p, n, b, r, q, k };

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

char ascii_pieces[12] = "PNBRQKpnbrqk";

char* unicode_pieces[12] = {"♙", "♘", "♗", "♖", "♕", "♔", "♟︎", "♞", "♝", "♜", "♛", "♚"};

int char_pieces[] = {
    ['P'] = P,
    ['N'] = N,
    ['B'] = B,
    ['R'] = R,
    ['Q'] = Q,
    ['K'] = K,
    ['p'] = p,
    ['n'] = n,
    ['b'] = b,
    ['r'] = r,
    ['q'] = q,
    ['k'] = k
};

#define empty_board "8/8/8/8/8/8/8/8 b - - "
#define start_position "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1 "
#define tricky_position "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1 "
#define killer_position "rnbqkb1r/pp1p1pPp/8/2p1pP2/1P1P4/3P3P/P1P1P3/RNBQKBNR w KQkq e6 0 1"
#define cmk_position "r2q1rk1/ppp2ppp/2n1bn2/2b1p3/3pP3/3P1NPP/PPP1NPB1/R1BQ1RK1 b - - 0 9 "

#define get_bit(bitboard, square) (bitboard & (1ULL << square))
#define set_bit(bitboard, square) (bitboard |= (1ULL << square))
#define pop_bit(bitboard, square) (bitboard ^= (1ULL << square))

#define encode_move(source, target, piece, promoted, capture, double, enpassant, castling) \
((source) | (target << 6) | (piece << 12) | (promoted << 16) | (capture << 20) | (double << 21) | (enpassant << 22) | (castling << 23)) \

#define get_move_source(move)    (move & 0x3f)
#define get_move_target(move)    ((move & 0xfc0) >> 6)
#define get_move_piece(move)     ((move & 0xf000) >> 12)
#define get_move_promoted(move)  ((move & 0xf0000) >> 16)
#define get_move_capture(move)   ((move & 0x100000) >> 20)
#define get_move_double(move)    ((move & 0x200000) >> 21)
#define get_move_enpassant(move) ((move & 0x400000) >> 22)
#define get_move_castling(move)  ((move & 0x800000) >> 23)

typedef struct {
    int moves[256];

    int count;
} moves;

static inline void add_move(moves *move_list, int move) {
    move_list->moves[move_list->count] = move;
    move_list->count++;
}

char promoted_pieces[] = {[Q] = 'q', [R] = 'r', [B] = 'b', [N] = 'n', [q] = 'q', [r] = 'r', [b] = 'b', [n] = 'n'};

void print_move(int move) {
    printf("%s%s%c", square_to_coordinates[get_move_source(move)], square_to_coordinates[get_move_target(move)], promoted_pieces[get_move_promoted(move)]);
}

void print_move_count(int move, long count) {
    printf("%s%s%c: %ld\n", square_to_coordinates[get_move_source(move)], square_to_coordinates[get_move_target(move)], promoted_pieces[get_move_promoted(move)], count);
}

void print_move_list(moves *move_list) {
    printf("\n    move    piece   capture   double    enpass    castling\n\n");
    
    // loop over moves within a move list
    for (int move_count = 0; move_count < move_list->count; move_count++)
    {
        // init move
        int move = move_list->moves[move_count];
        
        #ifdef WIN64
            // print move
            printf("    %s%s%c   %c       %d         %d         %d         %d\n", square_to_coordinates[get_move_source(move)],
                                                                                  square_to_coordinates[get_move_target(move)],
                                                                                  promoted_pieces[get_move_promoted(move)] ? promoted_pieces[get_move_promoted(move)] : ' ',
                                                                                  ascii_pieces[get_move_piece(move)],
                                                                                  get_move_capture(move) ? 1 : 0,
                                                                                  get_move_double(move) ? 1 : 0,
                                                                                  get_move_enpassant(move) ? 1 : 0,
                                                                                  get_move_castling(move) ? 1 : 0);
        #else
            // print move
            printf("    %s%s%c   %s       %d         %d         %d         %d\n", square_to_coordinates[get_move_source(move)],
                                                                                  square_to_coordinates[get_move_target(move)],
                                                                                  promoted_pieces[get_move_promoted(move)] ? promoted_pieces[get_move_promoted(move)] : ' ',
                                                                                  unicode_pieces[get_move_piece(move)],
                                                                                  get_move_capture(move) ? 1 : 0,
                                                                                  get_move_double(move) ? 1 : 0,
                                                                                  get_move_enpassant(move) ? 1 : 0,
                                                                                  get_move_castling(move) ? 1 : 0);
        #endif
        
        // print total number of moves
    }
    printf("\n\n    Total number of moves: %d\n\n", move_list->count);
}

static inline int count_bits (U64 bitboard) {
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
    return count_bits((bitboard & -bitboard)-1);
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

// define bitboards
U64 bitboards[12];

// occupancy bitboards
U64 occupancies[3];

// side to move
int side = white;

int enpassant = no_sq;

int castle = -1;



const int bishop_relevant_bits[64] = {
    6, 5, 5, 5, 5, 5, 5, 6, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 9, 9, 7, 5, 5, 
    5, 5, 7, 7, 7, 7, 5, 5, 
    5, 5, 5, 5, 5, 5, 5, 5, 
    6, 5, 5, 5, 5, 5, 5, 6, 
};

const int rook_relevant_bits[64] = {
    12, 11, 11, 11, 11, 11, 11, 12, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    11, 10, 10, 10, 10, 10, 10, 11, 
    12, 11, 11, 11, 11, 11, 11, 12, 
};

U64 rook_magic_numbers[64] = {
    0x8a80104000800020ULL,
    0x140002000100040ULL,
    0x2801880a0017001ULL,
    0x100081001000420ULL,
    0x200020010080420ULL,
    0x3001c0002010008ULL,
    0x8480008002000100ULL,
    0x2080088004402900ULL,
    0x800098204000ULL,
    0x2024401000200040ULL,
    0x100802000801000ULL,
    0x120800800801000ULL,
    0x208808088000400ULL,
    0x2802200800400ULL,
    0x2200800100020080ULL,
    0x801000060821100ULL,
    0x80044006422000ULL,
    0x100808020004000ULL,
    0x12108a0010204200ULL,
    0x140848010000802ULL,
    0x481828014002800ULL,
    0x8094004002004100ULL,
    0x4010040010010802ULL,
    0x20008806104ULL,
    0x100400080208000ULL,
    0x2040002120081000ULL,
    0x21200680100081ULL,
    0x20100080080080ULL,
    0x2000a00200410ULL,
    0x20080800400ULL,
    0x80088400100102ULL,
    0x80004600042881ULL,
    0x4040008040800020ULL,
    0x440003000200801ULL,
    0x4200011004500ULL,
    0x188020010100100ULL,
    0x14800401802800ULL,
    0x2080040080800200ULL,
    0x124080204001001ULL,
    0x200046502000484ULL,
    0x480400080088020ULL,
    0x1000422010034000ULL,
    0x30200100110040ULL,
    0x100021010009ULL,
    0x2002080100110004ULL,
    0x202008004008002ULL,
    0x20020004010100ULL,
    0x2048440040820001ULL,
    0x101002200408200ULL,
    0x40802000401080ULL,
    0x4008142004410100ULL,
    0x2060820c0120200ULL,
    0x1001004080100ULL,
    0x20c020080040080ULL,
    0x2935610830022400ULL,
    0x44440041009200ULL,
    0x280001040802101ULL,
    0x2100190040002085ULL,
    0x80c0084100102001ULL,
    0x4024081001000421ULL,
    0x20030a0244872ULL,
    0x12001008414402ULL,
    0x2006104900a0804ULL,
    0x1004081002402ULL
};

U64 bishop_magic_numbers[64] = {
    0x40040844404084ULL,
    0x2004208a004208ULL,
    0x10190041080202ULL,
    0x108060845042010ULL,
    0x581104180800210ULL,
    0x2112080446200010ULL,
    0x1080820820060210ULL,
    0x3c0808410220200ULL,
    0x4050404440404ULL,
    0x21001420088ULL,
    0x24d0080801082102ULL,
    0x1020a0a020400ULL,
    0x40308200402ULL,
    0x4011002100800ULL,
    0x401484104104005ULL,
    0x801010402020200ULL,
    0x400210c3880100ULL,
    0x404022024108200ULL,
    0x810018200204102ULL,
    0x4002801a02003ULL,
    0x85040820080400ULL,
    0x810102c808880400ULL,
    0xe900410884800ULL,
    0x8002020480840102ULL,
    0x220200865090201ULL,
    0x2010100a02021202ULL,
    0x152048408022401ULL,
    0x20080002081110ULL,
    0x4001001021004000ULL,
    0x800040400a011002ULL,
    0xe4004081011002ULL,
    0x1c004001012080ULL,
    0x8004200962a00220ULL,
    0x8422100208500202ULL,
    0x2000402200300c08ULL,
    0x8646020080080080ULL,
    0x80020a0200100808ULL,
    0x2010004880111000ULL,
    0x623000a080011400ULL,
    0x42008c0340209202ULL,
    0x209188240001000ULL,
    0x400408a884001800ULL,
    0x110400a6080400ULL,
    0x1840060a44020800ULL,
    0x90080104000041ULL,
    0x201011000808101ULL,
    0x1a2208080504f080ULL,
    0x8012020600211212ULL,
    0x500861011240000ULL,
    0x180806108200800ULL,
    0x4000020e01040044ULL,
    0x300000261044000aULL,
    0x802241102020002ULL,
    0x20906061210001ULL,
    0x5a84841004010310ULL,
    0x4010801011c04ULL,
    0xa010109502200ULL,
    0x4a02012000ULL,
    0x500201010098b028ULL,
    0x8040002811040900ULL,
    0x28000010020204ULL,
    0x6000020202d0240ULL,
    0x8918844842082200ULL,
    0x4010011029020020ULL
};

U64 pawn_attacks[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];
U64 bishop_masks[64];
U64 rook_masks[64];
U64 bishop_attacks[64][512];
U64 rook_attacks[64][4096];
U64 queen_attacks[64];


// can remove if
U64 mask_pawn_attacks(int side, int square) {
    U64 attacks = 0ULL;
    U64 bitboard = 0ULL;
    set_bit(bitboard, square);

    if (side) {
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
    for (int i=r+1, j=c+1; i < 8 && j < 8; i++, j++) 
    { 
        t = 1ULL << (i*8 + j);
        attacks |= t;
        if (block & t) break;
    }
    for (int i=r+1, j=c-1; i < 8 && j >= 0; i++, j--) 
    { 
        t = 1ULL << (i*8 + j);
        attacks |= t;
        if (block & t) break;
    }
    for (int i=r-1, j=c+1; i >= 0 && j < 8; i--, j++) 
    { 
        t = 1ULL << (i*8 + j);
        attacks |= t;
        if (block & t) break;
    }
    for (int i=r-1, j=c-1; i >= 0 && j >= 0; i--, j--) 
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
    for (int i=r+1; i < 8; i++) 
    { 
        t = 1ULL << (i*8 + c);
        attacks |= t;
        if (block & t) break;
    }
    // go left
    for (int i=r-1; i >= 0; i--) 
    { 
        t = 1ULL << (i*8 + c);
        attacks |= t;
        if (block & t) break;
    }
    // go down
    for (int i=c+1; i < 8; i++) 
    { 
        t = 1ULL << (r*8 + i);
        attacks |= t;
        if (block & t) break;
    }
    // go up
    for (int i=c-1; i >= 0; i--) 
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

// arguments are global
void print_board() {
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {

            int square = rank*8 + file;

            if (!file)
                printf("  %d ", 8 - rank);

            int piece = -1;

            // goes over pieces
            for (int bb_piece = P; bb_piece <= k; bb_piece++) {
                if (get_bit(bitboards[bb_piece], square))
                    piece = bb_piece;
            }

            #ifdef WIN64
                printf(" %c", (piece == -1) ? '.' : ascii_pieces[piece]);
            #else
                printf(" %s", (piece == -1) ? "." : unicode_pieces[piece]);
            #endif
        }
        printf("\n");
    }
    printf("\n     a b c d e f g h \n\n");
    printf("     Side:    %s\n", !side ? "white" : "black");
    printf("     Enpassant: %s\n", enpassant != no_sq ? square_to_coordinates[enpassant] : "no");
    printf("     Castling: %c%c%c%c\n\n", castle & wk ? 'K' : '-', castle & wq ? 'Q' : '-', castle & bk ? 'k' : '-', castle & bq ? 'q' : '-');
    
}

void parse_fen(char* fen) {
    memset(bitboards, 0ULL, sizeof(bitboards));
    memset(occupancies, 0ULL, sizeof(occupancies));

    side = 0;
    enpassant = no_sq;
    castle = 0;

    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;

            if ((*fen > 'a' && *fen < 'z') || (*fen > 'A' && *fen < 'Z')) {
                int piece = char_pieces[*fen];
                set_bit(bitboards[piece], square);
                *fen++;
            } 
            
            if (*fen >= '0' && *fen <= '9') {
                int offset = *fen - '0';

                // why
                int piece = -1;
                for (int bb_piece = P; bb_piece <= k; bb_piece++) {
                    if (get_bit(bitboards[bb_piece], square)) {
                        piece = bb_piece;
                    }
                }

                if (piece == -1) {
                    file--;
                }

                file += offset;
                *fen++;
            }
            
            if (*fen == '/') {
                *fen++;
            }
        }
    }

    *fen++;
    side = *fen == 'w' ? white : black;

    fen += 2;
    while (*fen != ' ') {
        switch (*fen) {
            case 'K': castle |= wk; break;
            case 'Q': castle |= wq; break;
            case 'k': castle |= bk; break;
            case 'q': castle |= bq; break;
            case '-': break;
        }
        *fen++;
    }

    *fen++;
    if (*fen != '-') {
        int file = fen[0] - 'a';
        int rank = 8 - fen[1] + '0';

        enpassant = rank*8 + file;
    } else {
        enpassant = no_sq;
    }


    for (int piece = P; piece <= K; piece++) {
        occupancies[white] |= bitboards[piece];
    }
    for (int piece = p; piece <= k; piece++) {
        occupancies[black] |= bitboards[piece];
    }

    occupancies[both] = occupancies[white] | occupancies[black];
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

void init_sliders_attacks(int bishop) {
    for (int square=0; square<64; square++) {
        bishop_masks[square] = mask_bishop_attacks(square);
        rook_masks[square] = mask_rook_attacks(square);

        U64 attack_mask = bishop ? bishop_masks[square] : rook_masks[square];
        
        int relevant_bits_count = count_bits(attack_mask);

        int occupancy_indicies = 1 << relevant_bits_count;

        for (int index=0; index < occupancy_indicies; index++) {
            if (bishop) {
                U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                
                int magic_index = occupancy * bishop_magic_numbers[square] >> (64 - bishop_relevant_bits[square]);

                bishop_attacks[square][magic_index] = bishop_attacks_on_the_fly(square, occupancy);
            } else {
                U64 occupancy = set_occupancy(index, relevant_bits_count, attack_mask);
                
                int magic_index = occupancy * rook_magic_numbers[square] >> (64 - rook_relevant_bits[square]);

                rook_attacks[square][magic_index] = rook_attacks_on_the_fly(square, occupancy);
            }
        }
    }
}

void init_leapers_attacks() {
    for (int i=0; i < 64; i++) {
        pawn_attacks[white][i] = mask_pawn_attacks(white, i);
        pawn_attacks[black][i] = mask_pawn_attacks(black, i);
        knight_attacks[i] = mask_knight_attacks(i);
        king_attacks[i] = mask_king_attacks(i);
    }
}

static inline U64 get_bishop_attacks(int square, U64 occupancy) {
    occupancy &= bishop_masks[square];
    occupancy *= bishop_magic_numbers[square];
    occupancy >>= 64 - bishop_relevant_bits[square];

    return bishop_attacks[square][occupancy];
}

static inline U64 get_rook_attacks(int square, U64 occupancy) {
    occupancy &= rook_masks[square];
    occupancy *= rook_magic_numbers[square];
    occupancy >>= 64 - rook_relevant_bits[square];

    return rook_attacks[square][occupancy];
}

static inline U64 get_queen_attacks(int square, U64 occupancy) {
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

void init_all() {
    init_leapers_attacks();
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);
}

static inline int is_square_attacked(int square, int side) {
    if (side == white && (pawn_attacks[black][square] & bitboards[P])) return 1;
    if (side == black && (pawn_attacks[white][square] & bitboards[p])) return 1;
    if (knight_attacks[square] & (side == white ? bitboards[N] : bitboards[n])) return 1;
    if (king_attacks[square] & (side == white ? bitboards[K] : bitboards[k])) return 1;
    
    if (get_bishop_attacks(square, occupancies[both]) & (side == white ? bitboards[B] : bitboards[b])) return 1;
    if (get_rook_attacks(square, occupancies[both]) & (side == white ? bitboards[R] : bitboards[r])) return 1;
    if (get_queen_attacks(square, occupancies[both]) & (side == white ? bitboards[Q] : bitboards[q])) return 1;
    
    return 0;
}

void print_attacked_squares(int side) {
    printf("\n");
    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;

            if (!file) {
                printf("%d ", 8-rank);
            }

            printf(" %d", is_square_attacked(square, side) != 0);
        }

        printf("\n");
    }
    printf("\n   a b c d e f g h \n\n");
    // printf("\n    Bitboard: %llud\n\n", board);
}

#define copy_board()                                                       \
    U64 bitboards_copy[12], occupancies_copy[3];                           \
    int side_copy, enpassant_copy, castle_copy;                            \
    memcpy(bitboards_copy, bitboards, 96);                                 \
    memcpy(occupancies_copy, occupancies, 24);                             \
    side_copy = side, enpassant_copy = enpassant, castle_copy = castle;    \

#define take_back()                                                        \
    memcpy(bitboards, bitboards_copy, 96);                                 \
    memcpy(occupancies, occupancies_copy, 24);                             \
    side = side_copy, enpassant = enpassant_copy, castle = castle_copy;    \


// move types
enum { all_moves, only_captures };

const int castling_rights[64] = {
     7, 15, 15, 15,  3, 15, 15, 11,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    15, 15, 15, 15, 15, 15, 15, 15,
    13, 15, 15, 15, 12, 15, 15, 14
};

// returns 1 if valid move, 0 otherwise
static inline int make_move(int move, int move_flag) {

    if (move_flag == all_moves) {
        copy_board();

        // parse move
        int source_square = get_move_source(move);
        int target_square = get_move_target(move);
        int piece = get_move_piece(move);
        int promoted = get_move_promoted(move);
        int capture = get_move_capture(move);
        int double_push = get_move_double(move);
        int enpass = get_move_enpassant(move);
        int castling = get_move_castling(move);

        // move piece
        pop_bit(bitboards[piece], source_square);
        set_bit(bitboards[piece], target_square);


        if (capture) {
            int start_piece, end_piece;

            if (side == white) {
                start_piece = p;
                end_piece =  k;
            } else {
                start_piece = P;
                end_piece = K;
            }

            for (int pp = start_piece; pp <= end_piece; pp++) {
                if (get_bit(bitboards[pp], target_square)) {
                    pop_bit(bitboards[pp], target_square);
                    break;
                }

            }
        }

        if (promoted) {
            pop_bit(bitboards[side == white ? P : p], target_square);
            set_bit(bitboards[promoted], target_square);
        }

        int tt = target_square + (-(side == black) + (side == white))*8;
        if (enpass) {
            pop_bit(bitboards[side == white ? p : P], tt);
        }

        enpassant = no_sq;
        if (double_push) {
            enpassant = tt;
        }

        if (castling) {
            switch (target_square) {
                case g1: 
                    pop_bit(bitboards[R], h1);
                    set_bit(bitboards[R], f1);
                    break;
                case c1:
                    pop_bit(bitboards[R], a1);
                    set_bit(bitboards[R], d1);
                    break;
                case g8:
                    pop_bit(bitboards[r], h8);
                    set_bit(bitboards[r], f8);
                    break;
                case c8:
                    pop_bit(bitboards[r], a8);
                    set_bit(bitboards[r], d8);
                    break;
            }
        }

        castle &= castling_rights[source_square];
        castle &= castling_rights[target_square];

        memset(occupancies, 0ULL, 24);
        for (int pp = P; pp <= K; pp++) {
            occupancies[white] |= bitboards[pp]; 
        }
        for (int pp = p; pp <= k; pp++) {
            occupancies[black] |= bitboards[pp]; 
        }
        occupancies[both] = occupancies[white] | occupancies[black];

        side ^= 1;

        if (is_square_attacked(side == white ? get_lsb1_index(bitboards[k]) : get_lsb1_index(bitboards[K]), side)) {
            take_back();
            return 0;
        } else {
            return 1;
        }

    } else {
        if (get_move_capture(move))
            make_move(move, all_moves);
        else 
            return 0;
    }
} 
     
static inline void generate_moves(moves* move_list) {
    move_list->count = 0;

    int source_square;
    int target_square;
    U64 bitboard, attacks;

    for (int piece = P; piece <= k; piece++) {
        bitboard = bitboards[piece];

        // pawn moves and castles
        if (side == white) {
            if (piece == P) {
                // for each pawn piece check movements forward, attacks on black
                while (bitboard) {
                    source_square = get_lsb1_index(bitboard);
                    // go one up
                    target_square = source_square - 8;

                    // if target in bounds, and can move up
                    if (!(target_square < a8) && !get_bit(occupancies[both], target_square)) {
                        // if at the second top most row
                        if (source_square >= a7 && source_square <= h7) {
                            add_move(move_list, encode_move(source_square, target_square, piece, Q, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, R, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, B, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, N, 0, 0, 0, 0)); 
                        } else {
                            // no promotion
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0));
                            // if at the second row, can move twice
                            if (source_square >= a2 && source_square <= h2 && !get_bit(occupancies[both], target_square - 8)) {
                                add_move(move_list, encode_move(source_square, target_square-8, piece, 0, 0, 1, 0, 0));
                            }
                        }
                    }

                    attacks = pawn_attacks[side][source_square] & occupancies[black];
                    // for each pawn piece check attacks on black
                    while(attacks) {
                        target_square = get_lsb1_index(attacks);
                        // if capturing leads to promotion
                        if (source_square >= a7 && source_square <= h7) {
                            add_move(move_list, encode_move(source_square, target_square, piece, Q, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, R, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, B, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, N, 1, 0, 0, 0)); 
                        } else {
                            // no promotion
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0));    
                        }
         
                        pop_bit(attacks, target_square);
                    }

                    // check if can capture en passant
                    if (enpassant != no_sq) {
                        U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);
                        if (enpassant_attacks) {
                            int target_enpassant = get_lsb1_index(enpassant_attacks);
                            add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0));
                        }
                    }

                    pop_bit(bitboard, source_square);
                }
            }
            if (piece == K) {
                if (castle & wk) {
                    if (!get_bit(occupancies[both], f1) && !get_bit(occupancies[both], g1)) {
                        if (!is_square_attacked(e1, black) && !is_square_attacked(f1, black)) {
                            add_move(move_list, encode_move(e1, g1, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }

                if (castle & wq) {
                    if (!get_bit(occupancies[both], d1) && !get_bit(occupancies[both], c1) && !get_bit(occupancies[both], b1)) {
                        if (!is_square_attacked(e1, black) && !is_square_attacked(d1, black)) {
                            add_move(move_list, encode_move(e1, c1, piece, 0, 0, 0, 0, 1));
                        }
                    }
                }
            }
        } else {
            if (piece == p) {
                while (bitboard) {
                    source_square = get_lsb1_index(bitboard);
                    target_square = source_square + 8;

                    if (!(target_square > h1) && !get_bit(occupancies[both], target_square)) {
                        if (source_square >= a2 && source_square <= h2) {
                            add_move(move_list, encode_move(source_square, target_square, piece, q, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, r, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, b, 0, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, n, 0, 0, 0, 0)); 
                        } else {
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0)); 
                            if (source_square >= a7 && source_square <= h7 && !get_bit(occupancies[both], target_square + 8)) {
                                add_move(move_list, encode_move(source_square, target_square+8, piece, 0, 0, 1, 0, 0)); 
                            }
                        }
                    }

                    attacks = pawn_attacks[side][source_square] & occupancies[white];
                    while(attacks) {
                        target_square = get_lsb1_index(attacks);
                      
                        if (source_square >= a2 && source_square <= h2) {
                            add_move(move_list, encode_move(source_square, target_square, piece, q, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, r, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, b, 1, 0, 0, 0));
                            add_move(move_list, encode_move(source_square, target_square, piece, n, 1, 0, 0, 0)); 
                        } else {
                            add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0)); 
                        }
                        
                        pop_bit(attacks, target_square);
                    }

                    if (enpassant != no_sq) {
                        U64 enpassant_attacks = pawn_attacks[side][source_square] & (1ULL << enpassant);
                        if (enpassant_attacks) {
                            int target_enpassant = get_lsb1_index(enpassant_attacks);
                            add_move(move_list, encode_move(source_square, target_enpassant, piece, 0, 1, 0, 1, 0)); 
                        }
                    }

                    pop_bit(bitboard, source_square);
                }
            }
            if (piece == k) {
                if (castle & bk) {
                    if (!get_bit(occupancies[both], f8) && !get_bit(occupancies[both], g8)) {
                        if (!is_square_attacked(e8, white) && !is_square_attacked(f8, white)) {
                            add_move(move_list, encode_move(e8, g8, piece, 0, 0, 0, 0, 1)); 
                        }
                    }
                }

                if (castle & bq) {
                    if (!get_bit(occupancies[both], d8) && !get_bit(occupancies[both], c8) && !get_bit(occupancies[both], b8)) {
                        if (!is_square_attacked(e8, white) && !is_square_attacked(d8, white)) {
                            add_move(move_list, encode_move(e8, c8, piece, 0, 0, 0, 0, 1)); 
                        }
                    }
                }
            }
        }

        if ((side == white) ? piece == N : piece == n) {
            while (bitboard) {
                source_square = get_lsb1_index(bitboard);

                attacks = knight_attacks[source_square] & (side == white ? ~occupancies[white] : ~occupancies[black]);

                while (attacks) {
                    target_square = get_lsb1_index(attacks);

                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square)) {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0)); 
                    } else {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0)); 
                    }

                    pop_bit(attacks, target_square);
                }

                pop_bit(bitboard, source_square);
            }
        } 
        
        if ((side == white) ? piece == K : piece == k) {
            while (bitboard) {
                source_square = get_lsb1_index(bitboard);

                attacks = king_attacks[source_square] & (side == white ? ~occupancies[white] : ~occupancies[black]);

                while (attacks) {
                    target_square = get_lsb1_index(attacks);

                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square)) {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0)); 
                    } else {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0)); 
                    }

                    pop_bit(attacks, target_square);
                }

                pop_bit(bitboard, source_square);
            }
        } 

        if ((side == white) ? piece == B : piece == b) {
            while (bitboard) {
                source_square = get_lsb1_index(bitboard);

                attacks = get_bishop_attacks(source_square, occupancies[both]) & (side == white ? ~occupancies[white] : ~occupancies[black]);

                while (attacks) {
                    target_square = get_lsb1_index(attacks);

                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square)) {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0)); 
                    } else {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0)); 
                    }

                    pop_bit(attacks, target_square);
                }

                pop_bit(bitboard, source_square);
            }
        } 

        if ((side == white) ? piece == R : piece == r) {
            while (bitboard) {
                source_square = get_lsb1_index(bitboard);

                attacks = get_rook_attacks(source_square, occupancies[both]) & (side == white ? ~occupancies[white] : ~occupancies[black]);

                while (attacks) {
                    target_square = get_lsb1_index(attacks);

                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square)) {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0)); 
                    } else {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0)); 
                    }

                    pop_bit(attacks, target_square);
                }

                pop_bit(bitboard, source_square);
            }
        } 

        if ((side == white) ? piece == Q : piece == q) {
            while (bitboard) {
                source_square = get_lsb1_index(bitboard);

                attacks = get_queen_attacks(source_square, occupancies[both]) & (side == white ? ~occupancies[white] : ~occupancies[black]);

                while (attacks) {
                    target_square = get_lsb1_index(attacks);

                    if (!get_bit(((side == white) ? occupancies[black] : occupancies[white]), target_square)) {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 0, 0, 0, 0)); 
                    } else {
                        add_move(move_list, encode_move(source_square, target_square, piece, 0, 1, 0, 0, 0)); 
                    }

                    pop_bit(attacks, target_square);
                }

                pop_bit(bitboard, source_square);
            }
        } 
    }
}

long long int get_time_ms() {
    #ifdef WIN64
        return GetTickCount();
    #else
        struct timeval time_value;
        gettimeofday(&time_value, NULL);
        return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
    #endif
}

long nodes = 0;
long local_nodes = 0;

static inline void perft_driver(int depth) {

    if (depth == 0) {
        nodes++;
        local_nodes++;
        return;
    }

    moves moves;

    generate_moves(&moves);

    for (int i=0; i < moves.count; i++) {

        copy_board();

        if (!make_move(moves.moves[i], all_moves))
            continue;

        perft_driver(depth-1);

        take_back();
    }
}

void perft_test(int depth) {

    printf("\n Performance test\n\n");
    moves moves;

    generate_moves(&moves);

    int start = get_time_ms();

    for (int i=0; i < moves.count; i++) {
        
        copy_board();

        if (!make_move(moves.moves[i], all_moves))
            continue;

        long sum_nodes = nodes;

        perft_driver(depth-1);
        
        long old_nodes = nodes - sum_nodes;

        take_back();

        // print_move(moves.moves[i]);
        printf("%s%s%c: %ld\n", square_to_coordinates[get_move_source(moves.moves[i])]
                         , square_to_coordinates[get_move_target(moves.moves[i])]
                         , promoted_pieces[get_move_promoted(moves.moves[i])]
                         , old_nodes);
    }

    printf("\n Depth: %d\n", depth);
    printf(" Nodes: %ld\n", nodes);
    printf(" Time: %lld\n\n", get_time_ms()-start);
}

// extactly matches PNBRQKpnbrqk
// in order to index by the piece enum
int material_score[12] = {
    100,
    300,
    350,
    500,
    1000,
    10000,
    -100,
    -300,
    -350,
    -500,
    -1000,
    -10000,
};

const int pawn_score[64] = {
    90,  90,  90,  90,  90,  90,  90,  90,
    30,  30,  30,  40,  40,  30,  30,  30,
    20,  20,  20,  30,  30,  30,  20,  20,
    10,  10,  10,  20,  20,  10,  10,  10,
     5,   5,  10,  20,  20,   5,   5,   5,
     0,   0,   0,   5,   5,   0,   0,   0,
     0,   0,   0, -10, -10,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0
};

// knight positional score
const int knight_score[64] = {
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5,   0,   0,  10,  10,   0,   0,  -5,
    -5,   5,  20,  20,  20,  20,   5,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,  10,  20,  30,  30,  20,  10,  -5,
    -5,   5,  20,  10,  10,  20,   5,  -5,
    -5,   0,   0,   0,   0,   0,   0,  -5,
    -5, -10,   0,   0,   0,   0, -10,  -5
};

// bishop positional score
const int bishop_score[64] = {
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   0,  10,  10,   0,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,  10,   0,   0,   0,   0,  10,   0,
     0,  30,   0,   0,   0,   0,  30,   0,
     0,   0, -10,   0,   0, -10,   0,   0

};

// rook positional score
const int rook_score[64] ={
    50,  50,  50,  50,  50,  50,  50,  50,
    50,  50,  50,  50,  50,  50,  50,  50,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,  10,  20,  20,  10,   0,   0,
     0,   0,   0,  20,  20,   0,   0,   0

};

// king positional score
const int king_score[64] = {
     0,   0,   0,   0,   0,   0,   0,   0,
     0,   0,   5,   5,   5,   5,   0,   0,
     0,   5,   5,  10,  10,   5,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   5,  10,  20,  20,  10,   5,   0,
     0,   0,   5,  10,  10,   5,   0,   0,
     0,   5,   5,  -5,  -5,   0,   5,   0,
     0,   0,   5,   0, -15,   0,  10,   0
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

static inline int evaluate() {

    int score = 0;

    U64 bitboard;

    int piece, square;

    for (int pp = P; pp <= k; pp++) {

        // get all of the bits with the piece turned on
        bitboard = bitboards[pp];

        while(bitboard) {
            piece = pp;

            square = get_lsb1_index(bitboard);

            score += material_score[piece];
            
            switch(piece) {
                case P: score += pawn_score[square]; break;
                case N: score += knight_score[square]; break;
                case B: score += bishop_score[square]; break;
                case R: score += rook_score[square]; break;
                case K: score += king_score[square]; break;

                case p: score -= pawn_score[mirror_score[square]]; break;
                case n: score -= knight_score[mirror_score[square]]; break;
                case b: score -= bishop_score[mirror_score[square]]; break;
                case r: score -= rook_score[mirror_score[square]]; break;
                case k: score -= king_score[mirror_score[square]]; break;
            }

            pop_bit(bitboard, square);
        }
    }

    return (side == white) ? score : -score;
}

static int mvv_lva[12][12] = {
 	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600,

	105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605,
	104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604,
	103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603,
	102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602,
	101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601,
	100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600
};

#define max_ply 246

// like the stockfishes
int killer_moves[2][max_ply];
int history_moves[12][64];

// pv length
int pv_length[max_ply];
int pv_table[max_ply][max_ply];

int follow_pv, score_pv;

// half move counter
int ply;

int best_move;

static inline void enable_pv_scoring(moves* moves) {
    follow_pv = 0;    

    for (int i = 0; i < moves->count; i++) {
        if (pv_table[0][ply] == moves->moves[i]) {
            // enable move scoring
            score_pv = 1;

            // enable following pv
            follow_pv = 1;
        }
    }
}

/*
    How we sort different moves:
    1. PV move
    2. captures using MVV/LVA
    3. 1st killer move
    4. 2nd killer move
    5. History moves
    6. Rest of the moves
*/
static inline int score_move(int move) {
    
    // if move scoring is allowed
    if (score_pv) {
        // make sure i
        if (pv_table[0][ply] == move) {           
            score_pv = 0;
            return 20000;
        }
    }

    if (get_move_capture(move)) {
        int target_piece = P;

        int start_piece, end_piece;

        // go over your color's pieces
        if (side == white) { start_piece = p; end_piece =  k; } 
        else { start_piece = P; end_piece = K; }

        for (int pp = start_piece; pp <= end_piece; pp++) {
            // get the piece on the target
            if (get_bit(bitboards[pp], get_move_target(move))) {
                target_piece = pp;
                break;
            }
        }
        return mvv_lva[get_move_piece(move)][target_piece] + 10000;
    } else {
        // score 1st, 2d killer move
        if (killer_moves[0][ply] == move)
            return 9000;
        // score history move
        else if (killer_moves[1][ply] == move)
            return 8000;
        else 
            return history_moves[get_move_piece(move)][get_move_target(move)];
    }
    return 0;
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

static inline int sort_moves(moves* moves) {
    int move_scores[moves->count];

    // fill array based on scores
    for (int i = 0; i < moves->count; i++) {
        move_scores[i] = score_move(moves->moves[i]);
    }

    // 
    for (int i = 0; i < moves->count; i++) {
        for (int j = i+1; j < moves->count; j++) {
            if (move_scores[i] < move_scores[j]) {
                int temp_score = move_scores[i];
                move_scores[i] = move_scores[j];
                move_scores[j] = temp_score;

                int temp_move = moves->moves[i];
                moves->moves[i] = moves->moves[j];
                moves->moves[j] = temp_move;
            }
        }
    }
}

static inline int quiescence(int alpha, int beta) {
    nodes++;    

    int eval = evaluate();
    // fail-hard beta cutoff
    if (eval >= beta) {
        // move fails high
        return beta;
    } 

    // found a better move
    if (eval > alpha) {
        alpha = eval;
    }

    moves moves;

    generate_moves(&moves);

    sort_moves(&moves);

    for (int count = 0; count < moves.count; count++) {
        copy_board();

        ply++;

        if (make_move(moves.moves[count], only_captures) == 0) {
            // if make move is illegal
            ply--;
            continue;
        }

        int score = -quiescence(-beta, -alpha);

        ply--;
        take_back()
        // fail-hard beta cutoff
        if (score >= beta) {
            // move fails high
            return beta;
        } 

        // found a better move
        if (score > alpha) {
            alpha = score;
        }
    }
    return alpha;
}

const int full_depth_moves = 4;
const int reduction_limit = 3;

static inline int negamax(int alpha, int beta, int depth) {
    int found_pv = 0;

    // init pv length
    pv_length[ply] = ply;

    if (depth == 0) { 
        return quiescence(alpha, beta);
    }

    // we are too deep, theres an overflow on ply
    if (ply > max_ply - 1) {
        return evaluate();
    }

    nodes++;

    // check if opponent is in check
    int in_check = is_square_attacked((side==white) ? get_lsb1_index(bitboards[K]) : get_lsb1_index(bitboards[k]), side ^ 1);
    // search deeper if in check
    if (in_check) depth++;

    int legal_moves = 0;
 
    // null move pruning
    if (depth >= 3 && !in_check && ply) {
        copy_board();
        side ^= 1;
        enpassant = no_sq;

        int score = -negamax(-beta, -beta + 1, depth - 3);

        take_back();

        if (score >= beta) 
            return beta;
    }

    moves moves;

    generate_moves(&moves);

    if (follow_pv) {
        enable_pv_scoring(&moves);
    }

    sort_moves(&moves);

    int moves_searched = 0;

    for (int count = 0; count < moves.count; count++) {
        copy_board();

        ply++;

        if (make_move(moves.moves[count], all_moves) == 0) {
            // if make move is illegal
            ply--;
            continue;
        }

        legal_moves++;

        int score;

        // PVS & LMR
        if (found_pv) {
            score = -negamax(-alpha - 1, - alpha, depth - 1);

            if (score > alpha && score < beta) {
                // if failed to be proven bad
                score = -negamax(-beta, -alpha, depth - 1);
            }
        } else {
            if (moves_searched == 0) {
                score = -negamax(-beta, -alpha, depth-1);
            } else {
                if (moves_searched >= full_depth_moves &&
                    depth >= reduction_limit && 
                    !in_check && 
                    !get_move_capture(moves.moves[count]) &&
                    !get_move_promoted(moves.moves[count])) {
                    score = -negamax(-alpha - 1, -alpha, depth - 2);
                } else {
                    score = alpha + 1;
                }

                if (score > alpha) {
                    score = -negamax(-alpha - 1, -alpha, depth - 1);
                    if (score > alpha && score < beta) {
                        score = -negamax(-beta, -alpha, depth - 1);
                    }
                }
            }
        }

        ply--;
        take_back()

        moves_searched++;

        // fail-hard beta cutoff
        if (score >= beta) {
            if (!get_move_capture(moves.moves[count]))  {
                // remember two moves
                killer_moves[1][ply] = killer_moves[0][ply];
                killer_moves[0][ply] = moves.moves[count];
            }
            // move fails high
            return beta;
        }

        // found a better move
        if (score > alpha) {

            if (!get_move_capture(moves.moves[count]))  {
                history_moves[get_move_piece(moves.moves[count])][get_move_target(moves.moves[count])] += depth;
            }
            alpha = score;

            found_pv = 1;

            pv_table[ply][ply] = moves.moves[count];

            for (int next_ply = ply+1; next_ply < pv_length[ply+1]; next_ply++) {
                // copy move from deepetr ply into current plys line
                pv_table[ply][next_ply] = pv_table[ply+1][next_ply];
            }

            // adjust pv length
            pv_length[ply] = pv_length[ply+1];
        }
    }

    if (legal_moves == 0) {

        if (in_check) {
            // less than -50000 for it to be in alpha beta bounds
            // add ply for it to be able to checkmate in bigger depths
            return -49000 + ply;
        } else {
            return 0;
        }
    }

    // when move fails low
    return alpha;
}

// search for best move
void search_position(int depth) {
    
    int score = 0;
    nodes = 0;
    follow_pv = 0;
    score_pv = 0;
    
    memset(killer_moves, 0, sizeof(killer_moves));
    memset(history_moves, 0, sizeof(history_moves));
    memset(pv_length, 0, sizeof(pv_length));
    memset(pv_table, 0, sizeof(pv_table));

    int alpha = -50000;
    int beta = 50000;

    for (int current_depth = 1; current_depth <= depth; current_depth++) {
        follow_pv = 1;

        score = negamax(alpha, beta, current_depth);

        if (score <= alpha || score >= beta) {
            alpha = -50000;
            beta = 50000;
            continue;
        }
        alpha = score - 50;
        beta = score + 50;


        printf("info score cp %d depth %d nodes %ld pv ", score, current_depth, nodes);

        for (int count=0; count < pv_length[0]; count++) {
            print_move(pv_table[0][count]);
            printf("  ");
        }
        printf("\n");
    }

    // best move placeholders
    printf("bestmove ");
    print_move(pv_table[0][0]);
    printf("\n");
}


//////////////////////////////////////////////////////////////////////////////////////////////////////////

int parse_move(char* move_string) {

    moves moves;

    generate_moves(&moves);

    // coordinate e.g. a1b2 to square int 
    int source_square = (move_string[0]-'a') + (8 - (move_string[1]-'0')) * 8;    
    int target_square = (move_string[2]-'a') + (8 - (move_string[3]-'0')) * 8;    

    for (int i=0; i < moves.count; i++) {
        int move = moves.moves[i];

        if (source_square == get_move_source(move) && target_square == get_move_target(move)) {
            int promoted_piece = get_move_promoted(move);

            if (promoted_piece) {
                if ((promoted_piece == Q || promoted_piece == q) && move_string[4] == 'q')
                return move;

                if ((promoted_piece == R || promoted_piece == r) && move_string[4] == 'r')
                    return move;

                if ((promoted_piece == B || promoted_piece == b) && move_string[4] == 'b')
                    return move;

                if ((promoted_piece == N || promoted_piece == n) && move_string[4] == 'n')
                    return move;

                // even if the promotion doesnt make sense
                continue;
            }
            // legal move
            return move;
        }
    }

    // illegal move
    return 0;
}

void parse_position(char *command) {

    // position is 8 chars long, move to next string
    command += 9;

    char *current_char = command;

    // parse startpos
    if (strncmp(command, "startpos", 8) == 0) 
        parse_fen(start_position);
    else {
        printf("%s\n", current_char);

        current_char = strstr(command, "fen");

        // if no fen present in command
        if (current_char == NULL) {
            parse_fen(start_position);
        } else {
            // fen is 3 chars long, move to next string
            current_char += 4;

            parse_fen(current_char);
        }
    }

    current_char = strstr(command, "moves");

    if (current_char != NULL) {
        // moves is 5 chars long
        current_char += 6;

        while (*current_char) {

            int move = parse_move(current_char);

            if (move == 0) {
                break;
            }

            make_move(move, all_moves);
            // each move is 4 chars long
            current_char += 5;
        }
    }
    print_board();
    // printf("%s\n", current_char);
}

void parse_go(char *command) {
    int depth = -1;

    char* current_depth = NULL;

    if (current_depth = strstr(command, "depth")) {
        // depth is 5 chars long
        depth = atoi(current_depth + 6);
        search_position(depth);
    } else if (current_depth = strstr(command, "perft")) {
        depth = atoi(current_depth + 6);
        perft_test(depth);
    } else {
        // time controls placeholder
        search_position(2);
    } 
    // printf("depth: %d\n", depth); 
}

void uci_loop() {
    // reset stdin, stdout
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    // define input buffer
    char input[2000];

    printf("id name hhc\n");
    printf("id name Code hemihemichess\n");
    printf("uciok\n");

    while(1) {
        //reset user input
        memset(input, 0, sizeof(input));

        fflush(stdout);

        // get user input from stdin
        if (!fgets(input, 2000, stdin))
            continue;

        if (input[0] == '\n')
            continue;

        if (strncmp(input, "isready", 7) == 0) {
            printf("readyok\n");
        }

        else if (strncmp(input, "position", 8) == 0) {
            parse_position(input);
        }

        else if (strncmp(input, "ucinewgame", 10) == 0) {
            parse_position("position startpos");
        }

        else if (strncmp(input, "go", 2) == 0) {
            parse_go(input);
        }

        else if (strncmp(input, "quit", 4) == 0) {
            break;
        }

        else if (strncmp(input, "uci", 3) == 0) {
            printf("id name hhc\n");
            printf("id name Code hemihemichess\n");
            printf("uciok\n");
        }
    }
}

int main() {
    // init all
    init_all();
    
    int debug = 0;

    if (debug) {
        // parse_fen("rnbqkbnr/pppppppp/8/8/8/4P3/PPPP1PPP/RNBQKBNR b KQkq - 0 1 ");
        parse_fen(tricky_position);
        print_board();
        search_position(7); 
    } else {
        uci_loop();
    }

    return 0;
}