#ifndef domibuTranspositionTable
#define domibuTranspositionTable

#include "data.h"

nTTentry *nTT;
TTentry *TT;
U64 count_TT, count_nTT;
int TThit, TTowr, TTwr;

void printNline( Nline pline);
void printline( line pline);
void dodaj_move( move **pocetak, move *ind );


move *TTextractPV( board pos, char n);
Nmove nTTextractPV( Nboard pos, char n);
Nmove TTfind_move( U64 key);

void init_genrand64(U64 seed);
U64 genrand64_int64(void);
void initZobrist();
void setZobrist( board *b);
void nsetZobrist( Nboard *b);
long int LargestPrime(long int n);
U64 setTT( U64 n);

TTentry *TTlookup(U64 key);
void TTstore( U64 zobrist, move *pick, char depth, int score, char flag);

nTTentry *nTTlookup(U64 key);
void nTTstore( U64 zobrist, U64 data);


#endif
