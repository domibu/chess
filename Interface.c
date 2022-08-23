#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <errno.h>
#include "Interface.h"

void objtoarr(board_1 *arg, board *to)
{
	to->pieceset[0] = arg->w.K;
	to->pieceset[1] = arg->w.Q;
	to->pieceset[2] = arg->w.R;
	to->pieceset[3] = arg->w.B;
	to->pieceset[4] = arg->w.N;
	to->pieceset[5] = arg->w.P ;
	to->pieceset[6] = arg->w.pieces ;
	to->pieceset[7] = arg->w.atack;
	to->pieceset[8] = arg->b.K;
	to->pieceset[9] = arg->b.Q;
	to->pieceset[10] = arg->b.R;
	to->pieceset[11] = arg->b.B;
	to->pieceset[12] = arg->b.N;
	to->pieceset[13] = arg->b.P;
	to->pieceset[14] = arg->b.pieces;
	to->pieceset[15] = arg->b.atack;
	to->pieceset[16] = arg->all_p;
}

void arrtoobj(board *to, board_1 *arg)
{
	arg->w.K = to->pieceset[0];
	arg->w.Q = to->pieceset[1];
	arg->w.R = to->pieceset[2];
	arg->w.B = to->pieceset[3];
	arg->w.N = to->pieceset[4];
	arg->w.P = to->pieceset[5];
	arg->w.pieces = to->pieceset[6];
	arg->w.atack = to->pieceset[7];
	arg->b.K = to->pieceset[8];
	arg->b.Q = to->pieceset[9];
	arg->b.R = to->pieceset[10];
	arg->b.B = to->pieceset[11];
	arg->b.N = to->pieceset[12];
	arg->b.P = to->pieceset[13];
	arg->b.pieces = to->pieceset[14];
	arg->b.atack = to->pieceset[15];
	arg->all_p = to->pieceset[16];
}

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

char *printboard_1(board_1 arg)
{
	char buff[128];
	unsigned i, fm;
	unsigned short hm;
	char stm, castle[4] = "", enp[2] = "";
	U64 m = 0x8000000000000000;

	buff[0] = '\0';
	sprintf(buff, "%llu\n", arg.zobrist);
	for (i = 0; i<64; i++)
	{
		if (arg.w.P & (m >> i) ) strcat(buff,"P");
		else if (arg.w.N & (m >> i)) strcat(buff,"N");
		else if (arg.w.B & (m >> i)) strcat(buff, "B");
		else if (arg.w.R & (m >> i)) strcat(buff, "R");
		else if (arg.w.Q & (m >> i)) strcat(buff, "Q");
		else if (arg.w.K & (m >> i)) strcat(buff, "K");
		else if (arg.b.P & (m >> i)) strcat(buff, "p");
		else if (arg.b.N & (m >> i)) strcat(buff, "n");
		else if (arg.b.B & (m >> i)) strcat(buff, "b");
		else if (arg.b.R & (m >> i)) strcat(buff, "r");
		else if (arg.b.Q & (m >> i)) strcat(buff, "q");
		else if (arg.b.K & (m >> i)) strcat(buff, "k");
		else if (~arg.all_p & (m >> i)) strcat(buff, "¤");
		strcat(buff," ");
		if ((i+1)%8 == 0) strcat(buff,"\n");
	}
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

	sprintf(buff, "%s%c %s %s %d %d\n", buff, stm, castle, enp, hm, fm);
	return buff;
}

board_1 importFEN(char *fen)
{
	U64 m = 0x8000000000000000;
	board_1 chessb;
	int n, s = 0, enp;
	short hm_,fm_;
	char b[90], s_t_m[2], castle[5], enpas[2], hm[3], fm[5];
	sscanf(fen, "%s %s %s %s %s %s", b, s_t_m, castle, enpas, hm, fm);
	resetboard_1(&chessb);
	for (n = 0; n <= strlen(b); n++ )
		switch (b[n])
		{
			case 'p' : chessb.b.P |= m >> (n + s); break;
			case 'r' : chessb.b.R |= m >> (n + s); break;
			case 'n' : chessb.b.N |= m >> (n + s); break;
			case 'b' : chessb.b.B |= m >> (n + s); break;
			case 'q' : chessb.b.Q |= m >> (n + s); break;
			case 'k' : chessb.b.K |= m >> (n + s); break;
			case 'P' : chessb.w.P |= m >> (n + s); break;
			case 'R' : chessb.w.R |= m >> (n + s); break;
			case 'N' : chessb.w.N |= m >> (n + s); break;
			case 'B' : chessb.w.B |= m >> (n + s); break;
			case 'Q' : chessb.w.Q |= m >> (n + s); break;
			case 'K' : chessb.w.K |= m >> (n + s); break;
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
	return chessb;
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

void resetboard_1(board_1 *arg)
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
		sprintf(log_filename, "./%s/log.%03d", log_path, log_id);
		log_file = fopen(log_filename, "r");
		if (!log_file)
			break;
		fclose(log_file);
	}
	free(log_filename);

	return log_id;
}

int CreateLogDirectory(char dir_name)
{
    errno = 0;

    int ret = mkdir("logs", S_IRWXU);
    if (ret == -1) {
        switch (errno) {
            case EACCES :
                printf("the parent directory does not allow write\n");
				break;

            case EEXIST:
                printf("pathname already exists\n");
				return 0;

            case ENAMETOOLONG:
                printf("pathname is too long\n");
				break;
				
            default:
                perror("mkdir error\n");
        }
    }	

	return ret;
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
