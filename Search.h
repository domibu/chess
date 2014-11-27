#ifndef domibuSearch
#define domibuSearch

#include "data.h"

int stop;
move *marray;
U64 mfree;
Nline PV;

extern U64 count;

int delete_movelist(move *arg);

void sortmoves(Nmovelist *m_list, Nmove PV_move);

int eval( board *b);
int search(Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int draft);
int Quiesce( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int draft);

int nnegamax( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int draft);
int pvs_01( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int is_PV, int draft);
int pvs_02(Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int draft);


int ntestnegamax( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth);
int mnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth);
int anegamax( board *pos, line *pline, int alpha, int beta, int color, int depth);
int mTTnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth);
int aTTnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth);
int nTTnegamax( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth);


U64 NPerft(int depth, Nboard *arg, Nmovelist *ml);
U64 Perft(int depth, board *arg);
U64 divide_perft(int depth, board *arg);
U64 mPerft(int depth, board *arg);
U64 mdivide_perft(int depth, board *arg);
U64 Ndivide_perft(int depth, Nboard *arg, Nmovelist *ml);

#endif
