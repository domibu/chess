#ifndef domibuMoveGeneration
#define domibuMoveGeneration

#include "data.h"

Nmovelist *NML;

extern int stop;
extern U64 zobrist[782];

void print_state(Nboard arg);

void printmoveN(Nmove *m);
void printmovedetailsN(Nmove *ff);
void print_move_xboard(Nmove *m);

U64 gen_ho_atackN(Nboard arg);
char generate_movesN(Nmovelist *ZZZ, Nboard arg);
char generate_movesN_test(Nmovelist *ZZZ, Nboard arg);

U64 generate_captures(Nmovelist *ZZZ, Nboard arg);
U64 generate_captures_2(Nmovelist *ZZZ, Nboard arg);

int evaluate( Nboard arg, int draft, int color, Nboard *rb);

int Ndo_move(Nboard *b, Nmove m);
int Nundo_move(Nboard *b, Nmovelist *ml, Nmove m);

#endif

