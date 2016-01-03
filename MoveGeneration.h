#ifndef domibuMoveGeneration
#define domibuMoveGeneration

#include "data.h"

node_move_list *NML;

extern int stop;
extern U64 zobrist[782];

void print_state(board arg);

void printmoveN(move *m);
void printmovedetailsN(move *ff);
void print_move_xboard_1(move *m);

U64 gen_ho_atackN(board arg);
char generate_movesN(node_move_list *ZZZ, board arg);
char generate_movesN_test(node_move_list *ZZZ, board arg);

U64 generate_captures(node_move_list *ZZZ, board arg);
U64 generate_captures_2(node_move_list *ZZZ, board arg);

int evaluate( board arg, int draft, int color, board *rb);

int Ndo_move(board *b, move m);
int Nundo_move(board *b, node_move_list *ml, move m);

#endif

