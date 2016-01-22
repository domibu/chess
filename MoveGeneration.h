#ifndef domibuMoveGeneration
#define domibuMoveGeneration

#include "data.h"

#define QUIET_MV(move, piece, source, destination) ({move &= 0ULL; move ^= piece; move ^= source << 6; move ^= destination << 12;})

#define CAPT_TYP(ho, in, move, f, mask) ({\
		f = (ho[1] >> in) & 1LL;	\
		mask = 0x0000000000000008;	\
		move |= (move & ~mask) | ( -f & mask);	\
		f = (ho[2] >> in) & 1LL;	\
		mask += 8;	\
		move |= (move & ~mask) | ( -f & mask);	\
		f = (ho[3] >> in) & 1LL;	\
		mask += 8;	\
		move |= (move & ~mask) | ( -f & mask);	\
		f = (ho[4] >> in) & 1LL;	\
		mask += 8;	\
		move |= (move & ~mask) | ( -f & mask);	\
		f = (ho[5] >> in) & 1LL;	\
		mask += 8;	\
		move |= (move & ~mask) | ( -f & mask);	\
		})

node_move_list *NML;

extern int stop;
extern U64 zobrist[782];

char *print_state(board arg);

char *print_smith_notation(move *m);
char *print_move_details(move *ff);
char *print_SAN_notation(move *m);

U64 generate_hostile_atackmap(board arg);
char generate_moves(node_move_list *ZZZ, board arg);
char gm1(node_move_list *ZZZ, board arg);

U64 generate_captures(node_move_list *ZZZ, board arg);
U64 gc2(node_move_list *ZZZ, board arg);

int evaluate( board arg, int draft, int color, board *rb);

int do_move(board *b, move m);
int undo_move(board *b, node_move_list *ml, move m);

#endif
