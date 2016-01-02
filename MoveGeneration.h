#ifndef domibuMoveGeneration
#define domibuMoveGeneration

#include "data.h"

Nmovelist *NML;

extern int stop;
extern U64 zobrist[782];

void print_state(board arg);

void printmoveN(Nmove *m);
void printmovedetailsN(Nmove *ff);
void print_move_xboard_1(Nmove *m);

U64 gen_ho_atackN(board arg);
char generate_movesN(Nmovelist *ZZZ, board arg);
char generate_movesN_test(Nmovelist *ZZZ, board arg);

U64 generate_captures(Nmovelist *ZZZ, board arg);
U64 generate_captures_2(Nmovelist *ZZZ, board arg);

int evaluate( board arg, int draft, int color, board *rb);

int Ndo_move(board *b, Nmove m);
int Nundo_move(board *b, Nmovelist *ml, Nmove m);

#endif

