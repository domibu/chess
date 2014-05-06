#include <stdio.h>
#include <stdlib.h>
#include <string.h>

typedef unsigned long long U64;

typedef struct piece_set {
	U64 K, Q, R, B, N, P;
	U64 atack, pieces;
} piece_set;

typedef struct board {
	U64 info;
	piece_set w, b;
	U64 all_p;
} board;



typedef struct move {
	U64 info, from, dest;
	struct move *next;
} move;

U64 magicNumberRook[64] = {
        0xa180022080400230L, 0x40100040022000L, 0x80088020001002L, 0x80080280841000L, 0x4200042010460008L, 0x4800a0003040080L, 0x400110082041008L, 0x8000a041000880L, 0x10138001a080c010L, 0x804008200480L, 0x10011012000c0L, 0x22004128102200L, 0x200081201200cL, 0x202a001048460004L, 0x81000100420004L, 0x4000800380004500L, 0x208002904001L, 0x90004040026008L, 0x208808010002001L, 0x2002020020704940L, 0x8048010008110005L, 0x6820808004002200L, 0xa80040008023011L, 0xb1460000811044L, 0x4204400080008ea0L, 0xb002400180200184L, 0x2020200080100380L, 0x10080080100080L, 0x2204080080800400L, 0xa40080360080L, 0x2040604002810b1L, 0x8c218600004104L, 0x8180004000402000L, 0x488c402000401001L, 0x4018a00080801004L, 0x1230002105001008L, 0x8904800800800400L, 0x42000c42003810L, 0x8408110400b012L, 0x18086182000401L, 0x2240088020c28000L, 0x1001201040c004L, 0xa02008010420020L, 0x10003009010060L, 0x4008008008014L, 0x80020004008080L, 0x282020001008080L, 0x50000181204a0004L, 0x102042111804200L, 0x40002010004001c0L, 0x19220045508200L, 0x20030010060a900L, 0x8018028040080L, 0x88240002008080L, 0x10301802830400L, 0x332a4081140200L, 0x8080010a601241L, 0x1008010400021L, 0x4082001007241L, 0x211009001200509L, 0x8015001002441801L, 0x801000804000603L, 0xc0900220024a401L, 0x1000200608243L
    };

U64 magicNumberBishop[64] = {
        0x2910054208004104L, 0x2100630a7020180L, 0x5822022042000000L, 0x2ca804a100200020L, 0x204042200000900L, 0x2002121024000002L, 0x80404104202000e8L, 0x812a020205010840L, 0x8005181184080048L, 0x1001c20208010101L, 0x1001080204002100L, 0x1810080489021800L, 0x62040420010a00L, 0x5028043004300020L, 0xc0080a4402605002L, 0x8a00a0104220200L, 0x940000410821212L, 0x1808024a280210L, 0x40c0422080a0598L, 0x4228020082004050L, 0x200800400e00100L, 0x20b001230021040L, 0x90a0201900c00L, 0x4940120a0a0108L, 0x20208050a42180L, 0x1004804b280200L, 0x2048020024040010L, 0x102c04004010200L, 0x20408204c002010L, 0x2411100020080c1L, 0x102a008084042100L, 0x941030000a09846L, 0x244100800400200L, 0x4000901010080696L, 0x280404180020L, 0x800042008240100L, 0x220008400088020L, 0x4020182000904c9L, 0x23010400020600L, 0x41040020110302L, 0x412101004020818L, 0x8022080a09404208L, 0x1401210240484800L, 0x22244208010080L, 0x1105040104000210L, 0x2040088800c40081L, 0x8184810252000400L, 0x4004610041002200L, 0x40201a444400810L, 0x4611010802020008L, 0x80000b0401040402L, 0x20004821880a00L, 0x8200002022440100L, 0x9431801010068L, 0x1040c20806108040L, 0x804901403022a40L, 0x2400202602104000L, 0x208520209440204L, 0x40c000022013020L, 0x2000104000420600L, 0x400000260142410L, 0x800633408100500L, 0x2404080a1410L, 0x138200122002900L    
    };

U64 occupancyMaskRook[64] = {
        0x101010101017eL, 0x202020202027cL, 0x404040404047aL, 0x8080808080876L, 0x1010101010106eL, 0x2020202020205eL, 0x4040404040403eL, 0x8080808080807eL, 0x1010101017e00L, 0x2020202027c00L, 0x4040404047a00L, 0x8080808087600L, 0x10101010106e00L, 0x20202020205e00L, 0x40404040403e00L, 0x80808080807e00L, 0x10101017e0100L, 0x20202027c0200L, 0x40404047a0400L, 0x8080808760800L, 0x101010106e1000L, 0x202020205e2000L, 0x404040403e4000L, 0x808080807e8000L, 0x101017e010100L, 0x202027c020200L, 0x404047a040400L, 0x8080876080800L, 0x1010106e101000L, 0x2020205e202000L, 0x4040403e404000L, 0x8080807e808000L, 0x1017e01010100L, 0x2027c02020200L, 0x4047a04040400L, 0x8087608080800L, 0x10106e10101000L, 0x20205e20202000L, 0x40403e40404000L, 0x80807e80808000L, 0x17e0101010100L, 0x27c0202020200L, 0x47a0404040400L, 0x8760808080800L, 0x106e1010101000L, 0x205e2020202000L, 0x403e4040404000L, 0x807e8080808000L, 0x7e010101010100L, 0x7c020202020200L, 0x7a040404040400L, 0x76080808080800L, 0x6e101010101000L, 0x5e202020202000L, 0x3e404040404000L, 0x7e808080808000L, 0x7e01010101010100L, 0x7c02020202020200L, 0x7a04040404040400L, 0x7608080808080800L, 0x6e10101010101000L, 0x5e20202020202000L, 0x3e40404040404000L, 0x7e80808080808000L 
    };

U64 occupancyMaskBishop[64] = {
        0x40201008040200L, 0x402010080400L, 0x4020100a00L, 0x40221400L, 0x2442800L, 0x204085000L, 0x20408102000L, 0x2040810204000L, 0x20100804020000L, 0x40201008040000L, 0x4020100a0000L, 0x4022140000L, 0x244280000L, 0x20408500000L, 0x2040810200000L, 0x4081020400000L, 0x10080402000200L, 0x20100804000400L, 0x4020100a000a00L, 0x402214001400L, 0x24428002800L, 0x2040850005000L, 0x4081020002000L, 0x8102040004000L, 0x8040200020400L, 0x10080400040800L, 0x20100a000a1000L, 0x40221400142200L, 0x2442800284400L, 0x4085000500800L, 0x8102000201000L, 0x10204000402000L, 0x4020002040800L, 0x8040004081000L, 0x100a000a102000L, 0x22140014224000L, 0x44280028440200L, 0x8500050080400L, 0x10200020100800L, 0x20400040201000L, 0x2000204081000L, 0x4000408102000L, 0xa000a10204000L, 0x14001422400000L, 0x28002844020000L, 0x50005008040200L, 0x20002010080400L, 0x40004020100800L, 0x20408102000L, 0x40810204000L, 0xa1020400000L, 0x142240000000L, 0x284402000000L, 0x500804020000L, 0x201008040200L, 0x402010080400L, 0x2040810204000L, 0x4081020400000L, 0xa102040000000L, 0x14224000000000L, 0x28440200000000L, 0x50080402000000L, 0x20100804020000L, 0x40201008040200L     
    };

U64 occupancyVariationRook[64][4096], occupancyVariationBishop[64][512];
U64 magicMovesRook[64][4096], magicMovesBishop[64][512];
U64 movesNight[64];
U64 movesKing[64];

int magicNumberShiftsRook[64] = {
    52,53,53,53,53,53,53,52,53,54,54,54,54,54,54,53,
    53,54,54,54,54,54,54,53,53,54,54,54,54,54,54,53,
    53,54,54,54,54,54,54,53,53,54,54,54,54,54,54,53,
    53,54,54,54,54,54,54,53,52,53,53,53,53,53,53,52
};

int magicNumberShiftsBishop[64] = {
    58,59,59,59,59,59,59,58,59,59,59,59,59,59,59,59,
    59,59,57,57,57,57,59,59,59,59,57,55,55,57,59,59,
    59,59,57,55,55,57,59,59,59,59,57,57,57,57,59,59,
    59,59,59,59,59,59,59,59,58,59,59,59,59,59,59,58
};


void printBits(size_t const size, void const * const ptr);
int popCount (U64 x) ;
void printboard(board arg);
void generateOccupancyVariations(short isRook);
void getsetBits(U64 a, int *p);
void generateMoveDatabase(short isRook);
void generatemovesNight();
void generatemovesKing();
board importFEN(char *fen);
void resetboard(board *arg);
char *exportFEN(board arg);
move *generate_moves(board arg);

int mmall = 0, mfree = 0;


void resetboard(board *arg)
{
	U64 n = 0LL;
	arg->w.P &= n;
	arg->w.N &= n;
	arg->w.R &= n;
	arg->w.K &= n;
	arg->w.Q &= n;
	arg->w.B &= n;
	arg->b.P &= n;
	arg->b.N &= n;
	arg->b.R &= n;
	arg->b.K &= n;
	arg->b.Q &= n;
	arg->b.B &= n;
}

