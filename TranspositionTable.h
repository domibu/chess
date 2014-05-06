#ifndef domibuTranspositionTable
#define domibuTranspositionTable

typedef unsigned long long U64;

#ifndef BOARD
#define BOARD

typedef struct piece_set {
	U64 K, Q, R, B, N, P;
	U64 atack, pieces;
} piece_set;

typedef struct board {
	U64 info, zobrist;
	piece_set w, b;
	U64 all_p;
} board;

#endif

#ifndef MOVE
#define MOVE

typedef struct move {
	U64 info, from, dest;
	struct move *next;
} move;

#endif

#ifndef TTENTRY
#define TTENTRY

typedef struct TTentry {
	U64 zobrist;
	move pick;
	char depth;
	char flag;
	short int score;
	// age;
} TTentry;

#endif

#ifndef LINE
#define LINE

typedef struct line {
  int cmove;
  move argmove[20]; // 20 je moveMAX
} line;

#endif

#ifndef NMOVE
#define NMOVE

/*
ttentry data:
piecetype       0-2
capttype        3-5
source          6-11
dest            12-17
promo           18-20
enp square      21-25


undo board data:
old_enp         0-3
old_castle      4-7
old_hm          8-13
stm             14
*/

typedef unsigned Nmove;

#endif

#ifndef NLINE
#define NLINE

typedef struct Nline {
  int cmove;
  Nmove argmove[20]; // 20 je moveMAX
} Nline;

#endif


TTentry *TT;
U64 count_TT;
int TThit;

void printNline( Nline pline);
void printline( line pline);
void dodaj_move( move **pocetak, move *ind );


move *TTextractPV( board pos, char n);
void init_genrand64(U64 seed);
U64 genrand64_int64(void);
void initZobrist();
void setZobrist( board *b);
long int LargestPrime(long int n);
U64 setTT( U64 n);
TTentry *TTlookup(U64 key);
void TTstore( U64 zobrist, move *pick, char depth, int score, char flag);

#endif
