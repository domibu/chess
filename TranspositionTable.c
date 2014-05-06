#include <stdio.h>
#include <stdlib.h>
#include <math.h>
#include "TranspositionTable.h"

#define NN 312
#define MM 156
#define UINT64_C(c) (c ## ULL)
#define MATRIX_A UINT64_C(0xB5026F5AA96619E9)
#define UM UINT64_C(0xFFFFFFFF80000000) /* Most significant 33 bits */
#define LM UINT64_C(0x7FFFFFFF) /* Least significant 31 bits */

U64 zobrist[782];

TTentry *TT = NULL;
U64 count_TT = 0ULL;
int TThit = 0;


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

void setZobrist( board *b)
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
		else if (b->b.K & (m << it)) b->zobrist ^= zobrist[ 6*64 + it];
		else if (b->b.Q & (m << it)) b->zobrist ^= zobrist[ 7*64 + it];
		else if (b->b.R & (m << it)) b->zobrist ^= zobrist[ 8*64 + it];
		else if (b->b.B & (m << it)) b->zobrist ^= zobrist[ 9*64 + it];
		else if (b->b.N & (m << it)) b->zobrist ^= zobrist[ 10*64 + it];
		else if (b->b.P & (m << it)) b->zobrist ^= zobrist[ 11*64 + it];
	}
	b->zobrist ^= (b->info & 0x0000000000000002) ? zobrist[ 778] : 0ULL;
	b->zobrist ^= (b->info & 0x0000000000000004) ? zobrist[ 779] : 0ULL;
	b->zobrist ^= (b->info & 0x0000000000000008) ? zobrist[ 780] : 0ULL;
	b->zobrist ^= (b->info & 0x0000000000000010) ? zobrist[ 781] : 0ULL;
	
	enp = __builtin_ffsll( b->info & 0x000000000000FF00);
	b->zobrist ^= enp ? zobrist[ 768 + enp] : 0ULL;
	
	b->zobrist ^= b->info & 1ULL ? zobrist[ 777] : 0ULL; 

}

/* Calculate the largest (odd) prime number not greater than n.
 * I know this is a dumb way of doing it, but it's easily fast enough 
 * considering that it should only need to be done once per game. */
long int LargestPrime(long int n) 
{
  int max_fact = (int)sqrt((double)n), i;
   /* This clause should never be needed, but it's worth keeping for safety */
  if (n<5) return 3;
  n += (n%2) + 1;
  do {
    n-=2;
    for (i=3;i<=max_fact;i+=2) if (n%i == 0) break;
  } while (i<=max_fact);
  return n;
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

void TTstore( U64 zobrist, move *pick, char depth, int score, char flag)
{
	// always replace startegy
	unsigned ind = zobrist % count_TT;
	
	TT[ ind].zobrist = zobrist;

// bit ce direktno move spremljen kad bude bolje spakiran u 16 bita a prijasnji caslte i enpassa cca ce biti ionako posebno ovisno o poziciji a ne o potezu
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
		if (pline.argmove[it].info & 1ULL)	printf(" #%d ", it);
		printmove( &pline.argmove[it]);
    }
    printf("\n");
}

void printNline( Nline pline)
{
    int it;
    for (it = 0; pline.cmove > it; it++)
    {
		//if (pline.argmove[it].info & 1ULL)	printf(" #%d ", it);
		printmoveN( &pline.argmove[it]);
    }
    printf("\n");
}


move *TTextractPV( board pos, char n)
{
	char i;
	TTentry *entry;
	move *PV = NULL;
	for ( i = 0; i < n; i++)
	{
		entry = TTlookup( pos.zobrist);
		if (!entry)	break;
		dodaj_move( &PV, &entry->pick);
	
		//print FM
		if (pos.info & 1ULL)	printf(" #%llu ", pos.info >> 32);
		printmove( &entry->pick);

		do_move( &pos, &entry->pick);
	}
	printf("\n");
	return PV;
}

void dodaj_move( move **pocetak, move *ind )
{

	move *novi = *pocetak;
	*pocetak = malloc(sizeof(move));
	(*pocetak)->next = novi;
	(*pocetak)->from = ind->from;
	(*pocetak)->dest = ind->dest;
	(*pocetak)->info = ind->info;
}

