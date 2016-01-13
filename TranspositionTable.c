#include <stdio.h>
#include <stdlib.h>
#include <math.h>

#include "MoveGeneration.h"
#include "TranspositionTable.h"

#define NN 312
#define MM 156
#define UINT64_C(c) (c ## ULL)
#define MATRIX_A UINT64_C(0xB5026F5AA96619E9)
#define UM UINT64_C(0xFFFFFFFF80000000) /* Most significant 33 bits */
#define LM UINT64_C(0x7FFFFFFF) /* Least significant 31 bits */

U64 zobrist[782];
TTentry *nTT = NULL;

/* The array for the state vector */
static U64 mt[NN];
/* mti==NN+1 means mt[NN] is not initialized */
static int mti=NN+1;

/* initializes mt[NN] with a seed */
void init_genrand64(U64 seed)
{
	mt[0] = seed;
	for (mti=1; mti<NN; mti++)
		mt[mti] =  (UINT64_C (6364136223846793005) * (mt[mti-1] ^ (mt[mti-1] >> 62)) + mti);
}

/* generates a random number on [0, 2^64-1]-interval */
U64 genrand64_int64(void)
{
	int i;
	U64 x;
	static U64 mag01[2]={ 0, 0};

	if (mti >= NN) { /* generate NN words at one time */
		/* if init_genrand64() has not been called, */
		/* a default initial seed is used     */
		if (mti == NN+1)
			init_genrand64(UINT64_C(5489));

		for (i=0;i<NN-MM;i++) {
			x = (mt[i]&UM)|(mt[i+1]&LM);
			mt[i] = mt[i+MM] ^ (x>>1) ^ mag01[(int)(x&UINT64_C(1))];
		}
		for (;i<NN-1;i++) {
			x = (mt[i]&UM)|(mt[i+1]&LM);
			mt[i] = mt[i+(MM-NN)] ^ (x>>1) ^ mag01[(int)(x&UINT64_C(1))];
		}
		x = (mt[NN-1]&UM)|(mt[0]&LM);
		mt[NN-1] = mt[MM-1] ^ (x>>1) ^ mag01[(int)(x&UINT64_C(1))];

		mti = 0;
	}

	x = mt[mti++];

	x ^= (x >> 29) & UINT64_C(0x5555555555555555);
	x ^= (x << 17) & UINT64_C(0x71D67FFFEDA60000);
	x ^= (x << 37) & UINT64_C(0xFFF7EEE000000000);
	x ^= (x >> 43);

	return x;
}


void initZobrist()
{
	int it;
	//srand( 2711984302151);
	init_genrand64( 2711984302151);
	for (it = 0; it < 782; it++)
	{
		zobrist[it] = genrand64_int64();
	}
}

void set_zobrist_keys( board *b)
{
	int it, enp;
	U64 m = 1ULL;

	b->zobrist = 0ULL;

	for ( it = 0; it < 64; it++)
	{
		if (b->pieceset[8] & (m << it) )  b->zobrist ^= zobrist[ it];
		else if (b->pieceset[9] & (m << it)) b->zobrist ^= zobrist[ 64 + it];
		else if (b->pieceset[10] & (m << it)) b->zobrist ^= zobrist[ 2*64 + it];
		else if (b->pieceset[11] & (m << it)) b->zobrist ^= zobrist[ 3*64 + it];
		else if (b->pieceset[12] & (m << it)) b->zobrist ^= zobrist[ 4*64 + it];
		else if (b->pieceset[13] & (m << it)) b->zobrist ^= zobrist[ 5*64 + it];
		else if (b->pieceset[0] & (m << it)) b->zobrist ^= zobrist[ 6*64 + it];
		else if (b->pieceset[1] & (m << it)) b->zobrist ^= zobrist[ 7*64 + it];
		else if (b->pieceset[2] & (m << it)) b->zobrist ^= zobrist[ 8*64 + it];
		else if (b->pieceset[3] & (m << it)) b->zobrist ^= zobrist[ 9*64 + it];
		else if (b->pieceset[4] & (m << it)) b->zobrist ^= zobrist[ 10*64 + it];
		else if (b->pieceset[5] & (m << it)) b->zobrist ^= zobrist[ 11*64 + it];
	}
	b->zobrist ^= (b->info >> 4 & 1ULL) ? zobrist[ 778] : 0ULL;
	b->zobrist ^= (b->info >> 5 & 1ULL) ? zobrist[ 779] : 0ULL;
	b->zobrist ^= (b->info >> 6 & 1ULL) ? zobrist[ 780] : 0ULL;
	b->zobrist ^= (b->info >> 7 & 1ULL) ? zobrist[ 781] : 0ULL;

	enp = b->info >> 1 & 0x0000000000000007;
	b->zobrist ^= b->info & 1ULL ? zobrist[ 768 + enp] : 0ULL;

	b->zobrist ^= (b->info >> 14) & 1ULL ? zobrist[ 777] : 0ULL;
}

U64 set_TT( U64 n)
{
	U64 count;
	n *= 1024*1024;

	count = LargestPrime( n / sizeof(TTentry) );
	nTT = calloc( sizeof( TTentry), count);

	return count;
}

TTentry *lookup_TT(U64 key)
{
	unsigned ind = key % TTentry_count;
	if (nTT[ ind].zobrist != key)
		return NULL;
	TThit++;
	return &nTT[ ind];
}

void store_TT( U64 zobrist, U64 data)
{
	//always replace strategy
	U64 ind = zobrist % TTentry_count;

	TTwr++;
	nTT[ ind].zobrist = zobrist;
	nTT[ ind].data = data;
}

char *print_line_Smith_notation( line pline)
{
	int it;
	static char buff[2048];

	buff[0] = '\0';
	for (it = 0; pline.cmove > it; it++)
	{
		//if (pline.argmove[it].info & 1ULL)	printf(" #%d ", it);
		strcat(buff, print_smith_notation( &pline.argmove[it]));
	}
	strcat(buff, "\n");
	return buff;
}

void print_TTentry( TTentry *arg, board pos)
{
}

char *print_TT_PV( board pos, char n)
{
	char i;
	TTentry *entry;
	move PV = NULL, pick = NULL;
	static char buff[2048];

	buff[0] = '\0';
	for ( i = 0; i < n; i++)
	{
		entry = lookup_TT( pos.zobrist);
		if (!entry)
		{
			strcat(buff, "!entry");
			goto print_TT_PV_end;
		}

		TThit--;

		PV = (entry->data >> 0) & 0x00000003FFFFFF;
		if (i == 0) pick = PV;

		if (PV == 0)
		{
			strcat(buff, "PV==0");
			print_smith_notation(&pick);
			goto print_TT_PV_end;
		}

		strcat(buff, print_smith_notation( &PV));
		do_move( &pos, PV);
	}

print_TT_PV_end :
	strcat(buff, "\n");
	return buff;
}

move TTfind_move( U64 key)
{
	TTentry *entry;

	entry = lookup_TT( key);
	if (!entry)
	{
		Print(0, "!entry\n");
		return 0;
	}
	TThit--;
	return (entry->data >> 0) & 0x00000003FFFFFF;
}

void free_TT()
{
	free(nTT);
}
