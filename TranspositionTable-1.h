#ifndef domibuTranspositionTable1
#define domibuTranspositionTable1

#include "data-1.h"

U64 count_TT_1;
int TThit, TTowr, TTwr;

char *printline_1( line_1 pline);
void dodaj_move_1( move_1 **pocetak, move_1 *ind );


char *TTextractPV_1( board_1 pos, char n);

void init_genrand64(U64 seed);
U64 genrand64_int64(void);
void initZobrist();
void set_zobrist_1(board_1 *b);
U64 setTT_1( U64 n);

TTentry_1 *TTlookup_1(U64 key);
void TTstore_1( U64 zobrist, move_1 *pick, char depth, int score, char flag);

#endif
