// #include "bbc.h"
// #include "perft.h"
// #include "board.h"
// #include "time_controls.h"

// long local_nodes = 0;

// inline void perft_driver(int depth) {

//     if (depth == 0) {
//         nodes++;
//         local_nodes++;
//         return;
//     }

//     moves moves;

//     generate_moves(&moves);

//     for (int i=0; i < moves.count; i++) {

//         copy_board();

//         if (!make_move(moves.moves[i], all_moves))
//             continue;

//         perft_driver(depth-1);

//         take_back();

//         // U64 hash_from_scratch = generate_hash_key();
//         // if (hash_key != hash_from_scratch) {
//         //     printf("\n\nTake back\n");
//         //     printf("move: ");
//         //     print_move(moves.moves[i]);
//         //     printf(" hash key should be %llx\n", hash_from_scratch);
//         //     getchar();
//         // }
//     }
// }

// void perft_test(int depth) {

//     printf("\n Performance test\n\n");
//     moves moves;

//     generate_moves(&moves);

//     int start = get_time_ms();

//     for (int i=0; i < moves.count; i++) {
        
//         copy_board();

//         if (!make_move(moves.moves[i], all_moves))
//             continue;

//         long sum_nodes = nodes;

//         perft_driver(depth-1);
        
//         long old_nodes = nodes - sum_nodes;

//         take_back();

//         // print_move(moves.moves[i]);
//         printf("%s%s%c: %ld\n", square_to_coordinates[get_move_source(moves.moves[i])]
//                          , square_to_coordinates[get_move_target(moves.moves[i])]
//                          , promoted_pieces[get_move_promoted(moves.moves[i])]
//                          , old_nodes);
//     }

//     printf("\n Depth: %d\n", depth);
//     printf(" Nodes: %lld\n", nodes);
//     printf(" Time: %d\n\n", get_time_ms()-start);
// }