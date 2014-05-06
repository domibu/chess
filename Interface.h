#ifndef domibuChessInterface
#define domibuChessInterface

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

#ifndef NBOARD
#define NBOARD

/*
pieceset:


info:

*/

typedef struct Nboard {
	U64             zobrist, info;
	U64             pieceset[17];
} Nboard;

#endif


void objtoarr(board *arg, Nboard *to);
void arrtoobj(Nboard *to, board *arg);


void printNboard(Nboard arg);
void printboard(board arg);
board importFEN(char *fen);
Nboard NimportFEN(char *fen);
void resetboard(board *arg);
void resetNboard(Nboard *arg);

#endif

