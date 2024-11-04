#pragma once

#ifndef EVALUATION_H
#define EVALUATION_H

#include "board.h"
#include "bbc.h"

void initialize_evaluation_masks();

int mvv_lva[12][12] = {
 	{ 105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605 },
	{ 104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604 },
	{ 103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603 },
	{ 102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602 },
	{ 101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601 },
	{ 100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600 },

	{ 105, 205, 305, 405, 505, 605,  105, 205, 305, 405, 505, 605 },
	{ 104, 204, 304, 404, 504, 604,  104, 204, 304, 404, 504, 604 },
	{ 103, 203, 303, 403, 503, 603,  103, 203, 303, 403, 503, 603 },
	{ 102, 202, 302, 402, 502, 602,  102, 202, 302, 402, 502, 602 },
	{ 101, 201, 301, 401, 501, 601,  101, 201, 301, 401, 501, 601 },
	{ 100, 200, 300, 400, 500, 600,  100, 200, 300, 400, 500, 600 }
};

/*
    How we sort different moves:
    1. PV move
    2. captures using MVV/LVA
    3. 1st killer move
    4. 2nd killer move
    5. History moves
    6. Rest of the moves
*/
inline int score_move(int move) {
    
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

int evaluate();

inline void enable_pv_scoring(moves* moves) {
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

#endif // EVALUATION_H
