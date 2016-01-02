#ifndef domibuSearch1
#define domibuSearch1

#include "data-1.h"

move_1 *marray;
U64 mfree;

extern U64 count;

int delete_movelist(move_1 *arg);

int eval( board_1 *b);

int mnegamax( board_1 *pos, line *pline, int alpha, int beta, int color, int depth);
int anegamax( board_1 *pos, line *pline, int alpha, int beta, int color, int depth);
int mTTnegamax( board_1 *pos, line *pline, int alpha, int beta, int color, int depth);
int aTTnegamax( board_1 *pos, line *pline, int alpha, int beta, int color, int depth);

U64 Perft(int depth, board_1 *arg);
U64 divide_perft(int depth, board_1 *arg);
U64 mPerft(int depth, board_1 *arg);
U64 mdivide_perft(int depth, board_1 *arg);

#endif
