#define MAX_DEPTH 10
#define SAMPLING_COUNT 50

long long int depth_nodes = 0;
long long int branches_not_taken [MAX_DEPTH];
long long int sum_branching_factors [MAX_DEPTH];
long long int perft_startpos_results [MAX_DEPTH] = {20, 400, 8902, 197281,  4865609, 119060324,  3195901860, 84998978956, 2439530234167, 69352859712417};

static inline int negamax_record(int alpha, int beta, int depth, int initial_depth, int count_branches) {

    if (depth == 0) {
        depth_nodes++;
        return evaluate();
    }

    nodes++;

    // check if opponent is in check
    int in_check = is_square_attacked((side==white) ? get_lsb1_index(bitboards[K]) : get_lsb1_index(bitboards[k]), side ^ 1);
    int legal_moves = 0;
    int best_sofar;

    // old alpha value
    int old_alpha = alpha;

    moves moves;

    generate_moves(&moves);

    if (count_branches) {
        sum_branching_factors[initial_depth-depth] += moves.count;
    }

    for (int count = 0; count < moves.count; count++) {
        copy_board();

        ply++;

        if (make_move(moves.moves[count], all_moves) == 0) {
            ply--;
            continue;
        }

        legal_moves++;

        int score = -negamax_record(-beta, -alpha, depth-1, initial_depth, count_branches);

        ply--;
        take_back();

        if (score >= beta) {
            if (count_branches) {
                branches_not_taken[initial_depth-depth]++;
            }
            return beta;
        } 

        if (score > alpha) {
            alpha = score;

            if (ply == 0) {
                best_sofar = moves.moves[count];
            }
        }

        if (old_alpha != alpha) {
            best_move = best_sofar;
        }
    }

    if (legal_moves == 0) {

        if (in_check) {
            return -49000 + ply;
        } else {
            return 0;
        }
    }

    return alpha;
}

void record (int depth) {

    FILE* fp = fopen("./records/data.txt", "w");
    if (fp == NULL) {
        printf("error opening data.txt");
        return;
    }

    for (int i=1; i <= min(MAX_DEPTH, depth); i++){
        depth_nodes = 0;
        negamax_record(-50000, 50000, i, i, i == min(MAX_DEPTH, depth));
        printf("node count at depth %d: %lld\n", i, depth_nodes);
    }

    printf("\n");
    // the printing values of depth_nodes, branch_factors is different because branch_factors is cummulative
    for (int i=0; i < min(MAX_DEPTH, depth); i++) {
        printf("branches not taken at depth %d: %lld\n", i+1, branches_not_taken[i]);
        printf("nodes with cutting: %lld, nodes without cutting: %lld, percentage saved: %lld%%\n\n", sum_branching_factors[i], perft_startpos_results[i], 100*(perft_startpos_results[i]-sum_branching_factors[i])/sum_branching_factors[i]);
        fprintf(fp, "%d %lld %lld\n", i+1, sum_branching_factors[i], perft_startpos_results[i]);
    }

    fclose(fp);
}