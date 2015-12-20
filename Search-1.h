#ifndef domibuSearch1
#define domibuSearch1

#include "data-1.h"

move *marray;
U64 mfree;

extern U64 count;

int delete_movelist(move *arg);

int eval( board *b);

int mnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth);
int anegamax( board *pos, line *pline, int alpha, int beta, int color, int depth);
int mTTnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth);
int aTTnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth);

U64 Perft(int depth, board *arg);
U64 divide_perft(int depth, board *arg);
U64 mPerft(int depth, board *arg);
U64 mdivide_perft(int depth, board *arg);

#endif
