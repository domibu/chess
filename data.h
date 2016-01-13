#ifndef domibudata
#define domibudata

#define WIN 18000

typedef unsigned long long U64;

#ifndef BOARD
#define BOARD

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

typedef struct board {
	U64             zobrist;
	U64  info;
	U64             pieceset[17];
} board;

#endif

#ifndef MOVE
#define MOVE

/*
ttentry data:
piecetype       0-2
capttype        3-5
source          6-11
dest            12-17
promo           18-20
enp square      21-25
MVV-LSA		26-30

undo board_1 data:
old_enp         0-3
old_castle      4-7
old_hm          8-13
stm             14
*/

typedef unsigned move;

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

typedef struct node_move_list {
	unsigned char   quietcount, captcount;
        unsigned 	undo;
        U64             old_zobrist;
	move            mdata[256];
} node_move_list;

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

typedef struct TTentry {
        U64 zobrist;
        U64 data;
} TTentry;

#endif

#ifndef NLINE
#define NLINE

typedef struct line {
  int cmove;
  move argmove[50]; // 20 je moveMAX
} line;

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

#define LOG_PATH "."
FILE *log_file;

#endif
