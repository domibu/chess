#ifndef domibuMoveGeneration1
#define domibuMoveGeneration1

#include "data-1.h"

extern U64 zobrist[782];

char *printmove_1(move_1 *m_l);
char *printmoves_1(move_1 *m_l);

U64 gen_ho_atack_1(board_1 arg);
move_1 *generate_moves_2(board_1 arg);
char generate_moves_1(board_1 arg, move_1 *movearray);

void do_move_1(board_1 *b, move_1 *m);
void undo_move_1(board_1 *b, move_1 *m);

#endif