board importFEN(char *fen)
{
	U64 m = 0x8000000000000000;
	board chessb;
	int n, s = 0, enp;
	short hm_,fm_;
	char b[90], s_t_m[2], castle[5], enpas[2], hm[3], fm[5];
	sscanf(fen, "%s %s %s %s %s %s", b, s_t_m, castle, enpas, hm, fm);
	resetboard(&chessb);
	for (n = 0; n <= strlen(b); n++ )
	switch (b[n])
	{
		case 'p' : chessb.b.P |= (m >> n + s); break;
		case 'r' : chessb.b.R |= (m >> n + s); break; 
		case 'n' : chessb.b.N |= (m >> n + s); break; 
		case 'b' : chessb.b.B |= (m >> n + s); break; 
		case 'q' : chessb.b.Q |= (m >> n + s); break; 
		case 'k' : chessb.b.K |= (m >> n + s); break; 
		case 'P' : chessb.w.P |= (m >> n + s); break; 
		case 'R' : chessb.w.R |= (m >> n + s); break; 
		case 'N' : chessb.w.N |= (m >> n + s); break; 
		case 'B' : chessb.w.B |= (m >> n + s); break; 
		case 'Q' : chessb.w.Q |= (m >> n + s); break; 
		case 'K' : chessb.w.K |= (m >> n + s); break; 
		case '/' : s--; break; 
		case '1' : break; 
		case '2' : s++; break; 
		case '3' : s+=2; break; 
		case '4' : s+=3; break; 
		case '5' : s+=4; break; 
		case '6' : s+=5; break; 
		case '7' : s+=6; break; 
		case '8' : s+=7; break; 
	}
	chessb.w.pieces = chessb.w.P ^ chessb.w.N ^ chessb.w.R ^ chessb.w.B ^ chessb.w.K ^ chessb.w.Q;
	chessb.b.pieces = chessb.b.P ^ chessb.b.N ^ chessb.b.R ^ chessb.b.B ^ chessb.b.K ^ chessb.b.Q;
	chessb.all_p = chessb.w.pieces ^ chessb.b.pieces;
	chessb.info = 0LL;	
	if (s_t_m[0] == 'w')	chessb.info |= 1LL;	else	  chessb.info &= ~1LL;
	if (strstr(castle,"K") != NULL)	chessb.info |= (1LL << 1);	else	  chessb.info &= ~(1LL << 1);
	if (strstr(castle,"Q") != NULL)	chessb.info |= (1LL << 2);	else	  chessb.info &= ~(1LL << 2);
	if (strstr(castle,"k") != NULL)	chessb.info |= (1LL << 3);	else	  chessb.info &= ~(1LL << 3);
	if (strstr(castle,"q") != NULL)	chessb.info |= (1LL << 4);	else	  chessb.info &= ~(1LL << 4);
	switch (enpas[0])
	{
		case 'h' : enp = 1 ; break; 
		case 'g' : enp = 2; break; 
		case 'f' : enp = 3; break; 
		case 'e' : enp = 4; break; 
		case 'd' : enp = 5; break; 
		case 'c' : enp = 6; break; 
		case 'b' : enp = 7; break; 
		case 'a' : enp = 8; break;
		default : enp = 0;
	}
	chessb.info &= ~(0X00000000000000FF << 5);
	if (enp) chessb.info |= (1LL << 7 + enp);
	hm_ = strtol( hm, NULL, 10);
	fm_ = strtol( fm, NULL, 10);
	chessb.info ^= ((fm_^0LL) << 32) ^ ((hm_^0LL) << 16);
	printf("len : %d	%s\n", strlen(b), b);
	printf("%s\n", s_t_m);
	printf("%s\n", castle);
	printf("%s\n", enpas);
	printf("%s\n", hm);
	printf("%s\n", fm);
	return chessb;
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


void printboard(board arg)
{
	unsigned i, fm;
	unsigned short hm;
	char stm, castle[4] = "", enp[2] = "";
	U64 m = 0x8000000000000000;
//	U64 m = 0x0000000000000001;

	for (i = 0; i<64; i++)
	{
		if (arg.w.P & (m >> i) ) printf("P"); 
		else if (arg.w.N & (m >> i)) printf("N"); 
		else if (arg.w.B & (m >> i)) printf( "B"); 
		else if (arg.w.R & (m >> i)) printf( "R"); 
		else if (arg.w.Q & (m >> i)) printf( "Q"); 
		else if (arg.w.K & (m >> i)) printf( "K"); 
		else if (arg.b.P & (m >> i)) printf( "p"); 
		else if (arg.b.N & (m >> i)) printf( "n"); 
		else if (arg.b.B & (m >> i)) printf( "b"); 
		else if (arg.b.R & (m >> i)) printf( "r"); 
		else if (arg.b.Q & (m >> i)) printf( "q"); 
		else if (arg.b.K & (m >> i)) printf( "k"); 
		else if (~arg.all_p & (m >> i)) printf( "¤");
		printf(" ");
		//m >>=1;
		if ((i+1)%8 == 0) printf("\n");
	}
	//printf("\n");
	//printBits(8, &arg.info);
	hm = arg.info >> 16;
	fm = arg.info >> 32;
	stm = (arg.info & 1LL) ? 119 : 98;

	strcat(castle, 1LL << 1 & arg.info ? "K" : "");
	strcat(castle, 1LL << 2 & arg.info ? "Q" : "");
	strcat(castle, 1LL << 3 & arg.info ? "k" : "");
	strcat(castle, 1LL << 4 & arg.info ? "q" : "");
	if (!(0xFFLL << 8 & arg.info))    strcat(enp, " - ");
	else 
	{
		if (arg.info & (1LL << 8) ) strcat(enp, "h"); 
		else if (arg.info & (1LL << 9) ) strcat(enp, "g"); 
		else if (arg.info & (1LL << 10) ) strcat(enp, "f"); 
		else if (arg.info & (1LL << 11) ) strcat(enp, "e"); 
		else if (arg.info & (1LL << 12) ) strcat(enp, "d"); 
		else if (arg.info & (1LL << 13) ) strcat(enp, "c"); 
		else if (arg.info & (1LL << 14) ) strcat(enp, "b"); 
		else if (arg.info & (1LL << 15) ) strcat(enp, "a"); 
		strcat(enp, 1LL & arg.info ? "6" : "3");
	}


	printf("stm: %c hm: %d	fm: %d\n", stm, hm, fm);
	printf("cast_fl: %s enp_square: %s\n", castle, enp);
	
}

int popCount (U64 x) 
{
   int count = 0;
   while (x) {
       count++;
       x &= x - 1; // reset LS1B
   }
   return count;
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

void getsetBits(U64 a, int *p)
{
	int i, j = 0;
	int c;
	for (i = 0; i < 64; i++)
	{
		c = 1 & a;
		if (c)	
		{
			p[j] = i;  
			j++;
		}
		a >>= 1;
	}
	p[j] = -1;
}

void generateOccupancyVariations(short isRook)
{
	int i, j, bitRef;
	U64 mask;
	int variationcount;
	int setBitsInMask[13], setBitsInIndex[13];
	int bitCount[64];
	
	for (bitRef = 0; bitRef <= 63; bitRef++)
	{
		mask = isRook ? occupancyMaskRook[bitRef] : occupancyMaskBishop[bitRef];
		//mask = occupancyMaskRook[bitRef];
		getsetBits(mask, setBitsInMask);
			/*printBits(8, &mask);
			for (j = 0; j < 13; j++) printf("%d ", setBitsInMask[j]);*/
		bitCount[bitRef] = popCount(mask);
		variationcount = (1LL << bitCount[bitRef]);
		//printf("bitref %d var %d\n", bitRef, variationcount);
		for (i = 0; i < variationcount; i++)
		{
			if (isRook) occupancyVariationRook[bitRef][i] = 0;  else occupancyVariationBishop[bitRef][i] = 0;
			getsetBits(i, setBitsInIndex);
			//printBits(8, &mask);
			//printBits(4, &i);
			//for (j = 0; j < 13; j++) printf("%d ", setBitsInIndex[j]);
			for (j = 0; setBitsInIndex[j] != -1; j++)
			{
				if (isRook) occupancyVariationRook[bitRef][i] |= 1LL << setBitsInMask[setBitsInIndex[j]];
				else  occupancyVariationBishop[bitRef][i] |= 1LL << setBitsInMask[setBitsInIndex[j]];
			}
			//printBits(8, &occupancyVariationRook[bitRef][i]);
		}
	}
	

}

void generateMoveDatabase(short isRook)
{
        U64 validMoves;
        int variations, bitCount;
        int bitRef, i, j, magicIndex;
    
        for (bitRef=0; bitRef<=63; bitRef++)
        {
            bitCount = isRook ? popCount(occupancyMaskRook[bitRef]) : popCount(occupancyMaskBishop[bitRef]);
            //bitCount = popCount(occupancyMaskRook[bitRef]);
            variations = (1L << bitCount);
            
            for (i=0; i<variations; i++)
            {
                validMoves = 0;
                if (isRook)
                {
                    magicIndex = (occupancyVariationRook[bitRef][i] * magicNumberRook[bitRef]) >> magicNumberShiftsRook[bitRef];

                    for (j=bitRef+8; j<=63; j+=8) { validMoves |= (1LL << j); if ((occupancyVariationRook[bitRef][i] & (1LL << j)) != 0) break; }
                    for (j=bitRef-8; j>=0; j-=8) { validMoves |= (1LL << j); if ((occupancyVariationRook[bitRef][i] & (1LL << j)) != 0) break; }
                    for (j=bitRef+1; j%8!=0; j++) { validMoves |= (1LL << j); if ((occupancyVariationRook[bitRef][i] & (1LL << j)) != 0) break; }
                    for (j=bitRef-1; j%8!=7 && j>=0; j--) { validMoves |= (1LL << j); if ((occupancyVariationRook[bitRef][i] & (1LL << j)) != 0) break; }
                    
                    magicMovesRook[bitRef][magicIndex] = validMoves;
                }
                else
                {
                    magicIndex = (occupancyVariationBishop[bitRef][i] * magicNumberBishop[bitRef]) >> magicNumberShiftsBishop[bitRef];

                    for (j=bitRef+9; j%8!=0 && j<=63; j+=9) { validMoves |= (1LL << j); if ((occupancyVariationBishop[bitRef][i] & (1LL << j)) != 0) break; }
                    for (j=bitRef-9; j%8!=7 && j>=0; j-=9) { validMoves |= (1LL << j); if ((occupancyVariationBishop[bitRef][i] & (1LL << j)) != 0) break; }
                    for (j=bitRef+7; j%8!=7 && j<=63; j+=7) { validMoves |= (1LL << j); if ((occupancyVariationBishop[bitRef][i] & (1LL << j)) != 0) break; }
                    for (j=bitRef-7; j%8!=0 && j>=0; j-=7) { validMoves |= (1LL << j); if ((occupancyVariationBishop[bitRef][i] & (1LL << j)) != 0) break; }

                    magicMovesBishop[bitRef][magicIndex] = validMoves;
                }
            }
        }
}

void generatemovesNight()
{
	int b, r, f;
	for (b = 0; b < 64; b ++)
	{
		movesNight[b] = 0;
		f = b%8;
		r = b/8;
		if (f<7 && r<6) movesNight[b] |= 1LL << b+17; //NNW
		if (f<6 && r<7) movesNight[b] |= 1LL << b+10; //Ww.N
		if (f<6 && r>0) movesNight[b] |= 1LL << b-6; //WWS
		if (f<7 && r>1) movesNight[b] |= 1LL << b-15; //SSW
		if (f>0 && r>1) movesNight[b] |= 1LL << b-17; //SSE
		if (f>1 && r>0) movesNight[b] |= 1LL << b-10; //EES
		if (f>1 && r<7) movesNight[b] |= 1LL << b+6; //EEN
		if (f>0 && r<6) movesNight[b] |= 1LL << b+15; //NNE
	}
}

void generatemovesKing()
{
	int b, r, f;
	for (b = 0; b < 64; b ++)
	{
		movesKing[b] = 0;
		f = b%8;
		r = b/8;
		if (r<7) 	movesKing[b] |= 1LL << b+8; //N
		if (f<7 && r<7) movesKing[b] |= 1LL << b+9; //w.N
		if (f<7) 	movesKing[b] |= 1LL << b+1; //W
		if (f<7 && r>0) movesKing[b] |= 1LL << b-7; //WS
		if (r>0) 	movesKing[b] |= 1LL << b-8; //S
		if (f>0 && r>0) movesKing[b] |= 1LL << b-9; //ES
		if (f>0) 	movesKing[b] |= 1LL << b-1; //E
		if (f>0 && r<7) movesKing[b] |= 1LL << b+7; //EN
	}
}

U64 gen_ho_atack(board arg)
{
	U64 in_N, in_B, in_R, in_Q, in_K, in, at;
	piece_set *fr, *ho; 

	at = 0LL;

	if ((arg.info & 1LL) << 24)  //stm
	{
		fr = &arg.w;
		ho = &arg.b;
	at |= (ho->P & 0xFEFEFEFEFEFEFEFE) >> 9;
	at |= (ho->P & 0x7F7F7F7F7F7F7F7F) >> 7;
//		printf("white move\n");
	}
	else
	{
		fr = &arg.b;
		ho = &arg.w;
	at |= (ho->P & 0xFEFEFEFEFEFEFEFE) << 7;
	at |= (ho->P & 0x7F7F7F7F7F7F7F7F) << 9;
//		printf("black move\n");
	}
	//gledati napadnuta polja bez kralja zbog šahova
	arg.all_p ^= fr->K;  
	//NIGHT
	while (__builtin_popcountll(ho->N))
	{
		in_N = __builtin_ffsll(ho->N)-1;
		at |= movesNight[in_N];
		ho->N &= ~1LL << in_N; 
	}

	//BISHOP
	while (__builtin_popcountll(ho->B))
	{
		in_B = __builtin_ffsll(ho->B)-1;
		in = ((arg.all_p & occupancyMaskBishop[in_B]) * magicNumberBishop[in_B]) >> magicNumberShiftsBishop[in_B];
		at |= magicMovesBishop[in_B][in];
		ho->B &= ~1LL << in_B; 
	}

	//ROOK
	while (__builtin_popcountll(ho->R))
	{
		in_R = __builtin_ffsll(ho->R)-1;
		in = ((arg.all_p & occupancyMaskRook[in_R]) * magicNumberRook[in_R]) >> magicNumberShiftsRook[in_R];
		at |= magicMovesRook[in_R][in] ;
		ho->R &= ~1LL << in_R; 
	}
	
	//QUEEN
	while (__builtin_popcountll(ho->Q))
	{
		in_Q = __builtin_ffsll(ho->Q)-1;
		in_R = ((arg.all_p & occupancyMaskRook[in_Q]) * magicNumberRook[in_Q]) >> magicNumberShiftsRook[in_Q];
		in_B = ((arg.all_p & occupancyMaskBishop[in_Q]) * magicNumberBishop[in_Q]) >> magicNumberShiftsBishop[in_Q];
		at |= (magicMovesRook[in_Q][in_R] ^ magicMovesBishop[in_Q][in_B]);
		ho->Q &= ~1LL << in_Q; 
	}
	//KING
		in_K = __builtin_ffsll(ho->K)-1;
		at |= movesKing[in_K];
	
	//ho->atack &= ~arg.fr->pieces;		//!!!!!!!!!!!!!!!!!!!! samo prazna polja
	return at;
}

move *generate_moves(board arg)
{
	U64 ppb, ppr, bb, at_b, at_r, blank = 0LL, all_pp, mpb, mpr, piece_type, capture, stm, enp;
	U64 check_pieces, check_grid = ~0LL;
	U64 in_N, in_B, in_R, in_Q, in_K, in;
	U64 mN_q, mN_c, mB_q, mB_c, mR_q, mR_c, mQ_q, mQ_c, mK_q, mK_c;
	U64 mP1, mP2, mPE, mPW, mENP, mP1_prom, mPE_prom, mPW_prom, it_prom;
	U64 cas_bit_K, cas_bit_Q, cas_at_K, cas_at_Q, cas_occ_K, cas_occ_Q, promotion, ENProw, enp_P;
	U64 f, mask;
	piece_set *fr, *ho; 
	int in_at_b, in_at_r, king, in_at, in_k, at, pp, mpp, P1, P2, PE, PW;
	move *m_list = NULL, *m;
	//if arg.info  
	stm = (arg.info & 1LL) << 24;
	if (stm )
	{
		fr = &arg.w;
		ho = &arg.b;
		cas_bit_K = 0x0000000000000002;
		cas_bit_Q = 0x0000000000000004;
		cas_at_K =  0x000000000000000E;
		cas_at_Q =  0x0000000000000038;
		cas_occ_K = 0x0000000000000006;
		cas_occ_Q = 0x0000000000000070;
		promotion = 0xFF00000000000000;
		P1 = 8;
		P2 = 16;
		PE = 7;
		PW = 9;
		enp = (arg.info & 0x000000000000FF00) << 32;
		enp_P = enp >> 8;
		ENProw = 0x000000FF00000000;
	}
	else
	{
		fr = &arg.b;
		ho = &arg.w;
		cas_bit_K = 0x0000000000000008;
		cas_bit_Q = 0x0000000000000010;
		cas_at_K =  0x0E00000000000000;
		cas_at_Q =  0x3800000000000000;
		cas_occ_K = 0x0600000000000000;
		cas_occ_Q = 0x7000000000000000;
		promotion = 0x00000000000000FF;
		P1 = -8;
		P2 = -16;
		PE = -9;
		PW = -7;
		enp = (arg.info & 0x000000000000FF00) << 8;
		enp_P = enp << 8;
		ENProw = 0x00000000FF000000;
	}

	king = __builtin_ffsll(fr->K)-1;

	//!!!!!!!!!!!!!!! gradnja atack map, pieces, kasnije neće biti potrebno jer će se update-ati sa make, unmake move!!!!!!!!!!!!!!!!!!!!!!!
	fr->pieces = fr->P ^ fr->N ^ fr->R ^ fr->B ^ fr->K ^ fr->Q;
	ho->pieces = ho->P ^ ho->N ^ ho->R ^ ho->B ^ ho->K ^ ho->Q;
	arg.all_p = fr->pieces ^ ho->pieces;
	ho->atack = gen_ho_atack(arg);
	
	// provjeri jel šah
	if (fr->K & ho->atack)
	{
		//printf("is_check\n");
		if (stm)	in_K = (fr->K & 0xFEFEFEFEFEFEFEFE) << 7 & ho->P | (fr->K & 0x7F7F7F7F7F7F7F7F) << 9 & ho->P;  		
		else	in_K = (fr->K & 0xFEFEFEFEFEFEFEFE) >> 9 & ho->P | (fr->K & 0x7F7F7F7F7F7F7F7F) >> 7 & ho->P;  		

		in_at_b = ( (arg.all_p & occupancyMaskBishop[king]) * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho->B ^ ho->Q); // ~ frijendly pieces ??
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.all_p & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho->R ^ ho->Q); // ~ frijendly pieces ??
		
		in_N = movesNight[king] & ho->N;
		
		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.all_p & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); // ~ frijendly pieces ??
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.all_p & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
				check_grid = magicMovesRook[king][in_at_r] & magicMovesRook[at][in_at] | (1LL << at);
				
			}
			else if (__builtin_popcountll(in_N))
			{
				check_grid = in_N;
			}
			else if (__builtin_popcountll(in_K))
			{
				check_grid = in_K;
			}

		}
		else check_grid = 0LL;

		/*in_K = __builtin_ffsll(fr->K)-1;
		mK_q = movesKing[in_K] & ~fr->pieces & ~ho->atack;
		mK_c = mK_q & ho->pieces;
		mK_q &= ~ ho->pieces;
		while (__builtin_popcountll(mK_q))
		{
			in = __builtin_ffsll(mK_q)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_K);
			m_list->dest = (1LL << in);
			m_list->info ^= 1LL  ^ stm;
			//check fali, zapisi kao const osim stm
			mK_q &= ~m_list->dest; 
		}
		while (__builtin_popcountll(mK_c))
		{
			in = __builtin_ffsll(mK_c)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_K);
			m_list->dest = (1LL << in);
			m_list->info ^= 1LL ^ (1LL << 6) ^ stm;
			//check fali, zapisi kao const osim stm
			mK_c &= ~m_list->dest; 
		}*/
	}
	


	all_pp = 0LL;
	ppb = 0LL;
	ppr = 0LL;
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/	//ho->atack = 0LL; // PRIVREMENO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//bb = blank & occupancyMaskBishop[king];
	//printf("gen_m\n");

	// tražim vezane figure po dijagonali
	in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
	at_b = magicMovesBishop[king][in_at_b] & (ho->B ^ ho->Q); // ~ frijendly pieces ??
	//printBits(8, &at_b);
	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.all_p & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index kralja moze i izvan while	
		in_k = ((arg.all_p & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		ppb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at];
		//ppb = magicMovesBishop[king][in_k] ;
		all_pp ^= ppb;
		//while (__builtin_popcountll(ppb))
		{
			pp = __builtin_ffsll(ppb)-1;
			bb = arg.all_p & ~ (1LL << pp);
			in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
			in_k = ((bb & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
			mpb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at] &  ~ppb ^ (1LL << at); & check_grid;
			mpb &= check_grid;

			if (__builtin_popcountll( (fr->Q ^ fr->B) & ppb )) // vjerovatno ne treba popcount
			{
				//printf("mpb\n");
				//printBits(8, &mpb);
				while ( __builtin_popcountll(mpb))
				{
					mpp = __builtin_ffsll(mpb)-1;
					if (fr->B & ppb)	piece_type = (1LL << 3);
					else piece_type = (1LL << 1);
					//provjera check?
					capture = (1LL << mpp) & ho->pieces ? 1LL << 6 : 0LL;					
					
					//novi potez pp na mpp
					m = m_list;
					m_list = malloc(sizeof(move));
					m_list->next = m;
					m_list->from = ppb;
					m_list->dest = (1LL << mpp);
					m_list->info = piece_type ^ capture ^ stm; // check fali
		//odredi captured piece
		f = (ho->Q >> mpp) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> mpp) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> mpp) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> mpp) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> mpp) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					mpb &= ~m_list->dest; 
					
				}
			}
			//provjera za pješaka
			if ( fr->P & ppb)
			{
				pp = __builtin_ffsll(ppb)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
				if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho->pieces & ~promotion & check_grid);
				else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho->pieces & ~promotion & check_grid );
				// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
	/*printf("pawn_pinned\n");
	bb = (ho->pieces ^ enp); 
	printBits(8, &bb);
	bb = enp ;
	printBits(8, &enp);
	bb = (ppb << 9) ^ (ppb << 7) ;
	printBits(8, &bb);
		printBits(8, &ho->pieces);*/
				
				if (mpp)
				{
					// bez PROMOTION

					m = m_list;
					m_list = malloc(sizeof(move));
					m_list->next = m;
					m_list->from = ppb;
					m_list->dest = (1LL << (mpp-1));
					m_list->info = (1LL << 5) ^ (1LL << 6) ^ stm;// check fali i zapisi kao const osim stm
		//odredi captured piece
		f = (ho->Q >> (mpp-1)) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> (mpp-1)) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> (mpp-1)) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> (mpp-1)) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> (mpp-1)) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					mpb &= ~m_list->dest; //pitanje jel potrebno
				}
				else 
				{
				// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
					if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho->pieces & promotion & check_grid);
					else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho->pieces & promotion & check_grid );
					if (mpp)
					{
						//promotion 
						for (it_prom = 0x0000000000000100; it_prom ^ 0x0000000000001000; it_prom <<= 1)
						{
							in = __builtin_ffsll(mPW_prom)-1;
							m = m_list;
							m_list = malloc(sizeof(move));
							m_list->next = m;
							m_list->from = ppb;
							m_list->dest = (1LL << (mpp-1));
							m_list->info = (1LL << 5) ^ (1LL << 6) ^ (1LL << 15) ^ it_prom ^ stm;
		//odredi captured piece
		f = (ho->Q >> (mpp-1)) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> (mpp-1)) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> (mpp-1)) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> (mpp-1)) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> (mpp-1)) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
						}
						//check fali, zapisi kao const osim stm
						mpb &= ~m_list->dest; //pitanje jel potrebno
					}
					else
					{
						if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho->pieces^enp) & check_grid);
						else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho->pieces ^ enp) & check_grid);
						if (mpp)
						{
							//promotion nije kad je en'passan
							capture = 1LL << 6;
						
							m = m_list;
							m_list = malloc(sizeof(move));
							m_list->next = m;
							m_list->from = ppb;
							m_list->dest = (1LL << (mpp-1));
							m_list->info = (1LL << 5) ^ (1LL << 6) ^ (1LL << 12) ^ stm;
							//check fali, zapisi kao const osim stm
		//odredi captured piece
		f = (ho->Q >> (mpp-1)) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> (mpp-1)) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> (mpp-1)) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> (mpp-1)) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> (mpp-1)) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
							mpb &= ~m_list->dest; //pitanje jel potrebno
						}
					}
					
				}

			}
			//provjera za pješaka

			
			//ppb &= ~ (1LL << pp);
		}
		at_b &= ~(1LL << at); 
	}
	//ppb =  magicMovesBishop[at][in_at];
	//		printf("all_ppb\n");
	//printBits(8, &all_ppb);

	//printBits(8, &ppb);


	// tražim vezane figure po liniji i redu
	in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
	at_r = magicMovesRook[king][in_at_r] & (ho->R ^ ho->Q); // ~ frijendly pieces ??
	//printBits(8, &at_r);
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.all_p & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
		in_k = ((arg.all_p & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
		//ppb = magicMovesBishop[king][in_k] ;
		all_pp ^= ppr;
		//while (__builtin_popcountll(ppb))
		{
			pp = __builtin_ffsll(ppr)-1;
			bb = arg.all_p & ~ (1LL << pp);
			in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
			in_k = ((bb & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
			mpr = magicMovesRook[king][in_k] & magicMovesRook[at][in_at] &  ~ppr ^ (1LL << at); 
			mpr &= check_grid;
			//printf("mpr\n");
			//printBits(8, &mpr);
			
			if (__builtin_popcountll( (fr->Q ^ fr->R) & ppr )) 
			{
				while ( __builtin_popcountll(mpr))
				{
					mpp = __builtin_ffsll(mpr)-1;
					if (fr->R & ppr)	piece_type = (1LL << 2);
					else piece_type = (1LL << 1);
					//provjera check?
					capture = (1LL << mpp) & ho->pieces ? 1LL << 6 : 0LL;					
					
					//novi potez pp na mpp
					m = m_list;
					m_list = malloc(sizeof(move));
					m_list->next = m;
					m_list->from = ppr;
					m_list->dest = (1LL << mpp);
					m_list->info = piece_type ^ capture ^ stm; // check fali
		//odredi captured piece
		f = (ho->Q >> mpp) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> mpp) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> mpp) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> mpp) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> mpp) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					mpr &= ~m_list->dest; 
					
				}
			}


			
			//ppb &= ~ (1LL << pp);
		}
			//provjera za pješaka
			if ( fr->P & ppr)
			{
				pp = __builtin_ffsll(ppr)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
				if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.all_p) << 8) & ~arg.all_p & check_grid);// za dva
				else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.all_p) >> 8) & ~arg.all_p & check_grid);// za dva
	/*printf("pawn_pinned\n");
	bb = ( mpr & ((0x000000000000FF00 << 8) & ~ho->pieces) << 8 & ~ho->pieces); // dodaj ogranicenje za a i h liniju ovisno o smjeru capture
	printBits(8, &bb);
	bb = enp ;
	printBits(8, &enp);
	bb = (ppb << 9) ^ (ppb << 7) ;
	printBits(8, &bb);
		printBits(8, &ho->pieces);*/
		
