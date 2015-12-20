#ifndef domibudata1
#define domibudata1

#define WIN 18000

typedef unsigned long long U64;


#ifndef BOARD1
#define BOARD1

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

#ifndef MOVE1
#define MOVE1

typedef struct move {
	U64 info, from, dest;
	struct move1 *next;
} move;

#endif

#ifndef TTENTRY1
#define TTENTRY1

typedef struct TTentry {
	U64 zobrist;
	move pick;
	char depth;
	char flag;
	short int score;
} TTentry;

#endif

#ifndef LINE1
#define LINE1

typedef struct line {
  int cmove;
  move argmove[50];
} line;

#endif

#endif
