#define NOMINMAX
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <time.h> 
#include <unistd.h>
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
#define repetitions "2r3k1/R7/8/1R6/8/8/P4KPP/8 w - - 0 40 "

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
    if (get_move_promoted(move)) 
        printf("%s%s%c", square_to_coordinates[get_move_source(move)], square_to_coordinates[get_move_target(move)], promoted_pieces[get_move_promoted(move)]);
    else
        printf("%s%s", square_to_coordinates[get_move_source(move)], square_to_coordinates[get_move_target(move)]);
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

// almost unique position identifier aka position key
U64 hash_key = 0ULL;

U64 repetition_table[1000];
int repetition_index = 0;

// half move counter
int ply;
int half_moves = 0;
int fifty;

int quit = 0;
int movestogo = 30;
int movetime = -1;
int _time = -1;
int inc = 0;
int starttime = 0;
int stoptime = 0;
int timeset = 0;
int stopped = 0;
int in_opening = 1;

void reset_time_control() {
    in_opening = 1;
    quit = 0;
    movestogo = 30;
    movetime = -1;
    _time = -1;
    inc = 0;
    starttime = 0;
    stoptime = 0;
    timeset = 0;
    stopped = 0;
}

unsigned short swap_endian_u16(unsigned short x) {
    return (x >> 8) | (x << 8);
}
unsigned int swap_endian_u32(unsigned int x) {
    x = (x >> 24) |
        ((x << 8) & 0x00ff0000) |
        ((x >> 8) & 0x0000ff00) |
        (x << 24);
    return x;
}
U64 swap_endian_U64(U64 x) {
    x = (x >> 56) |
        ((x << 40) & 0x00ff000000000000) |
        ((x << 24) & 0x0000ff0000000000) |
        ((x << 8)  & 0x000000ff00000000) |
        ((x >> 8)  & 0x00000000ff000000) |
        ((x >> 24) & 0x0000000000ff0000) |
        ((x >> 40) & 0x000000000000ff00) |
        (x << 56);
    return x;
}




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