//PITANJE DA LI PAWN ADVANCE JE QUIET ILI CAPTURE
				if (mpp)
				{
					// nije uracunat PROMOTION

					m = m_list;
					m_list = malloc(sizeof(move));
					m_list->next = m;
					m_list->from = ppr;
					m_list->dest = (1LL << (mpp-1));
					m_list->info = (1LL << 5) ^ (1LL << 15) ^ (1LL << (16 + mpp%8)) ^ stm;// check fali i zapisi kao const osim stm, također izbjegni modulus
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					mpb &= ~m_list->dest; 
				}
				//else 
				{
					mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.all_p) & check_grid
							: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.all_p) & check_grid;// za  jedan 
					if (mpp)
					{
						//promotion nije kad je en'passan
						//capture = 1LL << 6;
					
						m = m_list;
						m_list = malloc(sizeof(move));
						m_list->next = m;
						m_list->from = ppr;
						m_list->dest = (1LL << (mpp-1));
						m_list->info = (1LL << 5) ^ (1LL << 15) ^ stm;
						//check fali, zapisi kao const osim stm
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
						mpb &= ~m_list->dest; 
					}
				}
				
			}
		at_r &= ~(1LL << at); 
	}
	//ppb =  magicMovesBishop[at][in_at];
	//printf("all_pp\n");
	//printBits(8, &all_pp);
	
	//legalnipotezi
	
	//PAWN
	fr->P &= ~ all_pp;
	if (stm)
	{
	mP2 = ((fr->P & 0x000000000000FF00) << 8 & ~arg.all_p) << 8 & ~arg.all_p & check_grid;
	mP1 = fr->P << 8 & ~arg.all_p & check_grid;
	mPE = (fr->P & 0xFEFEFEFEFEFEFEFE) << 7 & ho->pieces & check_grid;
	mPW = (fr->P & 0x7F7F7F7F7F7F7F7F) << 9 & ho->pieces & check_grid;
	mENP = (enp & 0xFEFEFEFEFEFEFEFE & check_grid) >> 9 & fr->P ^ (enp & 0x7F7F7F7F7F7F7F7F & check_grid) >> 7 & fr->P;
	mENP |= (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr->P ^ (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr->P;
	mP1_prom = mP1 & promotion;
	mPE_prom = mPE & promotion;
	mPW_prom = mPW & promotion;
	mP1 &= ~promotion;
	mPE &= ~promotion;
	mPW &= ~promotion;
	}
	else 
	{
	mP2 = ((fr->P & 0x00FF000000000000) >> 8 & ~arg.all_p) >> 8 & ~arg.all_p & check_grid;
	mP1 = fr->P >> 8 & ~arg.all_p & check_grid;
	mPE = (fr->P & 0xFEFEFEFEFEFEFEFE) >> 9 & ho->pieces & check_grid;
	mPW = (fr->P & 0x7F7F7F7F7F7F7F7F) >> 7 & ho->pieces & check_grid;
	mENP = (enp & 0x7F7F7F7F7F7F7F7F & check_grid) << 9 & fr->P ^ (enp & 0xFEFEFEFEFEFEFEFE & check_grid) << 7 & fr->P;
	mENP |= (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr->P ^ (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr->P;
	//mENP = 0LL;
	mP1_prom = mP1 & promotion;
	mPE_prom = mPE & promotion;
	mPW_prom = mPW & promotion;
	mP1 &= ~promotion;
	mPE &= ~promotion;
	mPW &= ~promotion;
	}

	while (__builtin_popcountll(mP2))
	{
		in = __builtin_ffsll(mP2)-1;
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->from = 1LL << (in - P2 );
		m_list->dest = 1LL << in;
		m_list->info = (1LL << 5) ^ (1LL << 15) ^ (1LL << (16 + in%8)) ^ stm;
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
		mP2 &= ~m_list->dest; 
	}
	while (__builtin_popcountll(mP1))
	{
		in = __builtin_ffsll(mP1)-1;
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->from = 1LL << (in - P1 );
		m_list->dest = 1LL << in;
		m_list->info = (1LL << 5) ^ (1LL << 15) ^ stm;
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
		mP1 &= ~m_list->dest; 
	}
	while (__builtin_popcountll(mPW))
	{
		in = __builtin_ffsll(mPW)-1;
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->from = 1LL << (in - PW);
		m_list->dest = 1LL << in;
		m_list->info = (1LL << 5) ^ (1LL << 6) ^ stm;
		//odredi captured piece
		f = (ho->Q >> in) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
		mPW &= ~m_list->dest; 
	}
	while (__builtin_popcountll(mPE))
	{
		in = __builtin_ffsll(mPE)-1;
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->from = 1LL << (in - PE);
		m_list->dest = 1LL << in;
		m_list->info = (1LL << 5) ^ (1LL << 6) ^ stm;
		//odredi captured piece
		f = (ho->Q >> in) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
		mPE &= ~m_list->dest; 
	}
	while (__builtin_popcountll(mENP))
	{
		
		in = __builtin_ffsll(mENP)-1;
		//enp check for 4-5 row pin
		bb = ~(1LL << in | ((enp << 8) >> (16*(stm >> 24)))) & arg.all_p & occupancyMaskRook[king];
		in_K = (bb * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		mR_q = magicMovesRook[king][in_K] & (ho->Q | ho-> R) & ENProw;
		
		if (!(mR_q))
		{
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->from = 1LL << in;
		m_list->dest = enp;
		m_list->info = (1LL << 5) ^ (1LL << 12) ^ stm;
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
		}
		mENP &= ~(1LL << in); 
	}
	while (__builtin_popcountll(mP1_prom))
	{
		for (it_prom = 0x0000000000000100; it_prom ^ 0x0000000000001000; it_prom <<= 1)
		{
		in = __builtin_ffsll(mP1_prom)-1;
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->from = 1LL << (in - P1 );
		m_list->dest = 1LL << in;
		m_list->info = (1LL << 5) ^ (1LL << 15) ^ it_prom ^ stm;
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
		}
		mP1_prom &= ~m_list->dest; 
	}
	while (__builtin_popcountll(mPW_prom))
	{
		for (it_prom = 0x0000000000000100; it_prom ^ 0x0000000000001000; it_prom <<= 1)
		{
		in = __builtin_ffsll(mPW_prom)-1;
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->from = 1LL << (in - PW);
		m_list->dest = 1LL << in;
		m_list->info = (1LL << 5) ^ (1LL << 6) ^ it_prom ^ stm;
		//odredi captured piece
		f = (ho->Q >> in) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		}
		//check fali, zapisi kao const osim stm
		mPW_prom &= ~m_list->dest; 
	}
	while (__builtin_popcountll(mPE_prom))
	{
		for (it_prom = 0x0000000000000100; it_prom ^ 0x0000000000001000; it_prom <<= 1)
		{
		in = __builtin_ffsll(mPE_prom)-1;
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->from = 1LL << (in - PE);
		m_list->dest = 1LL << in;
		m_list->info = (1LL << 5) ^ (1LL << 6) ^ it_prom ^ stm;
		//odredi captured piece
		f = (ho->Q >> in) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		}
		//check fali, zapisi kao const osim stm
		mPE_prom &= ~m_list->dest; 
	}
	
	
	
	
	//NIGHT
	fr->N &= ~ all_pp;
	while (__builtin_popcountll(fr->N))
	{
		in_N = __builtin_ffsll(fr->N)-1;
		mN_q = movesNight[in_N] & ~arg.all_p & check_grid;
		mN_c = movesNight[in_N] & ho->pieces & check_grid;
		
		while (__builtin_popcountll(mN_q))
		{
			in = __builtin_ffsll(mN_q)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_N);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 4) ^ stm;
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mN_q &= ~m_list->dest; 
		}
		while (__builtin_popcountll(mN_c))
		{
			in = __builtin_ffsll(mN_c)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_N);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 4) ^ (1LL << 6) ^ stm;
		//odredi captured piece
		f = (ho->Q >> in) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mN_c &= ~m_list->dest; 
		}
		fr->N &= ~1LL << in_N; 
	}

	//BISHOP
	fr->B &= ~ all_pp;
	while (__builtin_popcountll(fr->B))
	{
		in_B = __builtin_ffsll(fr->B)-1;
		in = ((arg.all_p & occupancyMaskBishop[in_B]) * magicNumberBishop[in_B]) >> magicNumberShiftsBishop[in_B];
		mB_q = magicMovesBishop[in_B][in] & ~ fr->pieces;
		mB_c = mB_q & ho->pieces & check_grid;
		mB_q &= ~ ho->pieces & check_grid;
	
		while (__builtin_popcountll(mB_q))
		{
			in = __builtin_ffsll(mB_q)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_B);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 3) ^ stm;
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mB_q &= ~m_list->dest; 
		}
		while (__builtin_popcountll(mB_c))
		{
			in = __builtin_ffsll(mB_c)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_B);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 3) ^ (1LL << 6) ^ stm;
		//odredi captured piece
		f = (ho->Q >> in) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mB_c &= ~m_list->dest; 
		}
		fr->B &= ~1LL << in_B; 
	}

	//ROOK
	fr->R &= ~ all_pp;
	while (__builtin_popcountll(fr->R))
	{
		in_R = __builtin_ffsll(fr->R)-1;
		in = ((arg.all_p & occupancyMaskRook[in_R]) * magicNumberRook[in_R]) >> magicNumberShiftsRook[in_R];
		mR_q = magicMovesRook[in_R][in] & ~ fr->pieces;
		mR_c = mR_q & ho->pieces & check_grid;
		mR_q &= ~ ho->pieces & check_grid;
	
		while (__builtin_popcountll(mR_q))
		{
			in = __builtin_ffsll(mR_q)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_R);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 2) ^ stm;
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mR_q &= ~m_list->dest; 
		}
		while (__builtin_popcountll(mR_c))
		{
			in = __builtin_ffsll(mR_c)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_R);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 2) ^ (1LL << 6) ^ stm;
		//odredi captured piece
		f = (ho->Q >> in) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mR_c &= ~m_list->dest; 
		}
		fr->R &= ~1LL << in_R; 
	}
	
	//QUEEN
	fr->Q &= ~ all_pp;
	while (__builtin_popcountll(fr->Q))
	{
		in_Q = __builtin_ffsll(fr->Q)-1;
		in_R = ((arg.all_p & occupancyMaskRook[in_Q]) * magicNumberRook[in_Q]) >> magicNumberShiftsRook[in_Q];
		in_B = ((arg.all_p & occupancyMaskBishop[in_Q]) * magicNumberBishop[in_Q]) >> magicNumberShiftsBishop[in_Q];
		mQ_q = (magicMovesRook[in_Q][in_R] ^ magicMovesBishop[in_Q][in_B]) & ~ fr->pieces;
		mQ_c = mQ_q & ho->pieces & check_grid;
		mQ_q &= ~ ho->pieces & check_grid;
	
		while (__builtin_popcountll(mQ_q))
		{
			in = __builtin_ffsll(mQ_q)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_Q);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 1) ^ stm;
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mQ_q &= ~m_list->dest; 
		}
		while (__builtin_popcountll(mQ_c))
		{
			in = __builtin_ffsll(mQ_c)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_Q);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 1) ^ (1LL << 6) ^ stm;
		//odredi captured piece
		f = (ho->Q >> in) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mQ_c &= ~m_list->dest; 
		}
		fr->Q &= ~1LL << in_Q; 
	}
	
	//KING
	//while (__builtin_popcountll(fr->K))
	{
		in_K = __builtin_ffsll(fr->K)-1;
		mK_q = movesKing[in_K] & ~fr->pieces & ~ho->atack;
		mK_c = mK_q & ho->pieces;
		mK_q &= ~ ho->pieces;
		
		while (__builtin_popcountll(mK_q))
		{
			in = __builtin_ffsll(mK_q)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_K);
			m_list->dest = (1LL << in);
			m_list->info = 1LL  ^ stm;
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mK_q &= ~m_list->dest; 
		}
		while (__builtin_popcountll(mK_c))
		{
			in = __builtin_ffsll(mK_c)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_K);
			m_list->dest = (1LL << in);
			m_list->info = 1LL ^ (1LL << 6) ^ stm;
		//odredi captured piece
		f = (ho->Q >> in) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mK_c &= ~m_list->dest; 
		}
		//fr->K &= ~1LL << in_K; 
	}
	//CASTLE
	if ( arg.info & cas_bit_K && !(ho->atack & cas_at_K) && !(arg.all_p & cas_occ_K) )
	{
			
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->info = 1LL << 13 ^ stm;
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
	}
	if ( arg.info & cas_bit_Q && !(ho->atack & cas_at_Q) && !(arg.all_p & cas_occ_Q) )
	{
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->info = 1LL << 14 ^ stm;
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
	}
	//potrebno je odrolati pinned pieces 
	
	
	/*printf("ho.atackmap\n");
	printBits(8, &ho->atack);
	printf("checkgrid\n");
	printBits(8, &check_grid);*/


	return m_list;
}

