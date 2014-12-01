#ifndef domibudata
#define domibudata

#define WIN 18000
//typedef unsigned U32;

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

#ifndef NBOARD
#define NBOARD

/*
pieceset:
KQRBNP          0-5
white pieces    6
white atacks    7
kqrbnp          8-13
black pieces    14
black atack     15
all pieces      16

info:
enp square      0-3
castle rights   4-7
hm              8-13
stm             14

*/

typedef struct Nboard {
	U64             zobrist;
	U64  info;
	U64             pieceset[17];
} Nboard;

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
MVV-LSA		26-30

undo board data:
old_enp         0-3
old_castle      4-7
old_hm          8-13
stm             14
*/

typedef unsigned Nmove;

#endif

#ifndef NMOVELIST
#define NMOVELIST

/*
undo: 
old_enp         0-3
old_castle      4-7
old_hm          8-13
stm             14
*/

typedef struct Nmovelist {
	unsigned char   quietcount, captcount;
        unsigned 	undo;
        U64             old_zobrist;
	Nmove            mdata[256]; 
} Nmovelist;

#endif

#ifndef NTTENTRY
#define NTTENTRY

/*
depth           0-7
score           8-23
node_type flag  24-25
moveinfo        26-46
age             47

*/

typedef struct nTTentry {
        U64 zobrist;
        U64 data;
} nTTentry;

#endif

#ifndef NLINE
#define NLINE

typedef struct Nline {
  int cmove;
  Nmove argmove[50]; // 20 je moveMAX
} Nline;

#endif


#ifndef EN_STATE
#define EN_STATE

#define OBSERVING 0
#define WAITING 1
#define THINKING 2
#define PONDERING 3
#define PONDERING_COMPLETE 4


int en_state; 

#endif

#endif