const U64 Random64[781] = {
   0x9D39247E33776D41, 0x2AF7398005AAA5C7, 0x44DB015024623547, 0x9C15F73E62A76AE2,
   0x75834465489C0C89, 0x3290AC3A203001BF, 0x0FBBAD1F61042279, 0xE83A908FF2FB60CA,
   0x0D7E765D58755C10, 0x1A083822CEAFE02D, 0x9605D5F0E25EC3B0, 0xD021FF5CD13A2ED5,
   0x40BDF15D4A672E32, 0x011355146FD56395, 0x5DB4832046F3D9E5, 0x239F8B2D7FF719CC,
   0x05D1A1AE85B49AA1, 0x679F848F6E8FC971, 0x7449BBFF801FED0B, 0x7D11CDB1C3B7ADF0,
   0x82C7709E781EB7CC, 0xF3218F1C9510786C, 0x331478F3AF51BBE6, 0x4BB38DE5E7219443,
   0xAA649C6EBCFD50FC, 0x8DBD98A352AFD40B, 0x87D2074B81D79217, 0x19F3C751D3E92AE1,
   0xB4AB30F062B19ABF, 0x7B0500AC42047AC4, 0xC9452CA81A09D85D, 0x24AA6C514DA27500,
   0x4C9F34427501B447, 0x14A68FD73C910841, 0xA71B9B83461CBD93, 0x03488B95B0F1850F,
   0x637B2B34FF93C040, 0x09D1BC9A3DD90A94, 0x3575668334A1DD3B, 0x735E2B97A4C45A23,
   0x18727070F1BD400B, 0x1FCBACD259BF02E7, 0xD310A7C2CE9B6555, 0xBF983FE0FE5D8244,
   0x9F74D14F7454A824, 0x51EBDC4AB9BA3035, 0x5C82C505DB9AB0FA, 0xFCF7FE8A3430B241,
   0x3253A729B9BA3DDE, 0x8C74C368081B3075, 0xB9BC6C87167C33E7, 0x7EF48F2B83024E20,
   0x11D505D4C351BD7F, 0x6568FCA92C76A243, 0x4DE0B0F40F32A7B8, 0x96D693460CC37E5D,
   0x42E240CB63689F2F, 0x6D2BDCDAE2919661, 0x42880B0236E4D951, 0x5F0F4A5898171BB6,
   0x39F890F579F92F88, 0x93C5B5F47356388B, 0x63DC359D8D231B78, 0xEC16CA8AEA98AD76,
   0x5355F900C2A82DC7, 0x07FB9F855A997142, 0x5093417AA8A7ED5E, 0x7BCBC38DA25A7F3C,
   0x19FC8A768CF4B6D4, 0x637A7780DECFC0D9, 0x8249A47AEE0E41F7, 0x79AD695501E7D1E8,
   0x14ACBAF4777D5776, 0xF145B6BECCDEA195, 0xDABF2AC8201752FC, 0x24C3C94DF9C8D3F6,
   0xBB6E2924F03912EA, 0x0CE26C0B95C980D9, 0xA49CD132BFBF7CC4, 0xE99D662AF4243939,
   0x27E6AD7891165C3F, 0x8535F040B9744FF1, 0x54B3F4FA5F40D873, 0x72B12C32127FED2B,
   0xEE954D3C7B411F47, 0x9A85AC909A24EAA1, 0x70AC4CD9F04F21F5, 0xF9B89D3E99A075C2,
   0x87B3E2B2B5C907B1, 0xA366E5B8C54F48B8, 0xAE4A9346CC3F7CF2, 0x1920C04D47267BBD,
   0x87BF02C6B49E2AE9, 0x092237AC237F3859, 0xFF07F64EF8ED14D0, 0x8DE8DCA9F03CC54E,
   0x9C1633264DB49C89, 0xB3F22C3D0B0B38ED, 0x390E5FB44D01144B, 0x5BFEA5B4712768E9,
   0x1E1032911FA78984, 0x9A74ACB964E78CB3, 0x4F80F7A035DAFB04, 0x6304D09A0B3738C4,
   0x2171E64683023A08, 0x5B9B63EB9CEFF80C, 0x506AACF489889342, 0x1881AFC9A3A701D6,
   0x6503080440750644, 0xDFD395339CDBF4A7, 0xEF927DBCF00C20F2, 0x7B32F7D1E03680EC,
   0xB9FD7620E7316243, 0x05A7E8A57DB91B77, 0xB5889C6E15630A75, 0x4A750A09CE9573F7,
   0xCF464CEC899A2F8A, 0xF538639CE705B824, 0x3C79A0FF5580EF7F, 0xEDE6C87F8477609D,
   0x799E81F05BC93F31, 0x86536B8CF3428A8C, 0x97D7374C60087B73, 0xA246637CFF328532,
   0x043FCAE60CC0EBA0, 0x920E449535DD359E, 0x70EB093B15B290CC, 0x73A1921916591CBD,
   0x56436C9FE1A1AA8D, 0xEFAC4B70633B8F81, 0xBB215798D45DF7AF, 0x45F20042F24F1768,
   0x930F80F4E8EB7462, 0xFF6712FFCFD75EA1, 0xAE623FD67468AA70, 0xDD2C5BC84BC8D8FC,
   0x7EED120D54CF2DD9, 0x22FE545401165F1C, 0xC91800E98FB99929, 0x808BD68E6AC10365,
   0xDEC468145B7605F6, 0x1BEDE3A3AEF53302, 0x43539603D6C55602, 0xAA969B5C691CCB7A,
   0xA87832D392EFEE56, 0x65942C7B3C7E11AE, 0xDED2D633CAD004F6, 0x21F08570F420E565,
   0xB415938D7DA94E3C, 0x91B859E59ECB6350, 0x10CFF333E0ED804A, 0x28AED140BE0BB7DD,
   0xC5CC1D89724FA456, 0x5648F680F11A2741, 0x2D255069F0B7DAB3, 0x9BC5A38EF729ABD4,
   0xEF2F054308F6A2BC, 0xAF2042F5CC5C2858, 0x480412BAB7F5BE2A, 0xAEF3AF4A563DFE43,
   0x19AFE59AE451497F, 0x52593803DFF1E840, 0xF4F076E65F2CE6F0, 0x11379625747D5AF3,
   0xBCE5D2248682C115, 0x9DA4243DE836994F, 0x066F70B33FE09017, 0x4DC4DE189B671A1C,
   0x51039AB7712457C3, 0xC07A3F80C31FB4B4, 0xB46EE9C5E64A6E7C, 0xB3819A42ABE61C87,
   0x21A007933A522A20, 0x2DF16F761598AA4F, 0x763C4A1371B368FD, 0xF793C46702E086A0,
   0xD7288E012AEB8D31, 0xDE336A2A4BC1C44B, 0x0BF692B38D079F23, 0x2C604A7A177326B3,
   0x4850E73E03EB6064, 0xCFC447F1E53C8E1B, 0xB05CA3F564268D99, 0x9AE182C8BC9474E8,
   0xA4FC4BD4FC5558CA, 0xE755178D58FC4E76, 0x69B97DB1A4C03DFE, 0xF9B5B7C4ACC67C96,
   0xFC6A82D64B8655FB, 0x9C684CB6C4D24417, 0x8EC97D2917456ED0, 0x6703DF9D2924E97E,
   0xC547F57E42A7444E, 0x78E37644E7CAD29E, 0xFE9A44E9362F05FA, 0x08BD35CC38336615,
   0x9315E5EB3A129ACE, 0x94061B871E04DF75, 0xDF1D9F9D784BA010, 0x3BBA57B68871B59D,
   0xD2B7ADEEDED1F73F, 0xF7A255D83BC373F8, 0xD7F4F2448C0CEB81, 0xD95BE88CD210FFA7,
   0x336F52F8FF4728E7, 0xA74049DAC312AC71, 0xA2F61BB6E437FDB5, 0x4F2A5CB07F6A35B3,
   0x87D380BDA5BF7859, 0x16B9F7E06C453A21, 0x7BA2484C8A0FD54E, 0xF3A678CAD9A2E38C,
   0x39B0BF7DDE437BA2, 0xFCAF55C1BF8A4424, 0x18FCF680573FA594, 0x4C0563B89F495AC3,
   0x40E087931A00930D, 0x8CFFA9412EB642C1, 0x68CA39053261169F, 0x7A1EE967D27579E2,
   0x9D1D60E5076F5B6F, 0x3810E399B6F65BA2, 0x32095B6D4AB5F9B1, 0x35CAB62109DD038A,
   0xA90B24499FCFAFB1, 0x77A225A07CC2C6BD, 0x513E5E634C70E331, 0x4361C0CA3F692F12,
   0xD941ACA44B20A45B, 0x528F7C8602C5807B, 0x52AB92BEB9613989, 0x9D1DFA2EFC557F73,
   0x722FF175F572C348, 0x1D1260A51107FE97, 0x7A249A57EC0C9BA2, 0x04208FE9E8F7F2D6,
   0x5A110C6058B920A0, 0x0CD9A497658A5698, 0x56FD23C8F9715A4C, 0x284C847B9D887AAE,
   0x04FEABFBBDB619CB, 0x742E1E651C60BA83, 0x9A9632E65904AD3C, 0x881B82A13B51B9E2,
   0x506E6744CD974924, 0xB0183DB56FFC6A79, 0x0ED9B915C66ED37E, 0x5E11E86D5873D484,
   0xF678647E3519AC6E, 0x1B85D488D0F20CC5, 0xDAB9FE6525D89021, 0x0D151D86ADB73615,
   0xA865A54EDCC0F019, 0x93C42566AEF98FFB, 0x99E7AFEABE000731, 0x48CBFF086DDF285A,
   0x7F9B6AF1EBF78BAF, 0x58627E1A149BBA21, 0x2CD16E2ABD791E33, 0xD363EFF5F0977996,
   0x0CE2A38C344A6EED, 0x1A804AADB9CFA741, 0x907F30421D78C5DE, 0x501F65EDB3034D07,
   0x37624AE5A48FA6E9, 0x957BAF61700CFF4E, 0x3A6C27934E31188A, 0xD49503536ABCA345,
   0x088E049589C432E0, 0xF943AEE7FEBF21B8, 0x6C3B8E3E336139D3, 0x364F6FFA464EE52E,
   0xD60F6DCEDC314222, 0x56963B0DCA418FC0, 0x16F50EDF91E513AF, 0xEF1955914B609F93,
   0x565601C0364E3228, 0xECB53939887E8175, 0xBAC7A9A18531294B, 0xB344C470397BBA52,
   0x65D34954DAF3CEBD, 0xB4B81B3FA97511E2, 0xB422061193D6F6A7, 0x071582401C38434D,
   0x7A13F18BBEDC4FF5, 0xBC4097B116C524D2, 0x59B97885E2F2EA28, 0x99170A5DC3115544,
   0x6F423357E7C6A9F9, 0x325928EE6E6F8794, 0xD0E4366228B03343, 0x565C31F7DE89EA27,
   0x30F5611484119414, 0xD873DB391292ED4F, 0x7BD94E1D8E17DEBC, 0xC7D9F16864A76E94,
   0x947AE053EE56E63C, 0xC8C93882F9475F5F, 0x3A9BF55BA91F81CA, 0xD9A11FBB3D9808E4,
   0x0FD22063EDC29FCA, 0xB3F256D8ACA0B0B9, 0xB03031A8B4516E84, 0x35DD37D5871448AF,
   0xE9F6082B05542E4E, 0xEBFAFA33D7254B59, 0x9255ABB50D532280, 0xB9AB4CE57F2D34F3,
   0x693501D628297551, 0xC62C58F97DD949BF, 0xCD454F8F19C5126A, 0xBBE83F4ECC2BDECB,
   0xDC842B7E2819E230, 0xBA89142E007503B8, 0xA3BC941D0A5061CB, 0xE9F6760E32CD8021,
   0x09C7E552BC76492F, 0x852F54934DA55CC9, 0x8107FCCF064FCF56, 0x098954D51FFF6580,
   0x23B70EDB1955C4BF, 0xC330DE426430F69D, 0x4715ED43E8A45C0A, 0xA8D7E4DAB780A08D,
   0x0572B974F03CE0BB, 0xB57D2E985E1419C7, 0xE8D9ECBE2CF3D73F, 0x2FE4B17170E59750,
   0x11317BA87905E790, 0x7FBF21EC8A1F45EC, 0x1725CABFCB045B00, 0x964E915CD5E2B207,
   0x3E2B8BCBF016D66D, 0xBE7444E39328A0AC, 0xF85B2B4FBCDE44B7, 0x49353FEA39BA63B1,
   0x1DD01AAFCD53486A, 0x1FCA8A92FD719F85, 0xFC7C95D827357AFA, 0x18A6A990C8B35EBD,
   0xCCCB7005C6B9C28D, 0x3BDBB92C43B17F26, 0xAA70B5B4F89695A2, 0xE94C39A54A98307F,
   0xB7A0B174CFF6F36E, 0xD4DBA84729AF48AD, 0x2E18BC1AD9704A68, 0x2DE0966DAF2F8B1C,
   0xB9C11D5B1E43A07E, 0x64972D68DEE33360, 0x94628D38D0C20584, 0xDBC0D2B6AB90A559,
   0xD2733C4335C6A72F, 0x7E75D99D94A70F4D, 0x6CED1983376FA72B, 0x97FCAACBF030BC24,
   0x7B77497B32503B12, 0x8547EDDFB81CCB94, 0x79999CDFF70902CB, 0xCFFE1939438E9B24,
   0x829626E3892D95D7, 0x92FAE24291F2B3F1, 0x63E22C147B9C3403, 0xC678B6D860284A1C,
   0x5873888850659AE7, 0x0981DCD296A8736D, 0x9F65789A6509A440, 0x9FF38FED72E9052F,
   0xE479EE5B9930578C, 0xE7F28ECD2D49EECD, 0x56C074A581EA17FE, 0x5544F7D774B14AEF,
   0x7B3F0195FC6F290F, 0x12153635B2C0CF57, 0x7F5126DBBA5E0CA7, 0x7A76956C3EAFB413,
   0x3D5774A11D31AB39, 0x8A1B083821F40CB4, 0x7B4A38E32537DF62, 0x950113646D1D6E03,
   0x4DA8979A0041E8A9, 0x3BC36E078F7515D7, 0x5D0A12F27AD310D1, 0x7F9D1A2E1EBE1327,
   0xDA3A361B1C5157B1, 0xDCDD7D20903D0C25, 0x36833336D068F707, 0xCE68341F79893389,
   0xAB9090168DD05F34, 0x43954B3252DC25E5, 0xB438C2B67F98E5E9, 0x10DCD78E3851A492,
   0xDBC27AB5447822BF, 0x9B3CDB65F82CA382, 0xB67B7896167B4C84, 0xBFCED1B0048EAC50,
   0xA9119B60369FFEBD, 0x1FFF7AC80904BF45, 0xAC12FB171817EEE7, 0xAF08DA9177DDA93D,
   0x1B0CAB936E65C744, 0xB559EB1D04E5E932, 0xC37B45B3F8D6F2BA, 0xC3A9DC228CAAC9E9,
   0xF3B8B6675A6507FF, 0x9FC477DE4ED681DA, 0x67378D8ECCEF96CB, 0x6DD856D94D259236,
   0xA319CE15B0B4DB31, 0x073973751F12DD5E, 0x8A8E849EB32781A5, 0xE1925C71285279F5,
   0x74C04BF1790C0EFE, 0x4DDA48153C94938A, 0x9D266D6A1CC0542C, 0x7440FB816508C4FE,
   0x13328503DF48229F, 0xD6BF7BAEE43CAC40, 0x4838D65F6EF6748F, 0x1E152328F3318DEA,
   0x8F8419A348F296BF, 0x72C8834A5957B511, 0xD7A023A73260B45C, 0x94EBC8ABCFB56DAE,
   0x9FC10D0F989993E0, 0xDE68A2355B93CAE6, 0xA44CFE79AE538BBE, 0x9D1D84FCCE371425,
   0x51D2B1AB2DDFB636, 0x2FD7E4B9E72CD38C, 0x65CA5B96B7552210, 0xDD69A0D8AB3B546D,
   0x604D51B25FBF70E2, 0x73AA8A564FB7AC9E, 0x1A8C1E992B941148, 0xAAC40A2703D9BEA0,
   0x764DBEAE7FA4F3A6, 0x1E99B96E70A9BE8B, 0x2C5E9DEB57EF4743, 0x3A938FEE32D29981,
   0x26E6DB8FFDF5ADFE, 0x469356C504EC9F9D, 0xC8763C5B08D1908C, 0x3F6C6AF859D80055,
   0x7F7CC39420A3A545, 0x9BFB227EBDF4C5CE, 0x89039D79D6FC5C5C, 0x8FE88B57305E2AB6,
   0xA09E8C8C35AB96DE, 0xFA7E393983325753, 0xD6B6D0ECC617C699, 0xDFEA21EA9E7557E3,
   0xB67C1FA481680AF8, 0xCA1E3785A9E724E5, 0x1CFC8BED0D681639, 0xD18D8549D140CAEA,
   0x4ED0FE7E9DC91335, 0xE4DBF0634473F5D2, 0x1761F93A44D5AEFE, 0x53898E4C3910DA55,
   0x734DE8181F6EC39A, 0x2680B122BAA28D97, 0x298AF231C85BAFAB, 0x7983EED3740847D5,
   0x66C1A2A1A60CD889, 0x9E17E49642A3E4C1, 0xEDB454E7BADC0805, 0x50B704CAB602C329,
   0x4CC317FB9CDDD023, 0x66B4835D9EAFEA22, 0x219B97E26FFC81BD, 0x261E4E4C0A333A9D,
   0x1FE2CCA76517DB90, 0xD7504DFA8816EDBB, 0xB9571FA04DC089C8, 0x1DDC0325259B27DE,
   0xCF3F4688801EB9AA, 0xF4F5D05C10CAB243, 0x38B6525C21A42B0E, 0x36F60E2BA4FA6800,
   0xEB3593803173E0CE, 0x9C4CD6257C5A3603, 0xAF0C317D32ADAA8A, 0x258E5A80C7204C4B,
   0x8B889D624D44885D, 0xF4D14597E660F855, 0xD4347F66EC8941C3, 0xE699ED85B0DFB40D,
   0x2472F6207C2D0484, 0xC2A1E7B5B459AEB5, 0xAB4F6451CC1D45EC, 0x63767572AE3D6174,
   0xA59E0BD101731A28, 0x116D0016CB948F09, 0x2CF9C8CA052F6E9F, 0x0B090A7560A968E3,
   0xABEEDDB2DDE06FF1, 0x58EFC10B06A2068D, 0xC6E57A78FBD986E0, 0x2EAB8CA63CE802D7,
   0x14A195640116F336, 0x7C0828DD624EC390, 0xD74BBE77E6116AC7, 0x804456AF10F5FB53,
   0xEBE9EA2ADF4321C7, 0x03219A39EE587A30, 0x49787FEF17AF9924, 0xA1E9300CD8520548,
   0x5B45E522E4B1B4EF, 0xB49C3B3995091A36, 0xD4490AD526F14431, 0x12A8F216AF9418C2,
   0x001F837CC7350524, 0x1877B51E57A764D5, 0xA2853B80F17F58EE, 0x993E1DE72D36D310,
   0xB3598080CE64A656, 0x252F59CF0D9F04BB, 0xD23C8E176D113600, 0x1BDA0492E7E4586E,
   0x21E0BD5026C619BF, 0x3B097ADAF088F94E, 0x8D14DEDB30BE846E, 0xF95CFFA23AF5F6F4,
   0x3871700761B3F743, 0xCA672B91E9E4FA16, 0x64C8E531BFF53B55, 0x241260ED4AD1E87D,
   0x106C09B972D2E822, 0x7FBA195410E5CA30, 0x7884D9BC6CB569D8, 0x0647DFEDCD894A29,
   0x63573FF03E224774, 0x4FC8E9560F91B123, 0x1DB956E450275779, 0xB8D91274B9E9D4FB,
   0xA2EBEE47E2FBFCE1, 0xD9F1F30CCD97FB09, 0xEFED53D75FD64E6B, 0x2E6D02C36017F67F,
   0xA9AA4D20DB084E9B, 0xB64BE8D8B25396C1, 0x70CB6AF7C2D5BCF0, 0x98F076A4F7A2322E,
   0xBF84470805E69B5F, 0x94C3251F06F90CF3, 0x3E003E616A6591E9, 0xB925A6CD0421AFF3,
   0x61BDD1307C66E300, 0xBF8D5108E27E0D48, 0x240AB57A8B888B20, 0xFC87614BAF287E07,
   0xEF02CDD06FFDB432, 0xA1082C0466DF6C0A, 0x8215E577001332C8, 0xD39BB9C3A48DB6CF,
   0x2738259634305C14, 0x61CF4F94C97DF93D, 0x1B6BACA2AE4E125B, 0x758F450C88572E0B,
   0x959F587D507A8359, 0xB063E962E045F54D, 0x60E8ED72C0DFF5D1, 0x7B64978555326F9F,
   0xFD080D236DA814BA, 0x8C90FD9B083F4558, 0x106F72FE81E2C590, 0x7976033A39F7D952,
   0xA4EC0132764CA04B, 0x733EA705FAE4FA77, 0xB4D8F77BC3E56167, 0x9E21F4F903B33FD9,
   0x9D765E419FB69F6D, 0xD30C088BA61EA5EF, 0x5D94337FBFAF7F5B, 0x1A4E4822EB4D7A59,
   0x6FFE73E81B637FB3, 0xDDF957BC36D8B9CA, 0x64D0E29EEA8838B3, 0x08DD9BDFD96B9F63,
   0x087E79E5A57D1D13, 0xE328E230E3E2B3FB, 0x1C2559E30F0946BE, 0x720BF5F26F4D2EAA,
   0xB0774D261CC609DB, 0x443F64EC5A371195, 0x4112CF68649A260E, 0xD813F2FAB7F5C5CA,
   0x660D3257380841EE, 0x59AC2C7873F910A3, 0xE846963877671A17, 0x93B633ABFA3469F8,
   0xC0C0F5A60EF4CDCF, 0xCAF21ECD4377B28C, 0x57277707199B8175, 0x506C11B9D90E8B1D,
   0xD83CC2687A19255F, 0x4A29C6465A314CD1, 0xED2DF21216235097, 0xB5635C95FF7296E2,
   0x22AF003AB672E811, 0x52E762596BF68235, 0x9AEBA33AC6ECC6B0, 0x944F6DE09134DFB6,
   0x6C47BEC883A7DE39, 0x6AD047C430A12104, 0xA5B1CFDBA0AB4067, 0x7C45D833AFF07862,
   0x5092EF950A16DA0B, 0x9338E69C052B8E7B, 0x455A4B4CFE30E3F5, 0x6B02E63195AD0CF8,
   0x6B17B224BAD6BF27, 0xD1E0CCD25BB9C169, 0xDE0C89A556B9AE70, 0x50065E535A213CF6,
   0x9C1169FA2777B874, 0x78EDEFD694AF1EED, 0x6DC93D9526A50E68, 0xEE97F453F06791ED,
   0x32AB0EDB696703D3, 0x3A6853C7E70757A7, 0x31865CED6120F37D, 0x67FEF95D92607890,
   0x1F2B1D1F15F6DC9C, 0xB69E38A8965C6B65, 0xAA9119FF184CCCF4, 0xF43C732873F24C13,
   0xFB4A3D794A9A80D2, 0x3550C2321FD6109C, 0x371F77E76BB8417E, 0x6BFA9AAE5EC05779,
   0xCD04F3FF001A4778, 0xE3273522064480CA, 0x9F91508BFFCFC14A, 0x049A7F41061A9E60,
   0xFCB6BE43A9F2FE9B, 0x08DE8A1C7797DA9B, 0x8F9887E6078735A1, 0xB5B4071DBFC73A66,
   0x230E343DFBA08D33, 0x43ED7F5A0FAE657D, 0x3A88A0FBBCB05C63, 0x21874B8B4D2DBC4F,
   0x1BDEA12E35F6A8C9, 0x53C065C6C8E63528, 0xE34A1D250E7A8D6B, 0xD6B04D3B7651DD7E,
   0x5E90277E7CB39E2D, 0x2C046F22062DC67D, 0xB10BB459132D0A26, 0x3FA9DDFB67E2F199,
   0x0E09B88E1914F7AF, 0x10E8B35AF3EEAB37, 0x9EEDECA8E272B933, 0xD4C718BC4AE8AE5F,
   0x81536D601170FC20, 0x91B534F885818A06, 0xEC8177F83F900978, 0x190E714FADA5156E,
   0xB592BF39B0364963, 0x89C350C893AE7DC1, 0xAC042E70F8B383F2, 0xB49B52E587A1EE60,
   0xFB152FE3FF26DA89, 0x3E666E6F69AE2C15, 0x3B544EBE544C19F9, 0xE805A1E290CF2456,
   0x24B33C9D7ED25117, 0xE74733427B72F0C1, 0x0A804D18B7097475, 0x57E3306D881EDB4F,
   0x4AE7D6A36EB5DBCB, 0x2D8D5432157064C8, 0xD1E649DE1E7F268B, 0x8A328A1CEDFE552C,
   0x07A3AEC79624C7DA, 0x84547DDC3E203C94, 0x990A98FD5071D263, 0x1A4FF12616EEFC89,
   0xF6F7FD1431714200, 0x30C05B1BA332F41C, 0x8D2636B81555A786, 0x46C9FEB55D120902,
   0xCCEC0A73B49C9921, 0x4E9D2827355FC492, 0x19EBB029435DCB0F, 0x4659D2B743848A2C,
   0x963EF2C96B33BE31, 0x74F85198B05A2E7D, 0x5A0F544DD2B1FB18, 0x03727073C2E134B1,
   0xC7F6AA2DE59AEA61, 0x352787BAA0D7C22F, 0x9853EAB63B5E0B35, 0xABBDCDD7ED5C0860,
   0xCF05DAF5AC8D77B0, 0x49CAD48CEBF4A71E, 0x7A4C10EC2158C4A6, 0xD9E92AA246BF719E,
   0x13AE978D09FE5557, 0x730499AF921549FF, 0x4E4B705B92903BA4, 0xFF577222C14F0A3A,
   0x55B6344CF97AAFAE, 0xB862225B055B6960, 0xCAC09AFBDDD2CDB4, 0xDAF8E9829FE96B5F,
   0xB5FDFC5D3132C498, 0x310CB380DB6F7503, 0xE87FBB46217A360E, 0x2102AE466EBB1148,
   0xF8549E1A3AA5E00D, 0x07A69AFDCC42261A, 0xC4C118BFE78FEAAE, 0xF9F4892ED96BD438,
   0x1AF3DBE25D8F45DA, 0xF5B4B0B0D2DEEEB4, 0x962ACEEFA82E1C84, 0x046E3ECAAF453CE9,
   0xF05D129681949A4C, 0x964781CE734B3C84, 0x9C2ED44081CE5FBD, 0x522E23F3925E319E,
   0x177E00F9FC32F791, 0x2BC60A63A6F3B3F2, 0x222BBFAE61725606, 0x486289DDCC3D6780,
   0x7DC7785B8EFDFC80, 0x8AF38731C02BA980, 0x1FAB64EA29A2DDF7, 0xE4D9429322CD065A,
   0x9DA058C67844F20C, 0x24C0E332B70019B0, 0x233003B5A6CFE6AD, 0xD586BD01C5C217F6,
   0x5E5637885F29BC2B, 0x7EBA726D8C94094B, 0x0A56A5F0BFE39272, 0xD79476A84EE20D06,
   0x9E4C1269BAA4BF37, 0x17EFEE45B0DEE640, 0x1D95B0A5FCF90BC6, 0x93CBE0B699C2585D,
   0x65FA4F227A2B6D79, 0xD5F9E858292504D5, 0xC2B5A03F71471A6F, 0x59300222B4561E00,
   0xCE2F8642CA0712DC, 0x7CA9723FBB2E8988, 0x2785338347F2BA08, 0xC61BB3A141E50E8C,
   0x150F361DAB9DEC26, 0x9F6A419D382595F4, 0x64A53DC924FE7AC9, 0x142DE49FFF7A7C3D,
   0x0C335248857FA9E7, 0x0A9C32D5EAE45305, 0xE6C42178C4BBB92E, 0x71F1CE2490D20B07,
   0xF1BCC3D275AFE51A, 0xE728E8C83C334074, 0x96FBF83A12884624, 0x81A1549FD6573DA5,
   0x5FA7867CAF35E149, 0x56986E2EF3ED091B, 0x917F1DD5F8886C61, 0xD20D8C88C8FFE65F,
   0x31D71DCE64B2C310, 0xF165B587DF898190, 0xA57E6339DD2CF3A0, 0x1EF6E6DBB1961EC9,
   0x70CC73D90BC26E24, 0xE21A6B35DF0C3AD7, 0x003A93D8B2806962, 0x1C99DED33CB890A1,
   0xCF3145DE0ADD4289, 0xD0E4427A5514FB72, 0x77C621CC9FB3A483, 0x67A34DAC4356550B,
   0xF8D626AAAF278509,
};