void do_move(board *b, move *m)
{
	U64 *p, *capt_p, stm, castle, hm, fm, m_cas, f_cas, empty = 0LL, CK, CQ, KS, KR, QR, hKR, hQR, t;
	piece_set *fr, *ho; 
	
	stm = b->info & 0x0000000000000001;
	fr = stm ? &b->w : &b->b;
	ho = stm ? &b->b : &b->w;
	KR = 0x0100000000000000 >> (stm*56);
	QR = 0x8000000000000000 >> (stm*56);
	hKR = 0x0000000000000001 << (stm*56);
	hQR = 0x0000000000000080 << (stm*56);
	//castle = 0x000000000000001E;
	castle = 0LL;
	
	if (!(m->info & 0x0000000000007000))
	{

	
	p = m->info & 1LL      ? &fr->K : p;
	p = m->info & 1LL << 1 ? &fr->Q : p;
	p = m->info & 1LL << 2 ? &fr->R : p;
	p = m->info & 1LL << 3 ? &fr->B : p;
	p = m->info & 1LL << 4 ? &fr->N : p;
	p = m->info & 1LL << 5 ? &fr->P : p;

	//castle
	/*t = (m->dest & KR);
	printf("KR-dest\n");
	printBits(8, &t);
	t = ( m->info & 1LL << 26 );
	printf("igra top\n");
	printBits(8, &t);*/
	//provjera da li igra kralj ili je rokada fr gubi oba castla
	m_cas = 0x000000000000018 >> (stm<<1);
	f_cas = (m->info & 1LL) | (m->info & 0x000000000006000) >> 13;
	castle |= (castle & ~m_cas) | (-f_cas & m_cas);
	//provjera da li je top igrao fr gubi castle
	m_cas = 0x000000000000008 >> (stm<<1);
	f_cas = (m->info & 1LL << 2) && ( m->from & KR); 
	castle |= (castle & ~m_cas) | (-f_cas & m_cas);
	m_cas = 0x000000000000010 >> (stm<<1);
	f_cas = (m->info & 1LL << 2) && ( m->from & QR);
	castle |= (castle & ~m_cas) | (-f_cas & m_cas);
	//provjera da li je top pojeden ho gubi castle 
//nije provjereno jel je mask na vecoj mjeri od uvjeta
	m_cas = 0x000000000000001 << (stm<<1);
	f_cas = (m->dest & hKR) && ( m->info & 1LL << 26 ); 
	castle |= (castle & ~m_cas) | (-f_cas & m_cas);
	m_cas = 0x000000000000002 << (stm<<1);
	f_cas = (m->dest & hQR) && ( m->info & 1LL << 26 );
	castle |= (castle & ~m_cas) | (-f_cas & m_cas);



	//b->info & 0x000000000000006 (m->info & 1LL << 26 && (p->R & 0x0000000000000001) )

	//doing move
	//printf("prije\n");
	//printBits(8, p);
	*p &= ~m->from;
	*p |= m->dest;
	//printf("poslije\n");
	//printBits(8, p);
	
	//removing captured piece
	capt_p = &empty;
	capt_p = m->info & 1LL << 25 ? &ho->Q : capt_p;
	capt_p = m->info & 1LL << 26 ? &ho->R : capt_p;
	capt_p = m->info & 1LL << 27 ? &ho->B : capt_p;
	capt_p = m->info & 1LL << 28 ? &ho->N : capt_p;
	capt_p = m->info & 1LL << 29 ? &ho->P : capt_p;
	*capt_p &= ~m->dest;	
	
	//update all_p
	b->all_p &= ~m->from;
	b->all_p |= m->dest;
	}
	else if (m->info & 0x0000000000001000)
	{
	//hm = ((m->info & 0x0000000000001000));
	//printf("enpuvjet\n");
	//printBits(8, &hm);

		//castle = 0x000000000000001E;
		fr->P &= ~m->from;
		fr->P |= m->dest;
		ho->P &= stm ? ~m->dest >> 8 : ~m->dest << 8;	

		b->all_p &= ~m->from;
		b->all_p |= m->dest;
		b->all_p &= stm ? ~m->dest >> 8 : ~m->dest << 8;

	}
	else if (m->info & 0x0000000000002000)
	{
	//hm = ((m->info & 0x0000000000002000));
	//printf("casK\n");
	//printBits(8, &hm);

		CK = 0x0200000000000000 >> stm*56;
		CQ = 0x2000000000000000 >> stm*56;
		KS = 0x0800000000000000 >> stm*56;

		castle = 0x000000000000018 >> (stm*2);
		fr->K &= ~KS;
		fr->K |= CK;
		fr->R &= ~KR;
		fr->R |= CK << 1;

		b->all_p &= ~KS;
		b->all_p |= CK;
		b->all_p &= ~KR;
		b->all_p |= CK << 1;

	}
	else if (m->info & 0x0000000000006000)
	{
	//hm = ((m->info & 0x0000000000006000));
	//printf("casQ\n");
	//printBits(8, &hm);

		CK = 0x0200000000000000 >> stm*56;
		CQ = 0x2000000000000000 >> stm*56;
		KS = 0x0800000000000000 >> stm*56;

		castle = 0x000000000000018 >> (stm*2);
		fr->K &= ~KS;
		fr->K |= CQ;
		fr->R &= ~QR;
		fr->R |= CQ >> 1;

		b->all_p &= ~KS;
		b->all_p |= CQ;
		b->all_p &= ~QR;
		b->all_p |= CQ >> 1;

	}
	
	
	//half move korak je 1 ako potez nije capture ili pawn advance
	hm = m->info & 0x000000003E008000 ? -(b->info & 0x0000000000FF0000) : (1LL << 16);

	// b_info = stm ^ castle ^ doublepawnpush ^ halfmove ^ fullmove

	b->info &= ~castle; 
	b->info ^= 0x0000000000000001;
	b->info ^= (b->info & 0x000000000000FF00) ^ ( m->info & 0x0000000000FF0000) >> 8;
	b->info += hm;
	b->info += (~ stm & 1LL) << 32;
}

