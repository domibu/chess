#include <stdio.h>
#include <string.h>
#include "Interface.h"

char *printboard(board arg)
{
	static char buff[512];
	unsigned i, fm;
	unsigned short hm, enp_;
	char stm, castle[4] = "", enp[2] = "";
	U64 m = 0x8000000000000000;

	buff[0] = '\0';
	sprintf(buff, "%llu\n", arg.zobrist);
	for (i = 0; i<64; i++)
	{
		if (arg.pieceset[5] & (m >> i) ) strcat(buff,"P");
		else if (arg.pieceset[4] & (m >> i)) strcat(buff,"N");
		else if (arg.pieceset[3] & (m >> i)) strcat(buff, "B");
		else if (arg.pieceset[2] & (m >> i)) strcat(buff, "R");
		else if (arg.pieceset[1] & (m >> i)) strcat(buff, "Q");
		else if (arg.pieceset[0] & (m >> i)) strcat(buff, "K");
		else if (arg.pieceset[13] & (m >> i)) strcat(buff, "p");
		else if (arg.pieceset[12] & (m >> i)) strcat(buff, "n");
		else if (arg.pieceset[11] & (m >> i)) strcat(buff, "b");
		else if (arg.pieceset[10] & (m >> i)) strcat(buff, "r");
		else if (arg.pieceset[9] & (m >> i)) strcat(buff, "q");
		else if (arg.pieceset[8] & (m >> i)) strcat(buff, "k");
		else if (~arg.pieceset[16] & (m >> i)) strcat(buff, "¤");
		strcat(buff," ");
		if ((i+1)%8 == 0) strcat(buff,"\n");
	}
	hm = (arg.info >> 8) & 0x00000003F;
	fm = arg.info >> 16;
	stm = (arg.info & (1LL << 14)) ? 119 : 98;

	strcat(castle, 1LL << 4 & arg.info ? "K" : "");
	strcat(castle, 1LL << 5 & arg.info ? "Q" : "");
	strcat(castle, 1LL << 6 & arg.info ? "k" : "");
	strcat(castle, 1LL << 7 & arg.info ? "q" : "");
	enp_ = arg.info >> 1 & 0x07;
	if (!(1LL & arg.info))    strcat(enp, " - ");
	else switch (enp_)
	{
		case 0: strcat(enp, "h");
		case 1: strcat(enp, "g");
		case 2: strcat(enp, "f");
		case 3: strcat(enp, "e");
		case 4: strcat(enp, "d");
		case 5: strcat(enp, "c");
		case 6: strcat(enp, "b");
		case 7: strcat(enp, "a");
			strcat(enp, 1LL << 14 & arg.info ? "6" : "3");
	}

	hm = arg.info >> 8 & 0x000000000000001F;
	fm = arg.info >> 15 & 0x00000000000007FF;

	sprintf(buff, "%s%c %s %s %d %d\n", buff, stm, castle, enp, hm, fm);
	return buff;
}

board NimportFEN(char *fen)
{
	U64 m = 0x8000000000000000;
	board chessb;
	int n, s = 0;
	short hm_,fm_;
	unsigned char  enp;
	char b[90], s_t_m[2], castle[5], enpas[2], hm[3], fm[5];
	sscanf(fen, "%s %s %s %s %s %s", b, s_t_m, castle, enpas, hm, fm);
	resetboard(&chessb);
	for (n = 0; n <= strlen(b); n++ )
		switch (b[n])
		{
			case 'p' : chessb.pieceset[13] |= m >> (n + s); break;
			case 'r' : chessb.pieceset[10] |= m >> (n + s); break;
			case 'n' : chessb.pieceset[12] |= m >> (n + s); break;
			case 'b' : chessb.pieceset[11] |= m >> (n + s); break;
			case 'q' : chessb.pieceset[9] |= m >> (n + s); break;
			case 'k' : chessb.pieceset[8] |= m >> (n + s); break;
			case 'P' : chessb.pieceset[5] |= m >> (n + s); break;
			case 'R' : chessb.pieceset[2] |= m >> (n + s); break;
			case 'N' : chessb.pieceset[4] |= m >> (n + s); break;
			case 'B' : chessb.pieceset[3] |= m >> (n + s); break;
			case 'Q' : chessb.pieceset[1] |= m >> (n + s); break;
			case 'K' : chessb.pieceset[0] |= m >> (n + s); break;
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
	chessb.info = 0LL;
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
	if (enp) chessb.info |= ((enp-1) << 1) ^ 1;

	if (strstr(castle,"K") != NULL)	chessb.info |= (1LL << 4);	else	  chessb.info &= ~(1LL << 4);
	if (strstr(castle,"Q") != NULL)	chessb.info |= (1LL << 5);	else	  chessb.info &= ~(1LL << 5);
	if (strstr(castle,"k") != NULL)	chessb.info |= (1LL << 6);	else	  chessb.info &= ~(1LL << 6);
	if (strstr(castle,"q") != NULL)	chessb.info |= (1LL << 7);	else	  chessb.info &= ~(1LL << 7);
	hm_ = strtol( hm, NULL, 10);
	fm_ = strtol( fm, NULL, 10);
	chessb.info ^= ((hm_) << 8) ^ ((fm_) << 16);
	if (s_t_m[0] == 'w')	chessb.info |= 1LL << 14;	else	  chessb.info &= ~(1LL << 14);
	return chessb;
}

void resetboard(board *arg)
{
	U64 n = ~0xFFFFFFFFFFFFFFFF;
	char i;
	for ( i = 0; i < 17; i++)
	{
		arg->pieceset[i] = n;
	}
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
		case 6: return "O-O";
		case 7: return "O-O-O";
		case 13: return "O-O";
		case 14: return "O-O-O";
	}
}

void square(int index, char *sq)
{
	sq[0] = 104 - index % 8;
	sq[1] = index / 8 + 49;
	sq[2] = 0;
}

void square2(char *sq, int *index)
{
	*index = (sq[1]-49)*8 + (104-sq[0]);
}

char *printBits(size_t const size, void const * const ptr)
{
	unsigned char *b = (unsigned char*) ptr;
	unsigned char byte;
	int i, j;
	static char buff[3072];

	buff[0] = '\0';
	for (i=size-1;i>=0;i--)
	{
		for (j=7;j>=0;j--)
		{
			byte = b[i] & (1<<j);
			byte >>= j;
			sprintf(buff, "%s%u", buff, byte);
		}
		sprintf(buff, "%s\n", buff);
	}
	return buff;
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

int InitializeLogId(char *log_path)
{
	FILE *log_file;
	int log_id;
	char *log_filename;

	log_filename = malloc(sizeof(log_path)+3);

	for (log_id=0; log_id<1000; log_id++)
	{
		sprintf(log_filename, "%s/log.%03d", log_path, log_id);
		log_file = fopen(log_filename, "r");
		if (!log_file)
			break;
		fclose(log_file);
	}
	free(log_filename);

	return log_id;
}

void Print(int vb, char *fmt, ...)
{
	va_list ap;
	char pbuff[2046];
	char tag;

	va_start(ap, fmt);
	if (vb)
		vfprintf(stdout, fmt, ap);
	if (log_file)
	{
		switch (vb)
		{
			case 0: tag = 35; break;
			case 1: tag = 62; break;
			case 2: tag = 60; break;
			default: tag = 126;
		}
		va_start(ap, fmt);
		sprintf(pbuff, "%c%.2045s", tag, fmt);
		vfprintf(log_file, pbuff, ap);
		fflush(log_file);
	}
	va_end(ap);
}