U64 pawn_attacks[2][64];
U64 knight_attacks[64];
U64 king_attacks[64];
U64 bishop_masks[64];
U64 rook_masks[64];
U64 bishop_attacks[64][512];
U64 rook_attacks[64][4096];
U64 queen_attacks[64];

typedef struct {
    U64 key;
    unsigned short move;
    unsigned short weight;
    unsigned int learn;
} book_entry;

long num_book_entries = 0;

book_entry* book;

void init_book() {
    FILE *pfile = fopen("./BIN/Perfect2023.bin", "rb");

    if (pfile == NULL) {
        printf("Book file not read\n");
        return;
    }
    fseek(pfile, 0, SEEK_END);
    long position = ftell(pfile);

    if (position < sizeof(book_entry)) {
        printf("No entries found\n");
        return; 
    }

    num_book_entries = position / sizeof(book_entry);
    printf("%ld entries found in file\n", num_book_entries);

    book = (book_entry*) malloc(num_book_entries * sizeof(book_entry));
    rewind(pfile);

    size_t return_value;
    return_value = fread(book, sizeof(book_entry), num_book_entries, pfile);
    printf("fread() %ld entries read in from file\n", return_value);
}
const char file_to_char[8] = {
    'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h'
};
const char rank_to_char[8] = {
    '1', '2', '3', '4', '5', '6', '7', '8'
};

