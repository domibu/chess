#ifndef domibuTranspositionTable1
#define domibuTranspositionTable1

#include "data-1.h"

U64 count_TT;
int TThit, TTowr, TTwr;

void printline( line_1 pline);
void dodaj_move( move_1 **pocetak, move_1 *ind );


move_1 *TTextractPV( board_1 pos, char n);

void init_genrand64(U64 seed);
U64 genrand64_int64(void);
void initZobrist();
void setZobrist(board_1 *b);
U64 setTT( U64 n);

TTentry_1 *TTlookup(U64 key);
void TTstore( U64 zobrist, move_1 *pick, char depth, int score, char flag);

#endif
