
#ifndef domibuChessInterface
#define domibuChessInterface

#include "data.h"
#include "data-1.h"

void objtoarr(board *arg, Nboard *to);
void arrtoobj(Nboard *to, board *arg);

void printNboard(Nboard arg);
void printboard(board arg);
board importFEN(char *fen);
Nboard NimportFEN(char *fen);
void resetboard(board *arg);
void resetNboard(Nboard *arg);

void printBits(size_t const size, void const * const ptr);
char *piece(int index);
void square(int index, char *sq);
void square2(char *sq, int *index);

long int LargestPrime(long int n);
#endif