void undo_move(board *b, move *m)
{
	U64 *p, *capt_p, stm, empty = 0LL, hm, CK, CQ, KS, KR, QR;
	piece_set *fr, *ho; 
	
	stm = m->info >> 24 & 0x0000000000000001 ;
	//stm = !(b->info & 1LL) ;
	fr = stm ? &b->w : &b->b;
	ho = stm ? &b->b : &b->w;

	if (!(m->info & 0x0000000000007000))
	{
	
	p = m->info & 1LL      ? &fr->K : p;
	p = m->info & 1LL << 1 ? &fr->Q : p;
	p = m->info & 1LL << 2 ? &fr->R : p;
	p = m->info & 1LL << 3 ? &fr->B : p;
	p = m->info & 1LL << 4 ? &fr->N : p;
	p = m->info & 1LL << 5 ? &fr->P : p;

	//undoing move
	*p &= ~m->dest;
	*p |= m->from;

	//returning captured piece
	capt_p = &empty;
	capt_p = m->info & 1LL << 25 ? &ho->Q : capt_p;
	capt_p = m->info & 1LL << 26 ? &ho->R : capt_p;
	capt_p = m->info & 1LL << 27 ? &ho->B : capt_p;
	capt_p = m->info & 1LL << 28 ? &ho->N : capt_p;
	capt_p = m->info & 1LL << 29 ? &ho->P : capt_p;
	*capt_p |= m->dest;	

	//update all_p
	b->all_p &= m->info & 0x000000003E000000 ? ~0LL : ~m->dest;
	//b->all_p &= ~m->dest;
	b->all_p |= m->from;
	}
	else if (m->info & 0x0000000000001000)
	{

		fr->P &= ~m->dest;
		fr->P |= m->from;
		ho->P |= stm ? m->dest >> 8 : m->dest << 8;	

		b->all_p &= ~m->dest;
		b->all_p |= m->from;
		b->all_p |= stm ? m->dest >> 8 : m->dest << 8;

	}
	else if (m->info & 0x0000000000002000)
	{
	/*hm = ((m->info & 0x0000000000002000));
	printf("casK\n");
	printBits(8, &hm);*/

		KR = 0x0100000000000000 >> stm*56;
		QR = 0x8000000000000000 >> stm*56;
		CK = 0x0200000000000000 >> stm*56;
		CQ = 0x2000000000000000 >> stm*56;
		KS = 0x0800000000000000 >> stm*56;

		fr->K |= KS;
		fr->K &= ~CK;
		fr->R |= KR;
		fr->R &= ~(CK << 1);

		b->all_p |= KS;
		b->all_p &= ~CK;
		b->all_p |= KR;
		b->all_p &= ~(CK << 1);

	}
	else if (m->info & 0x0000000000006000)
	{
	/*hm = ((m->info & 0x0000000000006000));
	printf("casQ\n");
	printBits(8, &hm);*/

		KR = 0x0100000000000000 >> stm*56;//mozda && ili ?
		QR = 0x8000000000000000 >> stm*56;
		CK = 0x0200000000000000 >> stm*56;
		CQ = 0x2000000000000000 >> stm*56;
		KS = 0x0800000000000000 >> stm*56;

		fr->K |= KS;
		fr->K &= ~CQ;
		fr->R |= QR;
		fr->R &= ~(CQ >> 1);

		b->all_p |= KS;
		b->all_p &= ~CQ;
		b->all_p |= QR;
		b->all_p &= ~(CQ >> 1);

	}
	
	b->info -= (b->info & 1LL) << 32;
	b->info ^= 0x0000000000000001;
	// b_info = old_castle old_enp old_hm | b_fm; 
	b->info = (m->info & 0x00FFFF1E00000000) >> 32 | (b->info & 0x000000FF00000001);
}


