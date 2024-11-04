#include "board.h"

const U64 not_a_file  = 18374403900871474942ULL;
const U64 not_h_file  = 9187201950435737471ULL;
const U64 not_ab_file = 18229723555195321596ULL;
const U64 not_gh_file = 4557430888798830399ULL;

const char* square_to_coordinates[64] = {
    "a8", "b8", "c8", "d8", "e8", "f8", "g8", "h8",
    "a7", "b7", "c7", "d7", "e7", "f7", "g7", "h7",
    "a6", "b6", "c6", "d6", "e6", "f6", "g6", "h6",
    "a5", "b5", "c5", "d5", "e5", "f5", "g5", "h5",
    "a4", "b4", "c4", "d4", "e4", "f4", "g4", "h4",
    "a3", "b3", "c3", "d3", "e3", "f3", "g3", "h3",
    "a2", "b2", "c2", "d2", "e2", "f2", "g2", "h2",
    "a1", "b1", "c1", "d1", "e1", "f1", "g1", "h1",
};

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
int make_move(int move, int move_flag) {

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
                pop_bit(bitboards[p], (target_square + 8));
                hash_key ^= piece_keys[p][target_square + 8];
            } else {
                pop_bit(bitboards[P], (target_square - 8));
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
    return 0;
} 
     
void generate_moves(moves* move_list) {
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
                            if (source_square >= a2 && source_square <= h2 && !get_bit(occupancies[both], (target_square - 8))) {
                                add_move(move_list, encode_move(source_square, (target_square-8), piece, 0, 0, 1, 0, 0));
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
                            if (source_square >= a7 && source_square <= h7 && !get_bit(occupancies[both], (target_square + 8))) {
                                add_move(move_list, encode_move(source_square, (target_square+8), piece, 0, 0, 1, 0, 0)); 
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
