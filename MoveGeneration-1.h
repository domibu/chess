#ifndef domibuMoveGeneration1
#define domibuMoveGeneration1

#include "data-1.h"

extern U64 zobrist[782];

void printmove(move *m_l);
void printmoves(move *m_l);

U64 gen_ho_atack(board_1 arg);
move *generate_movesALLOC(board_1 arg);
char generate_moves(board_1 arg, move *movearray);

void do_move(board_1 *b, move *m);
void do_move(board_1 *b, move *m);

#endif

