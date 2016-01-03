#ifndef domibuSearch
#define domibuSearch

#include "data.h"

int stop;
U64 mfree;
line PV;

extern U64 count;

void sortmoves(Nmovelist *m_list, move PV_move);

int search(board *pos, line *pline, int alpha, int beta, int color, int depth, int draft);
int Quiesce( board *pos, line *pline, int alpha, int beta, int color, int depth, int draft);

int nnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth, int draft);
int pvs_01( board *pos, line *pline, int alpha, int beta, int color, int depth, int is_PV, int draft);
int pvs_02(board *pos, line *pline, int alpha, int beta, int color, int depth, int draft);

int ntestnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth);
int nTTnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth);

U64 NPerft(int depth, board *arg, Nmovelist *ml);
U64 Ndivide_perft(int depth, board *arg, Nmovelist *ml);

#endif
