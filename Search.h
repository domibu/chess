#ifndef domibuSearch
#define domibuSearch

#include "data.h"

int stop;
U64 mfree;
line PV;

extern U64 count;

void sortmoves(node_move_list *m_list, move PV_move);

int search(board *pos, line *pline, int alpha, int beta, int color, int depth, int draft);
int Quiesce( board *pos, line *pline, int alpha, int beta, int color, int depth, int draft);

U64 Ndivide_perft(int depth, board *arg, node_move_list *ml);

#endif
