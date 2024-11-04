// #include "bbc.h"
#include "search.h"
// #include "board.h"
// #include "time_controls.h"
// #include "evaluation.h"
// #include "t_table.h"
// #include "openings.h"

#define infinity 50000
#define mate_value 49000

inline void sort_moves(moves* moves, int best_move) {
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

inline int is_repetition() {
    for (int i = 0; i < repetition_index; i++) {
        if (repetition_table[i] == hash_key) {
            return 1;
        }
    }
    
    return 0;
}    

inline int quiescence(int alpha, int beta) {
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

int pv_length[max_ply];

int negamax(int alpha, int beta, int depth) {
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

    // get evaluation score
	int static_eval = evaluate();
    
    // evaluation pruning / null move pruning
	if (depth < 3 && !pv_node && !in_check &&  abs(beta - 1) > -infinity + 100)
	{   
        // define evaluation margin
		int eval_margin = 120 * depth;
		
		// evaluation margin substracted from evaluation score fails high
		if (static_eval - eval_margin >= beta)
		    // evaluation margin substracted from evaluation score
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