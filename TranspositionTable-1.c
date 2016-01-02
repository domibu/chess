#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "MoveGeneration-1.h"
#include "TranspositionTable-1.h"

#define NN 312
#define MM 156
#define UINT64_C(c) (c ## ULL)
#define MATRIX_A UINT64_C(0xB5026F5AA96619E9)
#define UM UINT64_C(0xFFFFFFFF80000000) /* Most significant 33 bits */
#define LM UINT64_C(0x7FFFFFFF) /* Least significant 31 bits */

U64 zobrist[782];
TTentry *TT = NULL;

void setZobrist( board_1 *b)
{
	int it, enp;
	U64 m = 1ULL;

	b->zobrist = 0ULL;

	for ( it = 0; it < 64; it++)
	{
		if (b->b.K & (m << it) )  b->zobrist ^= zobrist[ it];
		else if (b->b.Q & (m << it)) b->zobrist ^= zobrist[ 64 + it];
		else if (b->b.R & (m << it)) b->zobrist ^= zobrist[ 2*64 + it];
		else if (b->b.B & (m << it)) b->zobrist ^= zobrist[ 3*64 + it];
		else if (b->b.N & (m << it)) b->zobrist ^= zobrist[ 4*64 + it];
		else if (b->b.P & (m << it)) b->zobrist ^= zobrist[ 5*64 + it];
		else if (b->w.K & (m << it)) b->zobrist ^= zobrist[ 6*64 + it];
		else if (b->w.Q & (m << it)) b->zobrist ^= zobrist[ 7*64 + it];
		else if (b->w.R & (m << it)) b->zobrist ^= zobrist[ 8*64 + it];
		else if (b->w.B & (m << it)) b->zobrist ^= zobrist[ 9*64 + it];
		else if (b->w.N & (m << it)) b->zobrist ^= zobrist[ 10*64 + it];
		else if (b->w.P & (m << it)) b->zobrist ^= zobrist[ 11*64 + it];
	}
	b->zobrist ^= (b->info & 0x0000000000000002) ? zobrist[ 778] : 0ULL;
	b->zobrist ^= (b->info & 0x0000000000000004) ? zobrist[ 779] : 0ULL;
	b->zobrist ^= (b->info & 0x0000000000000008) ? zobrist[ 780] : 0ULL;
	b->zobrist ^= (b->info & 0x0000000000000010) ? zobrist[ 781] : 0ULL;

	enp = __builtin_ffsll( b->info & 0x000000000000FF00);
	b->zobrist ^= enp ? zobrist[ 768 + enp] : 0ULL;

	b->zobrist ^= b->info & 1ULL ? zobrist[ 777] : 0ULL; 
}

U64 setTT( U64 n)
{
	U64 count;
	n *= 1024*1024;

	count = LargestPrime( n / sizeof(TTentry) );

	TT = calloc( sizeof( TTentry), count);

	return count;
}

TTentry *TTlookup(U64 key)
{
	unsigned ind = key % count_TT;

	if (TT[ ind].zobrist != key) return NULL;

	TThit++;
	return &TT[ ind];
}

void TTstore( U64 zobrist, move_1 *pick, char depth, int score, char flag)
{
	// always replace startegy
	unsigned ind = zobrist % count_TT;

	TT[ ind].zobrist = zobrist;

	if (pick)
	{
		TT[ ind].pick.info = pick->info;
		TT[ ind].pick.dest = pick->dest;
		TT[ ind].pick.from = pick->from;
	}
	else 	
	{
		TT[ ind].pick.info = 0ULL;
		TT[ ind].pick.dest = 0ULL;
		TT[ ind].pick.from = 0ULL;
	}
	TT[ ind].depth = depth;
	TT[ ind].score = score;
	TT[ ind].flag = flag;
}

void printline( line pline)
{
	int it;
	for (it = 0; pline.cmove > it; it++)
	{
		if (pline.argmove[it].info & 1ULL)
			printf(" #%d ", it);
		printmove( &pline.argmove[it]);
	}
	printf("\n");
}

move_1 *TTextractPV( board_1 pos, char n)
{
	char i;
	TTentry *entry;
	move_1 *PV = NULL;
	for ( i = 0; i < n; i++)
	{
		entry = TTlookup( pos.zobrist);
		if (!entry)	break;
		dodaj_move( &PV, &entry->pick);

		if (pos.info & 1ULL)
			printf(" #%llu ", pos.info >> 14);
		printmove( &entry->pick);
		do_move( &pos, &entry->pick);
	}
	printf("\n");
	return PV;
}

void dodaj_move( move_1 **pocetak, move_1 *ind )
{
	move_1 *novi = *pocetak;
	*pocetak = malloc(sizeof(move_1));
	(*pocetak)->next = novi;
	(*pocetak)->from = ind->from;
	(*pocetak)->dest = ind->dest;
	(*pocetak)->info = ind->info;
}
