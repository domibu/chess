#ifndef domibuMoveGeneration
#define domibuMoveGeneration

typedef unsigned long long U64;
//typedef unsigned U32;

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
hm		15-25

*/

typedef struct Nboard {
	U64             zobrist;
	U64             info;
	U64             pieceset[17];
} Nboard;

#endif

#ifndef MOVE
#define MOVE

typedef struct move {
	U64 info, from, dest;
	struct move *next;
} move;

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
        unsigned	undo;
        U64             old_zobrist;
	Nmove            mdata[256]; 
} Nmovelist;

#endif

Nmovelist *NML;

extern int stop ;
extern U64 zobrist[782];

void printBits(size_t const size, void const * const ptr);
void print_state(Nboard arg);
char *piece(int index);
void square(int index, char *sq);
void square2(char *sq, int *index);
void printmove(move *m_l);
void printmoveN( Nmove *m);
void printmoves(move *m_l);
void printmovedetailsN(Nmove *ff);
void print_move_xboard(Nmove *m);

U64 gen_ho_atack(board arg);
U64 gen_ho_atackN(Nboard arg);
move *generate_movesALLOC(board arg);
char generate_moves(board arg, move *movearray);
char generate_movesN(Nmovelist *ZZZ, Nboard arg);
char generate_movesN_test(Nmovelist *ZZZ, Nboard arg);

int evaluate( Nboard arg, int draft, int color, Nboard *rb);

int Ndo_move(Nboard *b, Nmove m);
int Nundo_move(Nboard *b, Nmovelist *ml, Nmove m);
void do_move(board *b, move *m);
void do_move(board *b, move *m);


#endif

