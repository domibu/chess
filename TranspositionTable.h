#ifndef domibuTranspositionTable
#define domibuTranspositionTable

#include "data.h"

U64 TTentry_count;
int TThit, TTowr, TTwr;

void print_line_Smith_notation( line pline);

move print_TT_PV( board pos, char n);
move TTfind_move( U64 key);

void init_genrand64(U64 seed);
U64 genrand64_int64(void);
void initZobrist();
void set_zobrist_keys( board *b);
U64 set_TT( U64 n);
void free_TT();

TTentry *lookup_TT(U64 key);
void store_TT( U64 zobrist, U64 data);

#endif