int convert_poly_move(unsigned short poly_move) {
    int ff = ((poly_move >> 6) & 7);
    int fr = ((poly_move >> 9) & 7) + 1;
    int tf = ((poly_move >> 0) & 7);
    int tr = ((poly_move >> 3) & 7) + 1;
    int pp = (poly_move >> 12) & 7;

    int source = ff + 8 * (8 - fr);
    int target = tf + 8 * (8 - tr);
    int piece;
    for (piece = P; piece <= k; piece++) {
        if (get_bit(bitboards[piece], source)) break;
    }
    int promoted;
    switch (pp)
    {
        case 0:
            promoted = 0;
            break;
        case 1:
            promoted = side == white ? N : n;
            break;
        case 2:
            promoted = side == white ? B : b;
            break;
        case 3:
            promoted = side == white ? R : r;
            break;
        case 4:
            promoted = side == white ? Q : q;
            break;

    }
    int capture = (get_bit(occupancies[both], target)) ? 1 : 0;
    int double_push = 0;
    if ((piece == P || piece == p ) && abs(source - target) == 16) {
        double_push = 1;
    }
    int enpas = 0;
    if ((piece == P || piece == p ) && abs(source - target) != 16 && abs(source - target) != 8 && !capture) {
        enpas = 1;
        capture = 1;
    }
    int castling = 0;
    if ((piece == K || piece == k) && (abs(source - target) == 2 || abs(source - target) == 3 || abs(source - target) == 4)) {
        if (target == a1) target = c1;
        if (target == a8) target = c8;
        if (target == h1) target = g1;
        if (target == h8) target = g8;
        castling = 1;
    }

    return encode_move(source, target, piece, promoted, capture, double_push, enpas, castling);
}

