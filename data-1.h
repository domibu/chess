#ifndef domibudata1
#define domibudata1

#define WIN 18000

typedef unsigned long long U64;


#ifndef BOARD_1
#define BOARD_1

typedef struct piece_set {
	U64 K, Q, R, B, N, P;
	U64 atack, pieces;
} piece_set;

typedef struct board_1 {
	U64 info, zobrist;
	piece_set w, b;
	U64 all_p;
} board_1;

#endif

#ifndef MOVE1
#define MOVE1

typedef struct move_1 {
	U64 info, from, dest;
	struct move1 *next;
} move_1;

#endif

#ifndef TTENTRY1
#define TTENTRY1

typedef struct TTentry_1 {
	U64 zobrist;
	move_1 pick;
	char depth;
	char flag;
	short int score;
} TTentry_1;

#endif

#ifndef LINE1
#define LINE1

typedef struct line_1 {
  int cmove;
  move_1 argmove[50];
} line_1;

#endif

#endif
