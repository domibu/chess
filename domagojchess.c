#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <math.h>

#define WIN 32000

typedef unsigned long long U64;

typedef struct piece_set {
	U64 K, Q, R, B, N, P;
	U64 atack, pieces;
} piece_set;

typedef struct board {
	U64 info, zobrist;
	piece_set w, b;
	U64 all_p;
} board;

typedef struct move {
	U64 info, from, dest;
	struct move *next;
} move;


typedef struct line {
  int cmove;
  move argmove[20]; // 20 je moveMAX
} line;

typedef struct TTentry {
	U64 zobrist;
	move pick;
	char depth;
	char flag;
	short int score;
	// age;
} TTentry;




void printBits(size_t const size, void const * const ptr);
int popCount (U64 x) ;
void getsetBits(U64 a, int *p);
void printboard(board arg);
board importFEN(char *fen);
char *exportFEN(board arg);
move *generate_movesALLOC(board arg);

void dodaj_move( move **pocetak, move *ind );

//move  pick[20];



void order_moves( move **list, move *hashmove)
{
	move *it, *prev = NULL;
	int s = 1,c = 0;
	it = *list;
	if (it)
	while (it)
	{	

		//printBits( 8, &it->info );
		//printBits( 8, &hashmove->info );
		if ((it->info == (hashmove)->info) 
		&& (it->from == (hashmove)->from) 
		&& (it->dest == (hashmove)->dest))
		{
			//printf("outprev");
			if (prev) 
			{
				prev->next = (hashmove)->next;
				(hashmove)->next = *list;
				*list = hashmove;
			//printf("inprev");
			}
			s = 0;
			goto kraj;
		}
		prev = it;
		it = it->next;
		//c ++;
		//printf(" c %d\n", c );
	}
	kraj :;
	
}


char *exportFEN(board arg)
{
	U64 m = 0x8000000000000000;
	int i, e=0, fm;
	short hm;
	char e_[1], *f = malloc(100*sizeof(char));
	for (i = 0; i<64; i++)
	{
		if (~arg.all_p & (m >> i)) e++;
		else 
		{
		sprintf(e_, "%d", e);
		if (e)	strcat(f, e_);
		e=0;
		if (arg.w.P & (m >> i) ) strcat(f, "P"); 
		else if (arg.w.N & (m >> i)) strcat(f, "N"); 
		else if (arg.w.B & (m >> i)) strcat(f, "B"); 
		else if (arg.w.R & (m >> i)) strcat(f, "R"); 
		else if (arg.w.Q & (m >> i)) strcat(f, "Q"); 
		else if (arg.w.K & (m >> i)) strcat(f, "K"); 
		else if (arg.b.P & (m >> i)) strcat(f, "p"); 
		else if (arg.b.N & (m >> i)) strcat(f, "n"); 
		else if (arg.b.B & (m >> i)) strcat(f, "b"); 
		else if (arg.b.R & (m >> i)) strcat(f, "r"); 
		else if (arg.b.Q & (m >> i)) strcat(f, "q"); 
		else if (arg.b.K & (m >> i)) strcat(f, "k"); 
		}
		//m >>=1;
		if ((i+1)%8 == 0 & i < 63) 
		{
		if (e) 
		{
			sprintf(e_, "%d", e);
			strcat(f, e_);
			e=0;
		}
		 	strcat(f, "/");
		}
	}
	sprintf(e_, "%d", e);
	if (e)	strcat(f, e_);
	
	strcat(f, 1LL & arg.info ? " w " : " b ");
	strcat(f, 1LL << 1 & arg.info ? "K" : "");
	strcat(f, 1LL << 2 & arg.info ? "Q" : "");
	strcat(f, 1LL << 3 & arg.info ? "k" : "");
	strcat(f, 1LL << 4 & arg.info ? "q" : "");
	/*strcat(f,  1LL & arg.info ? "w " : "b ");
	strcat(f,  1LL << 1 & arg.info ? "K" : "");
	strcat(f, 1LL << 2 & arg.info ? "Q" : "");
	strcat(f, 1LL << 3 & arg.info ? "k" : "");
	strcat(f, 1LL << 4 & arg.info ? "q" : "");*/
	if (!(0xFFLL << 8 & arg.info))    strcat(f, " - ");
	else 
	{
		if (arg.info & (1LL << 8) ) strcat(f, " h"); 
		else if (arg.info & (1LL << 9) ) strcat(f, " g"); 
		else if (arg.info & (1LL << 10) ) strcat(f, " f"); 
		else if (arg.info & (1LL << 11) ) strcat(f, " e"); 
		else if (arg.info & (1LL << 12) ) strcat(f, " d"); 
		else if (arg.info & (1LL << 13) ) strcat(f, " c"); 
		else if (arg.info & (1LL << 14) ) strcat(f, " b"); 
		else if (arg.info & (1LL << 15) ) strcat(f, " a"); 
		strcat(f, 1LL & arg.info ? "6 " : "3 ");
	}
	hm = arg.info >> 16;
	fm = arg.info >> 32;
	sprintf(e_, "%d %d\n", hm, fm);	
	strcat(f, e_);
	return f;
}



void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = b[i] & (1<<j);
            byte >>= j;
            printf("%u", byte);
        }
    puts("");
    }
	printf("\n");
}



/*void printmoves_c( int depth)
{
	int count = 0;
	int it;
	  int piece_type;
	  char promotion, fr[3], dst[3];
	for ( it = depth; it; it--)
	{
		printf("#%d ", it);
		piece_type = __builtin_ffsll( pick[ it].info)-1;
		square( __builtin_ffsll( pick[ it].dest)-1, dst);
		square( __builtin_ffsll( pick[ it].from)-1, fr);
		if (piece_type == 13 || piece_type == 14) 	printf("%s ", piece(piece_type));
		else	printf("%s%s%s ", piece(piece_type), fr, dst);
		count ++;
	}
	printf("\nmoves count: %d	\n", count);
}*/


void print_state(board arg)
{
	printBits(8, &arg.w.K);
	printBits(8, &arg.w.Q);
	printBits(8, &arg.w.R);
	printBits(8, &arg.w.B);
	printBits(8, &arg.w.N);
	printBits(8, &arg.w.P);
	printBits(8, &arg.b.K);
	printBits(8, &arg.b.Q);
	printBits(8, &arg.b.R);
	printBits(8, &arg.b.B);
	printBits(8, &arg.b.N);
	printBits(8, &arg.b.P);
	printBits(8, &arg.all_p);
	printBits(8, &arg.w.atack);
	printBits(8, &arg.b.atack);
	printBits(8, &arg.w.pieces);
	printBits(8, &arg.b.pieces);
	printBits(8, &arg.info);
}


move *find_move_by_index( move *l, int in)
{
	while (in)
	{
		l = l->next;
		in--;
	}
	return l;
}

int countmoves(move *m_l)
{
	int count = 0;
	while ( m_l)
	{
		m_l = m_l->next;
		count ++;
	}
	//printf("moves count: %d\n", count);
	return count;
}


move *kraj( move *pocetak )
{
	while (pocetak->next = NULL) pocetak = pocetak->next;
	return pocetak;
}




move *izbaci_move( move **pocetak)
{
//izbacuje iz skupine PRVI ali ne briše iz memorije!!

	*pocetak = (*pocetak)->next;
}



line pline;



