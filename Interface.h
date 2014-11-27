
#include "data.h"
#ifndef domibuChessInterface
#define domibuChessInterface



void objtoarr(board *arg, Nboard *to);
void arrtoobj(Nboard *to, board *arg);


void printNboard(Nboard arg);
void printboard(board arg);
board importFEN(char *fen);
Nboard NimportFEN(char *fen);
void resetboard(board *arg);
void resetNboard(Nboard *arg);

#endif