moves list_book_moves(U64 poly_key) {
    int index = 0;
    book_entry* entry = &book[1500];
    unsigned short move;
    moves moves;
    moves.count = 0;
    int temp_move;

    for (entry = book; entry < book + num_book_entries; entry++) {
        if (poly_key == swap_endian_U64(entry->key)) {
            move = swap_endian_u16(entry->move);

            // printf("%c%c%c%c\n", file_to_char[(move >> 6) & 7],
            //                    rank_to_char[(move >> 9) & 7],
            //                    file_to_char[(move >> 0) & 7],
            //                    rank_to_char[(move >> 3) & 7]);

            temp_move = convert_poly_move(move);
            // print_move(temp_move);
            // printf("\n\n");
            add_move(&moves, temp_move);
        }
    }
    return moves;
}

const int convert_piece_to_poly[12] = {
    1, 3, 5, 7, 9, 11, 0, 2, 4, 6, 8, 10
};
const int convert_square_to_poly[64] = {
    56, 57, 58, 59, 60, 61, 62, 63,
    48, 49, 50, 51, 52, 53, 54, 55,
    40, 41, 42, 43, 44, 45, 46, 47,
    32, 33, 34, 35, 36, 37, 38, 39,
    24, 25, 26, 27, 28, 29, 30, 31,
    16, 17, 18, 19, 20, 21, 22, 23,
    8 , 9 , 10, 11, 12, 13, 14, 15,
    0 , 1 , 2 , 3 , 4 , 5 , 6 , 7 , 
};
int has_pawn_for_capture() {
    int pawn_square = 0;
    int target_piece = side == white ? P : p;
    if (enpassant != no_sq) {
        pawn_square = side == white ? enpassant + 8 : enpassant - 8;
        // printf("ps: %llx\n", (bitboards[target_piece]));

        if (pawn_square % 8 != 7 && get_bit(bitboards[target_piece], pawn_square + 1)) 
            return 1;
        else if (pawn_square % 8 != 0 && get_bit(bitboards[target_piece], pawn_square - 1))
            return 1;
    }
    return 0;
}
U64 get_poly_keys() {
    int sq = 0, rank = 0, file = 0;
    U64 final_key = 0;
    int piece, poly_piece;
    int offset = 0;

    for (int pp = P; pp <= k; pp++) {
        U64 bitboard = bitboards[pp];
        while (bitboard != 0) {
            int square = get_lsb1_index(bitboard);
            poly_piece = convert_piece_to_poly[pp];
            int poly_square = convert_square_to_poly[square];
            final_key ^= Random64[(64*poly_piece) + poly_square];

            pop_bit(bitboard, square);
        }
    }

    offset = 768;
    if (castle & wk) final_key ^= Random64[offset + 0];
    if (castle & wq) final_key ^= Random64[offset + 1];
    if (castle & bk) final_key ^= Random64[offset + 2];
    if (castle & bq) final_key ^= Random64[offset + 3];

    offset = 772;
    if (has_pawn_for_capture()) {
        final_key ^= Random64[offset + (enpassant % 8)];
    }

    if (side == white) {
        final_key ^= Random64[780];
    }

    return final_key;
}
int get_book_move() {
    U64 poly_key = get_poly_keys();
    // printf("Polykey: %llx\n", poly_key);
    moves moves = list_book_moves(poly_key);
    if (moves.count == 0) return 0;

    srand((unsigned int)time(0));
    int rand_move = rand() % moves.count;
    return moves.moves[rand_move];
}

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

// hash pieces, en passant, castling rights
// piece, square
U64 piece_keys[12][64];
U64 enpassant_keys[64];
U64 castle_keys[16];
U64 side_key;

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
    printf("     Castling: %c%c%c%c\n", castle & wk ? 'K' : '-', castle & wq ? 'Q' : '-', castle & bk ? 'k' : '-', castle & bq ? 'q' : '-');
    printf("     Hash key: %llx\n\n", hash_key);
}

void reset_board() {
    memset(bitboards, 0ULL, sizeof(bitboards));
    memset(occupancies, 0ULL, sizeof(occupancies));
    side = 0;
    enpassant = no_sq;
    castle = 0;
    repetition_index = 0;
    fifty = 0;
    memset(repetition_table, 0ULL, sizeof(repetition_table));
}

void parse_fen(char* fen) {
    reset_board();

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
    fen++;

    fifty = atoi(fen);

    for (int piece = P; piece <= K; piece++) {
        occupancies[white] |= bitboards[piece];
    }
    for (int piece = p; piece <= k; piece++) {
        occupancies[black] |= bitboards[piece];
    }

    occupancies[both] = occupancies[white] | occupancies[black];

    hash_key = generate_hash_key();
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
    U64 hash_key_copy = hash_key;                                          \
    int fifty_copy = fifty;                                                \

#define take_back()                                                        \
    memcpy(bitboards, bitboards_copy, 96);                                 \
    memcpy(occupancies, occupancies_copy, 24);                             \
    side = side_copy, enpassant = enpassant_copy, castle = castle_copy;    \
    hash_key = hash_key_copy;                                              \
    fifty = fifty_copy;                                                    \


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

        hash_key ^= piece_keys[piece][source_square];
        hash_key ^= piece_keys[piece][target_square];

        fifty++;
        if (piece == P || piece == p) fifty = 0;

        if (capture) {
            fifty = 0;
            
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
                    
                    hash_key ^= piece_keys[pp][target_square];
                    
                    break;
                }

            }
        }

        if (promoted) {
            //pop_bit(bitboards[side == white ? P : p], target_square);

            if (side == white) {
                pop_bit(bitboards[P], target_square);
                hash_key ^= piece_keys[P][target_square];
            } else {
                pop_bit(bitboards[p], target_square);
                hash_key ^= piece_keys[p][target_square];
            }
            set_bit(bitboards[promoted], target_square);

            // add promoted piece to hash key
            hash_key ^= piece_keys[promoted][target_square];
        }

        if (enpass) {
            if (side == white) {
                pop_bit(bitboards[p], target_square + 8);
                hash_key ^= piece_keys[p][target_square + 8];
            } else {
                pop_bit(bitboards[P], target_square - 8);
                hash_key ^= piece_keys[P][target_square - 8];
            }
        }


        if (enpassant != no_sq) {
            hash_key ^= enpassant_keys[enpassant];
        }

        enpassant = no_sq;

        if (double_push) {
            //enpassant = tt;

           
            if (side == white) {
                enpassant = target_square + 8;
            } else {
                enpassant = target_square - 8;
            }
            hash_key ^= enpassant_keys[enpassant];
        }

        if (castling) {
            switch (target_square) {
                case g1: 
                    pop_bit(bitboards[R], h1);
                    set_bit(bitboards[R], f1);

                    hash_key ^= piece_keys[R][h1];
                    hash_key ^= piece_keys[R][f1];
                    break;
                case c1:
                    pop_bit(bitboards[R], a1);
                    set_bit(bitboards[R], d1);

                    hash_key ^= piece_keys[R][a1];
                    hash_key ^= piece_keys[R][d1];
                    break;
                case g8:
                    pop_bit(bitboards[r], h8);
                    set_bit(bitboards[r], f8);

                    hash_key ^= piece_keys[r][h8];
                    hash_key ^= piece_keys[r][f8];
                    break;
                case c8:
                    pop_bit(bitboards[r], a8);
                    set_bit(bitboards[r], d8);

                    hash_key ^= piece_keys[r][a8];
                    hash_key ^= piece_keys[r][d8];
                    break;
            }
        }

        // hash castling
        hash_key ^= castle_keys[castle];

        castle &= castling_rights[source_square];
        castle &= castling_rights[target_square];

        hash_key ^= castle_keys[castle];

        memset(occupancies, 0ULL, 24);
        for (int pp = P; pp <= K; pp++) {
            occupancies[white] |= bitboards[pp]; 
        }
        for (int pp = p; pp <= k; pp++) {
            occupancies[black] |= bitboards[pp]; 
        }
        occupancies[both] = occupancies[white] | occupancies[black];

        side ^= 1;

        hash_key ^= side_key;

        // U64 hash_from_scratch = generate_hash_key();
        // if (hash_key != hash_from_scratch) {
        //     printf("\n\nMake move\n");
        //     printf("move: ");
        //     print_move(move);
        //     print_board();
        //     printf(" hash key should be %llx\n", hash_from_scratch);
        //     getchar();
        // }

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

int get_time_ms() {
    #ifdef WIN64
        return GetTickCount();
    #else
        struct timeval time_value;
        gettimeofday(&time_value, NULL);
        return time_value.tv_sec * 1000 + time_value.tv_usec / 1000;
    #endif
}

