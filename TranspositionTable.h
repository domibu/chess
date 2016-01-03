#ifndef domibuTranspositionTable
#define domibuTranspositionTable

#include "data.h"

U64 count_nTT;
int TThit, TTowr, TTwr;

void printNline( Nline pline);

move nTTextractPV( board pos, char n);
move TTfind_move( U64 key);

void init_genrand64(U64 seed);
U64 genrand64_int64(void);
void initZobrist();
void nsetZobrist( board *b);
U64 setnTT( U64 n);
void freeTT();

nTTentry *nTTlookup(U64 key);
void nTTstore( U64 zobrist, U64 data);

#endif
