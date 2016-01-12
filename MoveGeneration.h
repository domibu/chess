#ifndef domibuMoveGeneration
#define domibuMoveGeneration

#include "data.h"

node_move_list *NML;

extern int stop;
extern U64 zobrist[782];

char *print_state(board arg);

char *print_smith_notation(move *m);
char *print_move_details(move *ff);
char *print_SAN_notation(move *m);

U64 generate_hostile_atackmap(board arg);
char generate_moves(node_move_list *ZZZ, board arg);
char gm1(node_move_list *ZZZ, board arg);

U64 generate_captures(node_move_list *ZZZ, board arg);
U64 gc2(node_move_list *ZZZ, board arg);

int evaluate( board arg, int draft, int color, board *rb);

int do_move(board *b, move m);
int undo_move(board *b, node_move_list *ml, move m);

#endif