int input_waiting() {
    #ifndef WIN32
        fd_set readfds;
        struct timeval tv;
        FD_ZERO (&readfds);
        FD_SET (fileno(stdin), &readfds);
        tv.tv_sec=0; tv.tv_usec=0;
        select(16, &readfds, 0, 0, &tv);

        return (FD_ISSET(fileno(stdin), &readfds));
    #else
        static int init = 0, pipe;
        static HANDLE inh;
        DWORD dw;

        if (!init)
        {
            init = 1;
            inh = GetStdHandle(STD_INPUT_HANDLE);
            pipe = !GetConsoleMode(inh, &dw);
            if (!pipe)
            {
                SetConsoleMode(inh, dw & ~(ENABLE_MOUSE_INPUT|ENABLE_WINDOW_INPUT));
                FlushConsoleInputBuffer(inh);
            }
        }
        
        if (pipe)
        {
           if (!PeekNamedPipe(inh, NULL, 0, NULL, &dw, NULL)) return 1;
           return dw;
        }
        
        else
        {
           GetNumberOfConsoleInputEvents(inh, &dw);
           return dw <= 1 ? 0 : dw;
        }

    #endif
}

void read_input() {
    // bytes to read holder
    int bytes;
    
    // GUI/user input
    char input[256] = "", *endc;

    // "listen" to STDIN
    if (input_waiting())
    {
        // tell engine to stop calculating
        stopped = 1;
        
        // loop to read bytes from STDIN
        do
        {
            // read bytes from STDIN
            bytes=read(fileno(stdin), input, 256);
        }
        
        // until bytes available
        while (bytes < 0);
        
        // searches for the first occurrence of '\n'
        endc = strchr(input,'\n');
        
        // if found new line set value at pointer to 0
        if (endc) *endc=0;
        
        // if input is available
        if (strlen(input) > 0)
        {
            // match UCI "quit" command
            if (!strncmp(input, "quit", 4))
            {
                // tell engine to terminate exacution    
                quit = 1;
            }

            // // match UCI "stop" command
            else if (!strncmp(input, "stop", 4))    {
                // tell engine to terminate exacution
                quit = 1;
            }
        }   
    }
}

// a bridge function to interact between search and GUI input
static void communicate() {
    // printf("timeset: %lld, stoptime: %lld, get_time_ms: %lld", timeset, stoptime, get_time_ms());
	// if time is up break here
    if(timeset == 1 && get_time_ms() > stoptime) {
		// tell engine to stop calculating
		stopped = 1;
	}
	
    // read GUI input
	read_input();
}

U64 nodes = 0;
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

        // U64 hash_from_scratch = generate_hash_key();
        // if (hash_key != hash_from_scratch) {
        //     printf("\n\nTake back\n");
        //     printf("move: ");
        //     print_move(moves.moves[i]);
        //     printf(" hash key should be %llx\n", hash_from_scratch);
        //     getchar();
        // }
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
    printf(" Nodes: %lld\n", nodes);
    printf(" Time: %d\n\n", get_time_ms()-start);
}

// extactly matches PNBRQKpnbrqk
// in order to index by the piece enum
const int material_score[2][12] = {
    // opening
    82, 337, 365, 477, 1025, 12000, -82, -337, -365, -477, -1025, -12000,
    // endgame
    94, 281, 297, 512,  936, 12000, -94, -281, -297, -512,  -936, -12000
};

const int opening_phase_score = 6192;
const int endgame_phase_score = 518;

enum {opening, endgame, middlegame};
enum {PAWN, KNIGHT, BISHOP, ROOK, QUEEN, KING};

