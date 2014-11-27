#ifndef domibuMoveGeneration
#define domibuMoveGeneration

#include "data.h"

Nmovelist *NML;

extern int stop ;
extern U64 zobrist[782];

void printBits(size_t const size, void const * const ptr);
void print_state(Nboard arg);
char *piece(int index);
void square(int index, char *sq);
void square2(char *sq, int *index);
void printmove(move *m_l);
void printmoveN( Nmove *m);
void printmoves(move *m_l);
void printmovedetailsN(Nmove *ff);
void print_move_xboard(Nmove *m);

U64 gen_ho_atack(board arg);
U64 gen_ho_atackN(Nboard arg);
move *generate_movesALLOC(board arg);
char generate_moves(board arg, move *movearray);
char generate_movesN(Nmovelist *ZZZ, Nboard arg);
char generate_movesN_test(Nmovelist *ZZZ, Nboard arg);

int evaluate( Nboard arg, int draft, int color, Nboard *rb);

int Ndo_move(Nboard *b, Nmove m);
int Nundo_move(Nboard *b, Nmovelist *ml, Nmove m);
void do_move(board *b, move *m);
void do_move(board *b, move *m);


#endif

