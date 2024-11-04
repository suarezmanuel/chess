#include "bbc.h"
#include "board.h"
#include "evaluation.h"
#include "magic.h"
#include "openings.h"
#include "t_table.h"
#include "time_controls.h"
#include "search.h"

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

void parse_fen(char* fen) {
    reset_board();

    for (int rank = 0; rank < 8; rank++) {
        for (int file = 0; file < 8; file++) {
            int square = rank * 8 + file;

            if ((*fen > 'a' && *fen < 'z') || (*fen > 'A' && *fen < 'Z')) {
                int piece = char_pieces[(int)(*fen)];
                set_bit(bitboards[piece], square);
                fen++;
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
                fen++;
            }
            
            if (*fen == '/') {
                fen++;
            }
        }
    }

    fen++;
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
        fen++;
    }

    fen++;
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