const int positional_score[2][6][64] = {
    //pawn
    0,   0,   0,   0,   0,   0,  0,   0,
    98, 134,  61,  95,  68, 126, 34, -11,
    -6,   7,  26,  31,  65,  56, 25, -20,
    -14,  13,   6,  21,  23,  12, 17, -23,
    -27,  -2,  -5,  12,  17,   6, 10, -25,
    -26,  -4,  -4, -10,   3,   3, 33, -12,
    -35,  -1, -20, -23, -15,  24, 38, -22,
    0,   0,   0,   0,   0,   0,  0,   0,
    
    // knight
    -167, -89, -34, -49,  61, -97, -15, -107,
    -73, -41,  72,  36,  23,  62,   7,  -17,
    -47,  60,  37,  65,  84, 129,  73,   44,
    -9,  17,  19,  53,  37,  69,  18,   22,
    -13,   4,  16,  13,  28,  19,  21,   -8,
    -23,  -9,  12,  10,  19,  17,  25,  -16,
    -29, -53, -12,  -3,  -1,  18, -14,  -19,
    -105, -21, -58, -33, -17, -28, -19,  -23,
    
    // bishop
    -29,   4, -82, -37, -25, -42,   7,  -8,
    -26,  16, -18, -13,  30,  59,  18, -47,
    -16,  37,  43,  40,  35,  50,  37,  -2,
    -4,   5,  19,  50,  37,  37,   7,  -2,
    -6,  13,  13,  26,  34,  12,  10,   4,
    0,  15,  15,  15,  14,  27,  18,  10,
    4,  15,  16,   0,   7,  21,  33,   1,
    -33,  -3, -14, -21, -13, -12, -39, -21,
    
    // rook
    32,  42,  32,  51, 63,  9,  31,  43,
    27,  32,  58,  62, 80, 67,  26,  44,
    -5,  19,  26,  36, 17, 45,  61,  16,
    -24, -11,   7,  26, 24, 35,  -8, -20,
    -36, -26, -12,  -1,  9, -7,   6, -23,
    -45, -25, -16, -17,  3,  0,  -5, -33,
    -44, -16, -20,  -9, -1, 11,  -6, -71,
    -19, -13,   1,  17, 16,  7, -37, -26,
    
    // queen
    -28,   0,  29,  12,  59,  44,  43,  45,
    -24, -39,  -5,   1, -16,  57,  28,  54,
    -13, -17,   7,   8,  29,  56,  47,  57,
    -27, -27, -16, -16,  -1,  17,  -2,   1,
    -9, -26,  -9, -10,  -2,  -4,   3,  -3,
    -14,   2, -11,  -2,  -5,   2,  14,   5,
    -35,  -8,  11,   2,   8,  15,  -3,   1,
    -1, -18,  -9,  10, -15, -25, -31, -50,
    
    // king
    -65,  23,  16, -15, -56, -34,   2,  13,
    29,  -1, -20,  -7,  -8,  -4, -38, -29,
    -9,  24,   2, -16, -20,   6,  22, -22,
    -17, -20, -12, -27, -30, -25, -14, -36,
    -49,  -1, -27, -39, -46, -44, -33, -51,
    -14, -14, -22, -46, -44, -30, -15, -27,
    1,   7,  -8, -64, -43, -16,   9,   8,
    -15,  36,  12, -54,   8, -28,  24,  14,


    // Endgame positional piece scores //

    //pawn
    0,   0,   0,   0,   0,   0,   0,   0,
    178, 173, 158, 134, 147, 132, 165, 187,
    94, 100,  85,  67,  56,  53,  82,  84,
    32,  24,  13,   5,  -2,   4,  17,  17,
    13,   9,  -3,  -7,  -7,  -8,   3,  -1,
    4,   7,  -6,   1,   0,  -5,  -1,  -8,
    13,   8,   8,  10,  13,   0,   2,  -7,
    0,   0,   0,   0,   0,   0,   0,   0,
    
    // knight
    -58, -38, -13, -28, -31, -27, -63, -99,
    -25,  -8, -25,  -2,  -9, -25, -24, -52,
    -24, -20,  10,   9,  -1,  -9, -19, -41,
    -17,   3,  22,  22,  22,  11,   8, -18,
    -18,  -6,  16,  25,  16,  17,   4, -18,
    -23,  -3,  -1,  15,  10,  -3, -20, -22,
    -42, -20, -10,  -5,  -2, -20, -23, -44,
    -29, -51, -23, -15, -22, -18, -50, -64,
    
    // bishop
    -14, -21, -11,  -8, -7,  -9, -17, -24,
    -8,  -4,   7, -12, -3, -13,  -4, -14,
    2,  -8,   0,  -1, -2,   6,   0,   4,
    -3,   9,  12,   9, 14,  10,   3,   2,
    -6,   3,  13,  19,  7,  10,  -3,  -9,
    -12,  -3,   8,  10, 13,   3,  -7, -15,
    -14, -18,  -7,  -1,  4,  -9, -15, -27,
    -23,  -9, -23,  -5, -9, -16,  -5, -17,
    
    // rook
    13, 10, 18, 15, 12,  12,   8,   5,
    11, 13, 13, 11, -3,   3,   8,   3,
    7,  7,  7,  5,  4,  -3,  -5,  -3,
    4,  3, 13,  1,  2,   1,  -1,   2,
    3,  5,  8,  4, -5,  -6,  -8, -11,
    -4,  0, -5, -1, -7, -12,  -8, -16,
    -6, -6,  0,  2, -9,  -9, -11,  -3,
    -9,  2,  3, -1, -5, -13,   4, -20,
    
    // queen
    -9,  22,  22,  27,  27,  19,  10,  20,
    -17,  20,  32,  41,  58,  25,  30,   0,
    -20,   6,   9,  49,  47,  35,  19,   9,
    3,  22,  24,  45,  57,  40,  57,  36,
    -18,  28,  19,  47,  31,  34,  39,  23,
    -16, -27,  15,   6,   9,  17,  10,   5,
    -22, -23, -30, -16, -16, -23, -36, -32,
    -33, -28, -22, -43,  -5, -32, -20, -41,
    
    // king
    -74, -35, -18, -18, -11,  15,   4, -17,
    -12,  17,  14,  17,  17,  38,  23,  11,
    10,  17,  23,  15,  20,  45,  44,  13,
    -8,  22,  24,  27,  26,  33,  26,   3,
    -18,  -4,  21,  24,  27,  23,   9, -11,
    -19,  -3,  11,  21,  23,  16,   7,  -9,
    -27, -11,   4,  13,  14,   4,  -5, -17,
    -53, -34, -21, -11, -28, -14, -24, -43
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

static const int bishop_unit = 4;
static const int queen_unit = 9;

static const int bishop_mobility_opening = 5;
static const int bishop_mobility_endgame = 5;
static const int queen_mobility_opening = 1;
static const int queen_mobility_endgame = 2;

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

static inline int get_game_phase_score() {
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

static inline int evaluate() {
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
                    if (bitboards[P] & file_masks[square] == 0) {
                        score_opening -= semi_open_file_score;
                        score_endgame -= semi_open_file_score;
                    }

                    // if no pawns at all
                    if ((bitboards[P] | bitboards[p]) & file_masks[square] == 0) {
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
                    if (bitboards[p] & file_masks[square] == 0) {
                        score_opening += semi_open_file_score;
                        score_endgame += semi_open_file_score;
                    }

                    // if no pawns at all
                    if ((bitboards[P] | bitboards[p]) & file_masks[square] == 0) {
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


#define infinity 50000
#define mate_value 49000
#define mate_score 48000

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


// hash table size
//#define hash_size 800000
int hash_entries = 0;

#define no_hash_entry 100000

// transposition table hash flags
#define hash_flag_exact 0
#define hash_flag_alpha 1
#define hash_flag_beta 2

// transposition table data structure
typedef struct {
    U64 hash_key;   // "almost" unique chess position identifier
    int best_move;  // best move from this position
    int depth;      // current search depth
    int flag;       // flag the type of node (fail-low/fail-high/PV) 
    int score;      // score (alpha/beta/PV)
} tt;               // transposition table (TT aka hash table)

// define TT instance
tt *hash_table = NULL; 

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

static inline int read_hash_entry(int alpha, int beta, int* best_move, int depth) {
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

static inline int write_hash_entry(int score, int best_move, int depth, int flag) {
    tt *hash_entry = &hash_table[hash_key % hash_entries];

    if (score < -mate_score) score -= ply;
    if (score > mate_score) score += ply;

    hash_entry->best_move = best_move;
    hash_entry->hash_key = hash_key;
    hash_entry->score = score;
    hash_entry->flag = flag;
    hash_entry->depth = depth;
}


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

        // extract move features
	    // int source_square = get_move_source(move);
	    // int target_square = get_move_target(move);	

	    // // make the first capture, so that X-ray defender show up
	    // pop_bit(bitboards[get_move_piece(move)], source_square);

	    // // captures of undefended pieces are good by definition
	    // if (!is_square_attacked(target_square, side ^ 1)) {
		//     // restore captured piece
		//     set_bit(bitboards[get_move_piece(move)], source_square);
		    
		//     // score undefended captures greater than other captures
		//     return 15000;
	    // }
	    
	    // // restore captured piece
	    // set_bit(bitboards[get_move_piece(move)], source_square);

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

static inline int sort_moves(moves* moves, int best_move) {
    int move_scores[moves->count];

    // fill array based on scores
    for (int i = 0; i < moves->count; i++) {
        if (best_move == moves->moves[i]) {
            move_scores[i] = 30000;
        } else {
            move_scores[i] = score_move(moves->moves[i]);
        }
    }

    // bubble sort
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


static inline int is_repetition() {
    for (int i = 0; i < repetition_index; i++) {
        if (repetition_table[i] == hash_key) {
            return 1;
        }
    }
    
    return 0;
}    

static inline int quiescence(int alpha, int beta) {
    if ((nodes & 2047) == 0)
        communicate();

    nodes++;   

    if (ply > max_ply - 1) {
        return evaluate();
    }

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

    sort_moves(&moves, 0);

    for (int count = 0; count < moves.count; count++) {
        copy_board();

        ply++;

        repetition_index++;
        repetition_table[repetition_index] = hash_key;


        if (make_move(moves.moves[count], only_captures) == 0) {
            // if make move is illegal
            ply--;

            repetition_index--;
            
            continue;
        }

        int score = -quiescence(-beta, -alpha);

        ply--;

        repetition_index--;

        take_back();

        // reutrn 0 if time is up
        if(stopped == 1) return 0;

        // found a better move
        if (score > alpha) {
            alpha = score;
        
            // fail-hard beta cutoff
            if (score >= beta) {
                // move fails high
                return beta;
            } 
        }
    }
    return alpha;
}

const int full_depth_moves = 4;
const int reduction_limit = 3;

static inline int negamax(int alpha, int beta, int depth) {
    // init pv length
    pv_length[ply] = ply;

    int score;

    int best_move = 0;

    int hash_flag = hash_flag_alpha;

    if ((ply && is_repetition()) || fifty >= 100) {
        return 0;
    }
    int pv_node = ((beta-alpha) > 1);
    if (ply && (score = read_hash_entry(alpha, beta, &best_move, depth)) != no_hash_entry && !pv_node) {
        return score;
    }


    if ((nodes & 2047) == 0)
        communicate();


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

    // get static evaluation score
	int static_eval = evaluate();
    
    // evaluation pruning / static null move pruning
	if (depth < 3 && !pv_node && !in_check &&  abs(beta - 1) > -infinity + 100)
	{   
        // define evaluation margin
		int eval_margin = 120 * depth;
		
		// evaluation margin substracted from static evaluation score fails high
		if (static_eval - eval_margin >= beta)
		    // evaluation margin substracted from static evaluation score
			return static_eval - eval_margin;
    }
 
    // null move pruning
    if (depth >= 3 && !in_check && ply) {
        copy_board();

        ply++;

        repetition_index++;
        repetition_table[repetition_index] = hash_key;

        
        if (enpassant != no_sq) {
            hash_key ^= enpassant_keys[enpassant];
        }
        enpassant = no_sq;
        
        side ^= 1;
        hash_key ^= side_key;

        score = -negamax(-beta, -beta + 1, depth - 3);

        ply--;

        repetition_index--;

        take_back();

        if(stopped == 1) {
            return 0;
        }

        if (score >= beta) 
            return beta;
    }

    //razoring
    if (!pv_node && !in_check && depth <= 3) {
        score = static_eval + 125;
        int new_score;

        if (score < beta) {
            if (depth == 1) {
                new_score = quiescence(alpha, beta);
                return (new_score > score) ? new_score : score;
            }
            score += 175;
            if (score < beta && depth <= 2) {
                new_score = quiescence(alpha, beta);
                if (new_score < beta) {
                    return (new_score > score) ? new_score : score;
                }
            }
        }
    }

    moves moves;

    generate_moves(&moves);

    if (follow_pv) {
        enable_pv_scoring(&moves);
    }

    sort_moves(&moves, best_move);

    int moves_searched = 0;

    for (int count = 0; count < moves.count; count++) {
        copy_board();

        ply++;

        repetition_index++;
        repetition_table[repetition_index] = hash_key;


        if (make_move(moves.moves[count], all_moves) == 0) {
            // if make move is illegal
            ply--;

            repetition_index--;

            continue;
        }

        legal_moves++;

        // PVS & LMR
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
        
        ply--;

        repetition_index--;

        take_back();

        if(stopped == 1) {
            return 0;
        }

        moves_searched++;

        // found a better move
        if (score > alpha) {

            hash_flag = hash_flag_exact;

            best_move = moves.moves[count];

            if (!get_move_capture(moves.moves[count]))  {
                history_moves[get_move_piece(moves.moves[count])][get_move_target(moves.moves[count])] += depth;
            }
            alpha = score;

            pv_table[ply][ply] = moves.moves[count];

            for (int next_ply = ply+1; next_ply < pv_length[ply+1]; next_ply++) {
                // copy move from deepetr ply into current plys line
                pv_table[ply][next_ply] = pv_table[ply+1][next_ply];
            }

            // adjust pv length
            pv_length[ply] = pv_length[ply+1];


            // fail-hard beta cutoff
            if (score >= beta) {
                write_hash_entry(beta, best_move, depth, hash_flag_beta);

                if (!get_move_capture(moves.moves[count]))  {
                    // remember two moves
                    killer_moves[1][ply] = killer_moves[0][ply];
                    killer_moves[0][ply] = moves.moves[count];
                }
                // move fails high
                return beta;
            }
        }
    }

    if (legal_moves == 0) {

        if (in_check) {
            // less than -50000 for it to be in alpha beta bounds
            // add ply for it to be able to checkmate in bigger depths
            return -mate_value + ply;
        } else {
            return 0;
        }
    }
    write_hash_entry(alpha, best_move, depth, hash_flag);

    // when move fails low
    return alpha;
}

// search for best move
void search_position(int depth) {
    if (in_opening) {
        int move = get_book_move();
        if (move) {
            printf("bestmove ");
            print_move(move);
            printf("\n");
            return;
        }
        in_opening = 0;
    }
    
    int score = 0;
    nodes = 0;
    stopped = 0;
    follow_pv = 0;
    score_pv = 0;
    
    memset(killer_moves, 0, sizeof(killer_moves));
    memset(history_moves, 0, sizeof(history_moves));
    memset(pv_length, 0, sizeof(pv_length));
    memset(pv_table, 0, sizeof(pv_table));

    int alpha = -infinity;
    int beta = infinity;

    for (int current_depth = 1; current_depth <= depth; current_depth++) {
        if (stopped)
            break;

        follow_pv = 1;

        score = negamax(alpha, beta, current_depth);

        if (score <= alpha || score >= beta) {
            alpha = -infinity;
            beta = infinity;
            continue;
        }
        alpha = score - 50;
        beta = score + 50;
        
        if (pv_length[0]) {
            if (score > -mate_value && score < -mate_score)
                printf("info score mate %d depth %d nodes %lld time %d pv ", -(score + mate_value) / 2 - 1, current_depth, nodes, get_time_ms() - starttime);
            
            else if (score > mate_score && score < mate_value)
                printf("info score mate %d depth %d nodes %lld time %d pv ", (mate_value - score) / 2 + 1, current_depth, nodes, get_time_ms() - starttime);   
            
            else
                printf("info score cp %d depth %d nodes %lld time %d pv ", score, current_depth, nodes, get_time_ms() - starttime);


            for (int count=0; count < pv_length[0]; count++) {
                print_move(pv_table[0][count]);
                printf("  ");
            }
            printf("\n");
        }
    }

    // best move placeholders
    printf("bestmove ");
    print_move(pv_table[0][0]);
    printf("\n");
}

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
    // shift pointer to the right where next token begins
    command += 9;
    
    // init pointer to the current character in the command string
    char *current_char = command;
    
    // parse UCI "startpos" command
    if (strncmp(command, "startpos", 8) == 0)
        // init chess board with start position
        parse_fen(start_position);
    
    // parse UCI "fen" command 
    else
    {
        // make sure "fen" command is available within command string
        current_char = strstr(command, "fen");
        
        // if no "fen" command is available within command string
        if (current_char == NULL)
            // init chess board with start position
            parse_fen(start_position);
            
        // found "fen" substring
        else
        {
            // shift pointer to the right where next token begins
            current_char += 4;
            
            // init chess board with position from FEN string
            parse_fen(current_char);
        }
    }
    
    // parse moves after position
    current_char = strstr(command, "moves");
    
    // moves available
    if (current_char != NULL)
    {
        half_moves = 0;
        // shift pointer to the right where next token begins
        current_char += 6;
        
        // loop over moves within a move string
        while(*current_char)
        {
            // parse next move
            int move = parse_move(current_char);
            
            // if no more moves
            if (move == 0)
                // break out of the loop
                break;
            
            // increment repetition index
            repetition_index++;
            
            // wtire hash key into a repetition table
            repetition_table[repetition_index] = hash_key;
            
            // make move on the chess board
            make_move(move, all_moves);
            half_moves++;
            // move current character mointer to the end of current move
            while (*current_char && *current_char != ' ') current_char++;
            
            // go to the next move
            current_char++;
        }        
    }
    
    // print board
    // print_board();
}

float get_movestogo(int ply_count) {
    return 59.3 + (72830 - 2330 * ply_count) / (2644 + ply_count * (10 + ply_count));
}

void parse_go(char *command) {
    reset_time_control();

    // printf("%s\n", command);
    // init parameters
    int depth = -1;

    // init argument
    char *argument = NULL;

    // infinite search
    if ((argument = strstr(command,"infinite"))) {}

    // match UCI "binc" command
    if ((argument = strstr(command,"binc")) && side == black)
        // parse black time increment
        inc = atoi(argument + 5);

    // match UCI "winc" command
    if ((argument = strstr(command,"winc")) && side == white)
        // parse white time increment
        inc = atoi(argument + 5);

    // match UCI "wtime" command
    if ((argument = strstr(command,"wtime")) && side == white)
        // parse white time limit
        _time = atoi(argument + 6);

    // match UCI "btime" command
    if ((argument = strstr(command,"btime")) && side == black)
        // parse black time limit
        _time = atoi(argument + 6);

    // match UCI "movestogo" command
    if ((argument = strstr(command,"movestogo")))
        // parse number of moves to go
        movestogo = atoi(argument + 10);

    // match UCI "movetime" command
    if ((argument = strstr(command,"movetime")))
        // parse amount of time allowed to spend to make a move
        movetime = atoi(argument + 9);

    // match UCI "depth" command
    if ((argument = strstr(command,"depth")))
        // parse search depth
        depth = atoi(argument + 6);

    // if move time is not available
    if(_time == -1 && movetime != -1)
    {
        // set time equal to move time
        _time = movetime;

        // set moves to go to 1
        movestogo = 1;
    }

    // init start time
    starttime = get_time_ms();

    // init search depth
    depth = depth;
    // printf("ply count: %d\n", half_moves);
    // printf("moves to go: %d\n", movestogo);
    // if time control is available
    if(_time != -1)
    {
        // flag we're playing with time control
        timeset = 1;
        // convert half moves into full moves
        if (movetime != -1) {
            movestogo = 4;
            movetime = -1;
        }
        else
            movestogo = get_movestogo(half_moves)/2;
        printf("ply count: %d\n", half_moves);
        printf("moves to go: %d\n", movestogo);
        printf("time: %d\n", _time);
        // set up timing
        _time /= movestogo;
        printf("time: %d\n", _time);
        if (_time > 1500) _time -= 50;
        stoptime = starttime + _time + inc;
        if (_time < 1500 && inc && depth == 64) stoptime = starttime + inc - 50;
    }

    // if depth is not available
    if(depth == -1)
        // set depth to 64 plies (takes ages to complete...)
        depth = 64;

    // print debug info
    printf("time:%d start:%u stop:%u depth:%d timeset:%d\n",
    _time, starttime, stoptime, depth, timeset);

    // search position
    search_position(depth);
}

void uci_loop() {
    int max_hash = 128;
    int mb = 64;

    // reset stdin, stdout
    setbuf(stdin, NULL);
    setbuf(stdout, NULL);

    // define input buffer
    char input[2000];

    printf("id name hhc\n");
    printf("id name Code hemihemichess\n");
    printf("option name Hash type spin default 64 min 4 max %d\n", max_hash);
    printf("uciok\n");

    while(1) {
        //reset user input
        memset(input, 0, sizeof(input));

        fflush(stdout);

        // get user input from stdin
        if (!fgets(input, 2000, stdin))
            continue;

        // printf("%s\n", input);

        if (input[0] == '\n')
            continue;

        if (strncmp(input, "isready", 7) == 0) {
            printf("readyok\n");
        }

        else if (strncmp(input, "position", 8) == 0) {
            parse_position(input);
            clear_hash_table(); 
        }

        else if (strncmp(input, "ucinewgame", 10) == 0) {
            parse_position("position startpos");
            clear_hash_table();
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

        else if (!strncmp(input, "setoption name Hash value", 26)) {
            sscanf(input, "%*s %*s %*s %*s %d", &mb);

            if (mb < 4) mb = 4;
            if (mb > max_hash) mb = max_hash;

            printf("Set hash table size to %dMB", mb);
            init_hash_table(mb);
        }

        else if (!strncmp(input, "d", 1)) {
            print_board();
        }
    }
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

void init_all() {
    init_leapers_attacks();
    init_sliders_attacks(bishop);
    init_sliders_attacks(rook);
    init_random_keys();
    initialize_evaluation_masks();
    init_hash_table(64);
    init_book();
}


int main() {
    // init all
    init_all(); 
    
    int debug = 0;

    // printf("move: %d", encode_move(d5, e5, k,0,0,0,0,0));
    if (debug) {
        //parse_fen("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR b KQkq e3 0 1 ");
        parse_fen(start_position);
        search_position(10);
    } else {
        uci_loop();
    }

    free(hash_table);
    free(book);

    return 0;
}