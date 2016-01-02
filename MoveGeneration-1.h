#ifndef domibuMoveGeneration1
#define domibuMoveGeneration1

#include "data-1.h"

extern U64 zobrist[782];

void printmove(move_1 *m_l);
void printmoves(move_1 *m_l);

U64 gen_ho_atack(board_1 arg);
move_1 *generate_movesALLOC(board_1 arg);
char generate_moves(board_1 arg, move_1 *movearray);

void do_move(board_1 *b, move_1 *m);
void do_move(board_1 *b, move_1 *m);

#endif