char *piece(int index)
{
	switch (index) 
	{
	 case 0: return "K";
	 case 1: return "Q";
	 case 2: return "R";
	 case 3: return "B";
	 case 4: return "N";
	 case 5: return "p";
	 case 13: return "O-O";
	 case 14: return "O-O-O";
	}
}

void square(int index, char *sq)
{	
	//char sq[2];
	// h -- 104; a -- 97;
	sq[0] = 104 - index % 8;
	// 1 -- 49; 7 -- 55;
	sq[1] = index / 8 + 49;
	sq[2] = 0;
	
}

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
}

void printmove(move *m_l)
{
	int stm, piece_type, capture, check, enp, castle, pawn_adv, dest, from, count = 0;
	short d_pa;
	char promotion, fr[3], dst[3];

		piece_type = __builtin_ffsll(m_l->info)-1;
		capture = m_l->info >> 6 & 1LL;
		check = m_l->info >> 7 & 1LL;
		promotion = (m_l->info & 0x0000000000000F00) >> 8;
		enp = m_l->info >> 12 & 1LL;
		castle = m_l->info >> 13 & 1LL ? 1 : 0;
		castle += m_l->info >> 14 & 1LL ? 2 : 0;
		pawn_adv = m_l->info >> 15 & 1LL;
		d_pa = (m_l->info & 0x0000000000FF0000) >> 16;
		stm = (m_l->info & 0x0000000001000000) >> 24;;
		//dest = __builtin_ffsll(m_l->dest)-1;
		//from = __builtin_ffsll(m_l->from)-1;
		square( __builtin_ffsll(m_l->dest)-1, dst);
		square( __builtin_ffsll(m_l->from)-1, fr);

		//printf("%s%s%s capt%d enp%d castle%d pawn_adv%d check%d", 
		//	piece(piece_type), fr, dst, capture,enp,castle,pawn_adv,check);
		if (piece_type == 13 || piece_type == 14) 	printf("%s ", piece(piece_type));
		else	printf("%s%s%s ", piece(piece_type), fr, dst);
		//printBits(1, &d_pa);
		//printBits(1, &promotion);
		//printBits(8, &m_l->info);
}

