#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "BitBoardMagic.h"
#include "Interface.h"
#include "MoveGeneration.h"
#include "Search.h"
#include "TranspositionTable.h"


extern move *marray;
extern line pline;
extern Nline Npline;
extern int TThit;

U64 /*mmall = 0,*/ count = 0, mfree = 0;
double razmisljao/*, potroseno_vrijeme*/;
struct timeval start, end;


int main ( int argc, char *argv[])
{

char setposition[6][150] = { 
/*pos 1 ply 8 84,998,978,956*/ "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1",
/*pos 2 ply 6 8.031.647.685 */"r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0",
/* pos 3 ply 7 178.633.661*/"8/2p5/3p4/KP5r/1R3p1k/8/4P1P1/8 w - - 0 0",
/*pos 4 ply 6 706.045.033*/"r3k2r/Pppp1ppp/1b3nbN/nP6/BBP1P3/q4N2/Pp1P2PP/R2Q1RK1 w kq - 0 1",
/*pos 5 ply 3 53.392,4 OK u usporedbi sa roce*/"rnbqkb1r/pp1p1ppp/2p5/4P3/2B5/8/PPP1NnPP/RNBQK2R w KQkq - 0 6"
/*pos 6 ply 6 6,923,051,137*/ "r4rk1/1pp1qppp/p1np1n2/2b1p1B1/2B1P1b1/P1NP1N2/1PP1QPPP/R4RK1 w - - 0 10"};
//char starting_position[100] = "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1";
//char starting_position[100] = "r3k2r/p1ppqpb1/bn2pnp1/3PN3/1p2P3/2N2Q1p/PPPBBPPP/R3K2R w KQkq - 0 0"


int n = 0, score, color;
board cb;
Nboard Ncb;
unsigned char capt, it;
move *fst_pick, *list, *domove = NULL;
Nmove *temp;
unsigned char ln[10], d = 0;


char FEN[100];

	if (argc == 2) n = strtol( argv[1], NULL, 10);
	
        InitializeMoveDatabase();
	initZobrist();
	count_TT = setTT( n);

	Ncb = NimportFEN(setposition[1]);
	cb = importFEN(setposition[1]);
	setZobrist( &cb);
	printf(" TTentry size: %d\n", sizeof(TTentry));
	printf(" prim: %llu\n", count_TT);
	printf(" Nmove size: %d\n", sizeof(Nmove));
	
	NML = malloc( sizeof(Nmovelist)*15);


while (1)
{
  char w[100]; 
	
	scanf("%s",w);

	if (strstr(w,"mTT") != NULL) 
	{	
		scanf("%d", &n);
		printboard(cb);		
		color = -1 + ((cb.info & 1ULL) << 1 );
		//printBits( 8, &cb.info);
		count = 0;
		TThit = 0;

		gettimeofday(&start, NULL);	
		score = mTTnegamax( &cb, &pline, -WIN, +WIN, color, n);
		gettimeofday(&end, NULL);	
		
		//printline( pline);
		TTextractPV( cb, n);
		razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
 		fprintf(stderr, "==%d  time=%.2f v=%.3e c=%d, hits=%d\n", score, razmisljao, count/razmisljao,  count, TThit); 

		//printf("score: %d d%d c%d moves%d", score, n, color, count);
		//printmove( fst_pick);
	}
	else
	if (strstr(w,"msearch") != NULL) 
	{	
		scanf("%d", &n);
		printboard(cb);		
		color = -1 + ((cb.info & 1ULL) << 1 );
		//printBits( 8, &cb.info);
		count = 0;

		gettimeofday(&start, NULL);	
		score = mnegamax( &cb, &pline, -WIN, +WIN, color, n);
		gettimeofday(&end, NULL);	
		
		printline( pline);
		razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
 		fprintf(stderr, "==%d  time=%.2f v=%.3e c=%d\n", score, razmisljao, count/razmisljao,  count); 

		//printf("score: %d d%d c%d moves%d", score, n, color, count);
		//printmove( fst_pick);
	}
	else
	if (strstr(w,"aTT") != NULL) 
	{	
		scanf("%d", &n);
		printboard(cb);		
		color = -1 + ((cb.info & 1ULL) << 1 );
		//printBits( 8, &cb.info);
		count = 0;
		TThit = 0;

		gettimeofday(&start, NULL);	
		marray = malloc( sizeof(move)*216*(n+1) );
		score = TTnegamax( &cb, &pline, -WIN, +WIN, color, n);
		free( marray);
		gettimeofday(&end, NULL);	
		
		//printline( pline);
		TTextractPV( cb, n);
		razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
 		fprintf(stderr, "==%d  time=%.2f v=%.3e c=%d, hits=%d\n", score, razmisljao, count/razmisljao,  count, TThit); 

		//printf("score: %d d%d c%d moves%d", score, n, color, count);
		//printmove( fst_pick);
	}
	else
	if (strstr(w,"nsearch") != NULL) 
	{	
		scanf("%d", &n);
		printNboard(Ncb);		
		color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
		//printBits( 8, &cb.info);
		count = 0;

		gettimeofday(&start, NULL);	
		//marray = malloc( sizeof(move)*216*(n+1) );
		score = nnegamax( &Ncb, &Npline, -WIN, +WIN, color, n);
		//free( marray);
		gettimeofday(&end, NULL);	
		
		printNline( Npline);
		razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
 		fprintf(stderr, "==%d  time=%.2f v=%.3e c=%d\n", score, razmisljao, count/razmisljao,  count); 

		//printf("score: %d d%d c%d moves%d", score, n, color, count);
		//printmove( fst_pick);
	}
	else
	if (strstr(w,"asearch") != NULL) 
	{	
		scanf("%d", &n);
		printboard(cb);		
		color = -1 + ((cb.info & 1ULL) << 1 );
		//printBits( 8, &cb.info);
		count = 0;

		gettimeofday(&start, NULL);	
		marray = malloc( sizeof(move)*216*(n+1) );
		score = negamax( &cb, &pline, -WIN, +WIN, color, n);
		free( marray);
		gettimeofday(&end, NULL);	
		
		printline( pline);
		razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
 		fprintf(stderr, "==%d  time=%.2f v=%.3e c=%d\n", score, razmisljao, count/razmisljao,  count); 

		//printf("score: %d d%d c%d moves%d", score, n, color, count);
		//printmove( fst_pick);
	}
	else
	if (strstr(w,"mdivide") != NULL) 
	{	
		scanf("%d", &n);
		gettimeofday(&start, NULL);	
		count = mdivide_perft(n, &cb);
		gettimeofday(&end, NULL);
		razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
 		fprintf(stderr, "time=%.2f v=%.3e c=%d\n", razmisljao, count/razmisljao,  count); 
	
	}
	else 
	if (strstr(w,"ndivide") != NULL) 
	{	
		scanf("%d", &n);
		gettimeofday(&start, NULL);
		//NML = malloc( sizeof(Nmovelist)*(n+1) );
		count = Ndivide_perft(n, &Ncb, NML);
		//free( NML);
		gettimeofday(&end, NULL);
		razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
 		fprintf(stderr, "time=%.2f v=%.3e c=%d\n", razmisljao, count/razmisljao,  count); 
	
	}

	if (strstr(w,"adivide") != NULL) 
	{	
		scanf("%d", &n);
		gettimeofday(&start, NULL);
		marray = malloc( sizeof(move)*216*(n+1) );
		count = divide_perft(n, &cb);
		free( marray);
		gettimeofday(&end, NULL);
		razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
 		fprintf(stderr, "time=%.2f v=%.3e c=%d\n", razmisljao, count/razmisljao,  count); 
	
	}
	else
	if ( strcmp(w,"nLegal") == 0 )	
	{	
	        //objtoarr( &cb, &Ncb);
		NML = malloc( sizeof(Nmovelist)*(n+1) );
		score = generate_movesN(NML, Ncb);
		capt = NML->captcount;
        	//printf("Bcapt : %d	\n", capt);
		capt++;
        	//printf("Acapt : %d	\n", capt);
                for (it = capt; (it < NML->quietcount) || (it > (NML->captcount)); it++)
		{
        		printf("%d  ", it);
		        printmoveN( &NML->mdata[it]);
        		printf("\n");
				
        	}
        	printf("quiet count: %d	\n", NML->quietcount);
        	
		/*for (n = 255; n > (NML->captcount); n--)     
		{
        		printf("%d  ", 255-n);
		        printmoveN( &NML->mdata[n]);
        		printf("\n");
				
        	}*/
        	printf("capt count: %d	\n", 255-NML->captcount);
        	printf("moves count: %d	\n", NML->quietcount + 255-NML->captcount);
		free( NML);
		
	} 
	else
	if ( strcmp(w,"legal") == 0 )	
	{	
		marray = malloc( sizeof(move)*256);
		score = generate_moves(cb, marray);
		for (n = 0; n < score; n++)     
		{
        		printf("%d  ", n+1);
		        printmove( &marray[n]);
        		printf("\n");
				
        	}
        	printf("moves count: %d	\n", score);
                free(marray);		
	} 
	else
	if ( strstr(w,"setpred") != NULL )	
	{	
		scanf("%d", &n);
		cb = importFEN(setposition[n]);		
		Ncb = NimportFEN(setposition[n]);
		setZobrist( &cb);	
	} 
	else 
	if ( strstr(w,"set") != NULL )	
	{	
		scanf("%[^\n]", FEN);
		cb = importFEN(FEN);		
		Ncb = NimportFEN(FEN);
//    print_state( Ncb);
		setZobrist( &cb);	
		printNboard(Ncb);	
//    objtoarr( &cb, &Ncb);
 	} 
	else
	if ( strstr(w,"do_move") != NULL )	
	{	
		scanf("%d", &n);
		ln[d] = n;
		score = generate_movesN(&NML[d], Ncb);
		
		        //printmoveN( &NML->mdata[n]);
                printf("m\n");
                //temp = &NML->mdata[n];
        	printBits(8, &NML[d].mdata[n]);
		Ndo_move( &Ncb, NML[d].mdata[n]);
		//dodaj_move( &domove, temp);
		printNboard(Ncb);
		d++;		
		
	}
	else if ( strstr(w,"undomove") != NULL )
	{
		d--;
		/*capt = NML[d].captcount;
		capt++;
                for (it = capt; (it < NML[d].quietcount) || (it > (NML[d].captcount)); it++)
		{
        		printf("%d  ", it);
		        printmoveN( &NML[d].mdata[it]);
        		printf("\n");
				
        	}
        	printf("quiet count: %d	\n", NML[d].quietcount);
        	printf("capt count: %d	\n", 255-NML[d].captcount);
        	printf("moves count: %d	\n", NML[d].quietcount + 255-NML[d].captcount);*/
		
		Nundo_move( &Ncb, &NML[d], NML[d].mdata[ln[d]]);
		//izbaci_move( &domove);
		printNboard(Ncb);		

	}/*
	else if ( strstr(w,"NML") != NULL )
	{
		for (n = 0; n < NML->quietcount; n++)     
		{
        		printf("%d  ", n);
		        printmoveN( &NML->mdata[n]);
        		printf("\n");
				
        	}
        	printf("quiet count: %d	\n", NML->quietcount);
        	
		for (n = 255; n > (NML->captcount); n--)     
		{
        		printf("%d  ", 255-n);
		        printmoveN( &NML->mdata[n]);
        		printf("\n");
				
        	}
        	printf("capt count: %d	\n", 255-NML->captcount);
        	printf("moves count: %d	\n", NML->quietcount + 255-NML->captcount);
	}*/
	else if ( strstr(w,"state") != NULL )
	{
//		print_state(cb);
                print_state( Ncb);
	}
	else if ( strstr(w,"Ndisp") != NULL )
	{
		printNboard(Ncb);	
    //print_state( Ncb);
			
	}
	else if ( strstr(w,"disp") != NULL )
	{
		printboard(cb);		
	}
	
	else if ( strstr(w,"change_stm") != NULL )
	{
		cb.info ^= 1LL;
		Ncb.info ^= 1LL << 14;
		setZobrist( &cb);
		printboard(cb);		
	}
	else if ( strstr(w,"new") != NULL )
	{
		//cb = importFEN(starting_position);
		cb = importFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");	
		Ncb = NimportFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");	
		setZobrist( &cb);	
	}
	/*
	else if ( strstr(w,"fen") != NULL )
	{
		printf("%s\n", exportFEN(cb));
	}
	else if ( strstr(w,"zob") != NULL )
	{

		initZobrist();
	}
	else if ( strstr(w,"printz") != NULL )
	{
		int i;
		for (i = 0; i < 782; i++) printf("%d. %llu\n", i, zobrist[ i]);
	}
	else if ( strstr(w,"order") != NULL )
	{
		scanf("%d", &n);
		temp = find_move_by_index(list, n);
		order_moves( &list, temp);
		printmoves( list);
				
	}*/
	else if ( strstr(w,"quit") != NULL )
	{
	        //free(NML);
		break;	
	}
}	

	return 0;
}


