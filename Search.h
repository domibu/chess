#ifndef domibuSearch
#define domibuSearch

#include "data.h"

int stop;
U64 mfree;
Nline PV;

extern U64 count;

void sortmoves(Nmovelist *m_list, Nmove PV_move);

int search(Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int draft);
int Quiesce( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int draft);

int nnegamax( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int draft);
int pvs_01( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int is_PV, int draft);
int pvs_02(Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int draft);

int ntestnegamax( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth);
int nTTnegamax( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth);

U64 NPerft(int depth, Nboard *arg, Nmovelist *ml);
U64 Ndivide_perft(int depth, Nboard *arg, Nmovelist *ml);

#endif