void printmoves(move *m_l)
{
	int stm, piece_type, capture, check, enp, castle, pawn_adv, dest, from, count = 0;
	short d_pa;
	char promotion;
	while ( m_l)
	{
		printf("%d  ", count);
		printmove( m_l);
		printf("\n");
				
		m_l = m_l->next;
		count ++;
	}
	printf("moves count: %d	\n", count);
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

int delete_movelist(move *arg)
{
	move *d; 
	int c = 0;
	while (arg != NULL)
	{
		d = arg;
		arg = (arg)->next;
		free(d);
		c++;
	}
	mfree += c;
	return c;
}

U64 Perft(int depth, board *arg)
{
    move *move_list, *it, *fr;
    int n_moves, i;
    U64 nodes = 0;
 
    if (depth == 1) return delete_movelist( generate_moves( *arg));
 
    move_list = generate_moves( *arg);
    for (it = move_list; it; it = it->next)
    {
        do_move(arg, it);
        nodes += Perft(depth - 1, arg);
        undo_move(arg, it);
    }
    //
    delete_movelist( move_list);
    //printf("obr %d\n", i);
    
    return nodes;
}

U64 divide_perft(int depth, board *arg)
{
    move *move_list, *it, *fr;
    int n_moves, i;
    U64 childs, nodes = 0;
    
    mfree = 0;
 
    if (depth == 0) return 1;
    /*if (depth > 1)
    {
    	printboard(*arg);
	printf("wk\n");
	printBits(8, &arg->w.K);
	printf("bk\n");
	printBits(8, &arg->b.K);
	printf("wp\n");
	printBits(8, &arg->w.P);
	printf("bp\n");
	printBits(8, &arg->b.P);
    } */
    move_list = generate_moves( *arg);
    for (it = move_list; it; it = it->next )
    {
    	childs = 0;
	printmove(it);
        do_move(arg, it);
		//printBits(1, &arg->B);
		//printBits(8, &arg->b.K);
        
        childs += depth > 10 ? divide_perft(depth - 1, arg) : Perft(depth - 1, arg);
        undo_move(arg, it);
	//fr = it;
	printf(" %llu\n", childs);
	nodes += childs;
    }
    delete_movelist(move_list);
    printf("count %llu obr %d\n ", nodes, mfree);
    return nodes;
}

move *kraj( move *pocetak )
{
	while (pocetak->next = NULL) pocetak = pocetak->next;
	return pocetak;
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

move *izbaci_move( move **pocetak)
{
//izbacuje iz skupine PRVI ali ne briše iz memorije!!

	*pocetak = (*pocetak)->next;
}



int main ( int argc, char *argv[])
{
//char starting_position[100] = "rnbqkbnr/pp2pppp/3p4/2p5/3PP3/8/PPP2PPP/RNBQKBNR b KQkq c6 0 3";
//char starting_position[100] = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w 0 1";
//char starting_position[100] = "r1b.Qkb.Nr/1ppp1ppp/p1n5/4p3/B3P3/5N2/PPPP1PPP/RNb.QK2R b KQkq - 1 4";
//char starting_position[100] = "rnbq1rk1/ppp1bppp/5n2/3P4/2B2p2/5N2/PPPP2PP/RNBQKR2 w Q - 6 6";
//char starting_position[100] = "r1b1k2r/pp1p1ppp/2n1p3/8/3Nn2q/2b4P/PPPBBPP1/R2QK2R w KQkq - 0 9";
//char starting_position[100] = "r1bqk2r/pp1p1ppp/2n1pn2/8/1b1NP3/2N5/PPP1BPPP/R1BQK2R w KQkq - 2 6";
//char starting_position[100] = "r1b1k2r/pp1p1ppp/2n1p3/8/1b1Nn2q/2N1B2P/PPP1BPP1/R2QK2R w KQkq - 1 8";
//char starting_position[100] = "4k2r/1b1p1ppp/p3rn2/qp6/2P4b/NP2R2P/P2B1QP1/4KB1R w Kk - 1 12";
//char starting_position[100] = "b3r1kr/3p1ppp/p6n/1p1BQ3/q2RKB2/NPP4P/P5Pb/8 w - - 3 1";
//char starting_position[100] = "4r1k1/6p1/2bp2q1/3PQPPp/pPpRKB2/N7/P6n/8 w - h6 3 1";
//char starting_position[100] = "4r1k1/6p1/2bp2q1/3PQPPp/pPpRKB2/N7/P6n/8 b - b3 0 1";
//char starting_position[100] = "4r1k1/1b5q/3p4/2pPQPpp/pP1RKBP1/N7/P6n/8 w - g6 0 2";
//char starting_position[100] = "r1bqk1nr/ppb2ppp/2n5/3pP3/3QP3/6K1/PPP3PP/RNB2BNR w kq d6 0 8";
//char starting_position[100] = "1n1rk1n1/b1p1pp1p/5b2/1pPPP3/q1PKP2r/3B4/4N1PP/RNBQ3R w - - 2 3";
//char starting_position[100] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 1";
//char starting_position[100] = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
//char starting_position[100] = "rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq - 0 6";
//char starting_position[100] = "rnbqk2r/ppp1nNpp/8/2b5/4p3/2P5/PP1P1PPP/RNBQKB1R w KQkq - 0 1";
//char starting_position[100] = "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10";
//char starting_position[100] = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -";
//char starting_position[100] = "r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1";
//char starting_position[100] = "r2q1rk1/pP1p2pp/Q4n2/bbp1p3/Np6/1B3NBn/pPPP1PPP/R3K2R b KQ - 0 1";
//char starting_position[100] = "6qk/8/8/8/8/8/8/7K b - - 0 1";
//char starting_position[100] = "1n2r1k1/Pb5q/3p4/2pPQPpp/pP1RKBP1/N7/8/8 b - - 5 3";

//char starting_position[100] = "8/2p5/3p4/1P5r/KR3p1k/8/4P1P1/8 b - - 0 1";
//char starting_position[100] = "8/2p5/3p4/KP5r/1R3p1k/6P1/4P3/8 b - - 0 1";

char starting_position[100] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";// pos 1 ply 6 3.195.901.860
//char starting_position[100] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq -";//pos 2 ply 3 97.862
//char starting_position[100] = "8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - -";// pos 3 ply 6 11.030.083
//pos 4 ply 2 264	
//char starting_position[100] = "rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq - 0 6";//pos 5 ply 2 1.352
//pos 6 ply 6 6,923,051,137


U64 blockers;
U64 friendly = 0x6a96810026;
U64 allpieces = 0x71290a7e92810026;
U64 blank = 0LL;
U64 _friendly;
U64 movesR, movesB, movesQ, movesN, movesK;
int n = 0;
U64 index;
board cb;
move *list, *domove = NULL, *temp;
char FEN[100];

	generateOccupancyVariations(1);
	generateMoveDatabase(1);
	generateOccupancyVariations(0);
	generateMoveDatabase(0);
	generatemovesNight();
	generatemovesKing();
	/*allpieces = blank;
	friendly = blank;*/
	if (argc == 2) n = strtol( argv[1], NULL, 10);
	/*blockers =  allpieces & occupancyMaskRook[n];
	index = (blockers * magicNumberRook[n]) >> magicNumberShiftsRook[n];
	movesR = magicMovesRook[n][index] & (~ friendly);
	printf("movesR:\n");
	printBits(8, &movesR);
	
	//printf("i %d l %d ll %d\n", sizeof(I), sizeof(L), sizeof(LL));
	//ispisiplocu(LL);
	//printBits(8, &occupancyMaskBishop[n]);
	blockers =  allpieces & occupancyMaskBishop[n];
	//printBits(8, &b.BBlockers);
	//printBits(8, &magicNumberBishop[n]);
	index = (blockers * magicNumberBishop[n]) >> magicNumberShiftsBishop[n];
	//printBits(8, &index);
	//_friendly = ~ friendly;
	//printBits(8, &_friendly);
	//printBits(8, &magicMovesBishop[n][index]);
	//printBits(8, &magicMovesRook[21][78]);
	printf("bishop:\n");
	movesB = magicMovesBishop[n][index] & (~ friendly);
	printBits(8, &movesB);
	movesQ = movesR ^ movesB;
	printf("queen:\n");
	printBits(8, &movesQ);
	printf("night:\n");
	movesN = movesNight[n];
	printBits(8, &movesN);
	printf("king:\n");
	movesK = movesKing[n];
	printBits(8, &movesK);
	printf("ffs %d\n", __builtin_ffsll(movesK)-1);
	
	printf("popc %d\n", __builtin_popcountll(movesK));*/
	cb = importFEN(starting_position);
while (1)
{
  char w[100]; 
	
	scanf("%s",w);

	if (strstr(w,"divide") != NULL) 
	{	
		scanf("%d", &n);
		divide_perft(n, &cb);
	}
	else
	if ( strcmp(w,"legal") == 0 )	
	{	
		list = generate_moves(cb);
		printmoves( list);
		
	} 
	if ( strstr(w,"set") != NULL )	
	{	
		scanf("%[^\n]", FEN);
		cb = importFEN(FEN);		
	} 
	else
	if ( strstr(w,"do_move") != NULL )	
	{	
		scanf("%d", &n);
		list = generate_moves(cb);
		
		temp = find_move_by_index(list, n);
		do_move( &cb, temp);
		dodaj_move( &domove, temp);
		printboard(cb);		
		
	}
	else if ( strstr(w,"undomove") != NULL )
	{
		undo_move( &cb, domove);
		izbaci_move( &domove);
		printboard(cb);		

	}
	else if ( strstr(w,"state") != NULL )
	{
		print_state(cb);
	}
	else if ( strstr(w,"disp") != NULL )
	{
		printboard(cb);		
	}
	else if ( strstr(w,"change_stm") != NULL )
	{
		cb.info ^= 1LL;
		printboard(cb);		
	}
	else if ( strstr(w,"fen") != NULL )
	{
		printf("%s\n", exportFEN(cb));
	}
	else if ( strstr(w,"quit") != NULL )
	{
		break;	
	}
}	
	/*printboard(cb);
	list = generate_moves(cb);

	printmoves(list);
	//countmoves(generate_moves(cb));

	
	printf("%s\n", exportFEN(cb));

	domove = find_move_by_index(list, n);
	printmove(domove);

	do_move(&cb, domove);
	printboard(cb);
	printf("%s\n", exportFEN(cb));

	undo_move(&cb, domove);
	printboard(cb);
	printf("%s\n", exportFEN(cb));*/
	//printf("depth %d node_count %llu\n", n, Perft(n, &cb));
	//divide_perft(n, &cb);

	return 0;
}

