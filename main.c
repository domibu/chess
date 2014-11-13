#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
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

#define MEMORY_MAX 2048
int n=200, memory = 64, score, color;
#define SD_MAX 50
int search_depth = SD_MAX;
int post=1;
extern int stop;
int pondering=0;
int ping=0;
//unsigned time=10000;
int computer=0, cores=0, egtpath=0, option=0;

int input_max_length = 512;
board cb;
Nboard Ncb;

struct t_man {
	unsigned en_time;
	unsigned op_time;
	unsigned fm;
	unsigned ctrl_style;
	unsigned moves_left;
	unsigned est_moves_game;
	unsigned moves_per_ctrl;
	unsigned increment;
	unsigned target;
	unsigned lag;
} t_manager;


/*
history.tag:
0-31 move_data
32-64 undo_move_data
*/
#define HISTORY_MAX 1000
struct hist {
	U64 tag[HISTORY_MAX];
	unsigned curr;
} history;

pthread_t threads[4];
timer_t timer_id;
struct itimerspec its;
struct sigevent sev;
struct sigaction setup_action;
sigset_t block_mask;
void catch_alrm();
#define SIG_TRGT SIGRTMIN

void *Thinking(void *void_ptr )
{
	int i;
	struct itimerspec curr_tick;
	Nmove pm;

	printf("mem %d\n", memory);
	
	count_nTT = setnTT( memory);

	for (i = 1; (en_state == THINKING) && (i <= search_depth); i++)
	{

		count = 0;
		/*TThit = 0;
		TTowr = 0;
		TTwr = 0;*/
		
		score = search( &Ncb, &Npline, -WIN -300, +WIN +300, color, i, 0);


		timer_gettime(timer_id, &curr_tick);
		double search_time = its.it_value.tv_sec + (double)its.it_value.tv_nsec/1000000000;
		search_time -= curr_tick.it_value.tv_sec + (double)curr_tick.it_value.tv_nsec/1000000000;

		if (en_state == THINKING)
			if (post)
			{
				fprintf(stdout, "%d	%d	%.2f	%llu	", i, score, search_time*100, count);
				pm = nTTextractPV( Ncb, i);
				/*printf("=%d= ", i);
				printmoveN(&pm);
				printf("\n");*/
				//memcpy( &PV, &Npline, sizeof(Nline));
			}
			else	pm = TTfind_move( Ncb.zobrist); 

		//gettimeofday(&end, NULL);
		//printNline(PV);
		//**MAKE MOVE**
		if ((score >= WIN) && (en_state == THINKING))
		{
			en_state = PONDERING;
			//printf(printf("***domibu WINS***"););
		}
		if ((score <= -WIN) && (en_state == THINKING))
		{
			en_state = PONDERING; 
		}
		//printNline(PV);
		/////PRINTING THINKING OUTPUT/////////////
	
		//	CHECK post for posting thinking output	//////////////////
			//printNline(PV);
			//printf("\n");
		//fprintf(stdout, "==%d pvs=%d time=%.2f v=%.3e c=%llu, hits=%d  writes %d overwrites %d\n", score, i, razmisljao, count/razmisljao,  count, TThit, TTwr, TTowr);
		//fflush(stdout);
	}
	// DOING MOVE	///////////////////
	if (en_state == PONDERING)
	{
		//	update history	//////////
		history.tag[history.curr] ^= 0ULL;
		history.tag[history.curr] = Ncb.info << 32;
		history.tag[history.curr] = pm;
		history.curr++;

		t_manager.fm += (color == -1) ? 1 : 0;
		//printf("STOPING - doingmove\n");
		Ndo_move( &Ncb, pm);
		//print_move_xboard( &PV.argmove[0]);
		printf("move ");
		printmoveN(&pm);
		printf("\n");
	}
	//razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;		
	free(nTT);
	//printf("exiting thinking thread\n");
	pthread_exit(NULL);
}

unsigned char capt, it;
move *fst_pick, *list, *domove = NULL;
Nmove *temp;
unsigned char ln[10], d = 0;



char FEN[100];
char w[512]; 


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


/////////////  PARSING COMMAND LINE ARGUMENTS //////////////////
//if (argc == 2) n = strtol( argv[1], NULL, 10);
// USE UNBEFFERED STD STREAMS ///////
//setvbuf(stdin, NULL, _IOLBF, BUFSIZ);	
setbuf(stdout, NULL);
setbuf(stdin, NULL);

sigemptyset (&block_mask);
/* Block other terminal-generated signals while handler runs. */
//sigaddset (&block_mask, SIGINT);
//sigaddset (&block_mask, SIGQUIT);
setup_action.sa_handler = catch_alrm;
setup_action.sa_mask = block_mask;
setup_action.sa_flags = SA_RESTART;
sigaction (SIG_TRGT, &setup_action, NULL);

sev.sigev_notify = SIGEV_SIGNAL;
sev.sigev_signo = SIG_TRGT;
sev.sigev_value.sival_ptr = &timer_id;
timer_create(CLOCK_REALTIME, &sev, &timer_id);

t_manager.ctrl_style = 0;
t_manager.moves_per_ctrl = 40;
t_manager.en_time = 5*60*100;
t_manager.lag = 10;
t_manager.est_moves_game = 16;

InitializeMoveDatabase();
initZobrist();
//count_TT = setTT( memory);
//count_nTT= setnTT( memory);

Ncb = NimportFEN(setposition[0]);
//cb = importFEN(setposition[1]);
//setZobrist( &cb);
nsetZobrist( &Ncb);
/*fprintf(stderr, "TTentry size: %d\n", sizeof(TTentry));
fprintf(stderr, " prim: %llu nprim %llu\n", count_TT, count_nTT);
fprintf(stderr, " Nmove size: %d\n", sizeof(Nmove));
fprintf(stderr, " U64 size: %d\n", sizeof(U64));*/
NML = malloc( sizeof(Nmovelist)*search_depth);

en_state = WAITING;


fgets(w, input_max_length, stdin);

if (strstr(w,"xboard") != NULL)	chess_engine_communication_protocol();

else

while (1)
{

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
	fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu, hits=%d\n", score, razmisljao, count/razmisljao,  count, TThit); 

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
	fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu\n", score, razmisljao, count/razmisljao,  count); 

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
	TTwr = 0;

	gettimeofday(&start, NULL);	
	marray = malloc( sizeof(move)*216*(n+1) );
	score = aTTnegamax( &cb, &pline, -WIN, +WIN, color, n);
	free( marray);
	gettimeofday(&end, NULL);	
	
	//printline( pline);
	TTextractPV( cb, n);
	razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
	fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu, hits=%d\n", score, razmisljao, count/razmisljao,  count, TThit); 

	//printf("score: %d d%d c%d moves%d", score, n, color, count);
	//printmove( fst_pick);
}
else
if (strstr(w,"nTT") != NULL)
{
	scanf("%d", &n);
	printNboard(Ncb);
	color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
	//printBits( 8, &cb.info);
	count = 0;
	TThit = 0;
	TTowr = 0;
	TTwr = 0;

	gettimeofday(&start, NULL);
	//marray = malloc( sizeof(move)*216*(n+1) );
	score = nTTnegamax( &Ncb, &Npline, -WIN, +WIN, color, n);
	//free( marray);
	gettimeofday(&end, NULL);

	//printline( pline);
	//Nmove PV_move;
	int log = strstr(w,"-log") ? 100 : 0;
	nTTextractPV( Ncb, n);
	//!!!!!!!!! print na stderror PV_end !!!!!!!!!!!!!!!!
	razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
	fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu, hits=%d  writes %d overwrites %d\n", score, razmisljao, count/razmisljao,  count, TThit, TTwr, TTowr);

	//printf("score: %d d%d c%d moves%d", score, n, color, count);
	//printmove( fst_pick);
}
else
if (strstr(w,"pvs02") != NULL)
{
	scanf("%d", &n);
//                printNboard(Ncb);
	color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
	//printBits( 8, &cb.info);
	
	count_nTT= setnTT( memory);

	int i;
	//marray = malloc( sizeof(move)*216*(n+1) );
	for (i = 1; i <= n; i++)
	{
		count = 0;
		TThit = 0;
		TTowr = 0;
		TTwr = 0;
	     
		gettimeofday(&start, NULL);
		score = pvs_02( &Ncb, &Npline, -WIN -300, +WIN +300, color, i, 0);
		//memcpy( &PV, &Npline, sizeof(Nline));
		gettimeofday(&end, NULL);

		printf("pvs02:%d        ", i);
		nTTextractPV( Ncb, i);
		
		
		razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
		fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu, hits=%d  writes %d overwrites %d\n", score, razmisljao, count/razmisljao,  count, TThit, TTwr, TTowr);
      }
	//free( marray);

	//printf("score: %d d%d c%d moves%d", score, n, color, count);
	//printmove( fst_pick);
}
else
if (strstr(w,"quesc") != NULL)
{
	scanf("%d", &n);
//                printNboard(Ncb);
	color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
	//printBits( 8, &cb.info);
	count = 0;

	gettimeofday(&start, NULL);
	int i;
	//marray = malloc( sizeof(move)*216*(n+1) );
	for (i = 1; i <= n; i++)
	{
		score = Quiesce( &Ncb, &Npline, -WIN, +WIN, color, i, 0);
		memcpy( &PV, &Npline, sizeof(Nline));
		printf("quiesce: %d 	", i);

	//free( marray);
		gettimeofday(&end, NULL);

		printNline( Npline);
		razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
		fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu\n", score, razmisljao, count/razmisljao,  count);

	}
	//printf("score: %d d%d c%d moves%d", score, n, color, count);
	//printmove( fst_pick);
}
else
if (strstr(w,"pvs01") != NULL)
{
	scanf("%d", &n);
//                printNboard(Ncb);
	color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
	//printBits( 8, &cb.info);
	count = 0;

	gettimeofday(&start, NULL);
	int i;
	//marray = malloc( sizeof(move)*216*(n+1) );
	for (i = 1; i <= n; i++)
	{
		score = pvs_01( &Ncb, &Npline, -WIN, +WIN, color, i, 0, 0);
		memcpy( &PV, &Npline, sizeof(Nline));
		printf("pvs01:%d 	", i);

	//free( marray);
		gettimeofday(&end, NULL);

		printNline( Npline);
		razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
		fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu\n", score, razmisljao, count/razmisljao,  count);

	}
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
	score = nnegamax( &Ncb, &Npline, -WIN-300, +WIN+300, color, n, 0);
	//free( marray);
	gettimeofday(&end, NULL);	
	
	printNline( Npline);
	razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
	fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu\n", score, razmisljao, count/razmisljao,  count); 

	//printf("score: %d d%d c%d moves%d", score, n, color, count);
	//printmove( fst_pick);
}
else
if (strstr(w,"ntestsearch") != NULL)
{
	scanf("%d", &n);
	printNboard(Ncb);
	color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
	//printBits( 8, &cb.info);
	count = 0;

	gettimeofday(&start, NULL);
	//marray = malloc( sizeof(move)*216*(n+1) );
	score = ntestnegamax( &Ncb, &Npline, -WIN, +WIN, color, n);
	//free( marray);
	gettimeofday(&end, NULL);

	printNline( Npline);
	razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
	fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu\n", score, razmisljao, count/razmisljao,  count);

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
	score = anegamax( &cb, &pline, -WIN, +WIN, color, n);
	free( marray);
	gettimeofday(&end, NULL);	
	
	printline( pline);
	razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
	fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu\n", score, razmisljao, count/razmisljao,  count); 

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
	fprintf(stderr, "time=%.2f v=%.3e c=%llu\n", razmisljao, count/razmisljao,  count); 

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
	fprintf(stderr, "time=%.2f v=%.3e c=%llu\n", razmisljao, count/razmisljao,  count); 
	
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
 		fprintf(stderr, "time=%.2f v=%.3e c=%llu\n", razmisljao, count/razmisljao,  count); 
	
	}
        else
        if ( strcmp(w,"sortmoves") == 0 )
        {
                //objtoarr( &cb, &Ncb);
                NML = malloc( sizeof(Nmovelist)*(n+1) );
                score = generate_movesN(NML, Ncb);
		sortmoves(NML, 0);
                capt = NML->captcount;
                //printf("Bcapt : %d    \n", capt);
                int limes;
                it = (capt > 218) ? 218 : 0;
                limes = (capt > 218) ? capt : NML->quietcount;

                //printf("Acapt : %d    \n", capt);
sortnLegal_for:     for (; it < limes ; it++)
                {
                        printf("%d  ", it);
                        printmoveN( &NML->mdata[it]);
                        printf("\n");

                }

                if (it == (capt ))
                {
                        it = 0;
                        limes = NML->quietcount;
                        goto sortnLegal_for;
                }

                printf("quiet count: %d \n", NML->quietcount);

                /*for (n = 255; n > (NML->captcount); n--)     
                {
                        printf("%d  ", 255-n);
                        printmoveN( &NML->mdata[n]);
                        printf("\n");
                                
                }*/
                printf("capt count: %d  \n", NML->captcount-218);
                printf("moves count: %d \n", NML->quietcount + NML->captcount - 218);
                free( NML);

        }
	else
	if ( strcmp(w,"nLegal") == 0 )	
	{	
	        //objtoarr( &cb, &Ncb);

		score = generate_movesN(NML, Ncb);
		capt = NML->captcount;
        	//printf("Bcapt : %d	\n", capt);
		int limes;
	        it = (capt > 218) ? 218 : 0;
        	limes = (capt > 218) ? capt : NML->quietcount;

        	//printf("Acapt : %d	\n", capt);
nLegal_for:	for (; it < limes ; it++)
		{
        		printf("%d  ", it);
		        printmoveN( &NML->mdata[it]);
        		printf("\n");
				
        	}

	        if (it == (capt ))
        	{
                	it = 0;
	                limes = NML->quietcount;
        	        goto nLegal_for;
	        }
		
        	printf("quiet count: %d	\n", NML->quietcount);
        	
		/*for (n = 255; n > (NML->captcount); n--)     
		{
        		printf("%d  ", 255-n);
		        printmoveN( &NML->mdata[n]);
        		printf("\n");
				
        	}*/
        	printf("capt count: %d	\n", 255-NML->captcount);
        	printf("moves count: %d	\n", NML->quietcount + NML->captcount - 218);
		
	} 
        else
        if ( strcmp(w,"ntestLegal") == 0 )
        {
                //objtoarr( &cb, &Ncb);
                NML = malloc( sizeof(Nmovelist)*(n+1) );
                score = generate_movesN_test(NML, Ncb);
                capt = NML->captcount;
                //printf("Bcapt : %d    \n", capt);
                int limes;
                it = (capt > 218) ? 218 : 0;
                limes = (capt > 218) ? capt : NML->quietcount;

                //printf("Acapt : %d    \n", capt);
ntestLegal_for:     for (; it < limes ; it++)
                {
                        printf("%d  ", it);
                        printmoveN( &NML->mdata[it]);
                        printf("\n");

                }

                if (it == (capt ))
                {
                        it = 0;
                        limes = NML->quietcount;
                        goto ntestLegal_for;
                }

                printf("quiet count: %d \n", NML->quietcount);

                /*for (n = 255; n > (NML->captcount); n--)     
                {
                        printf("%d  ", 255-n);
                        printmoveN( &NML->mdata[n]);
                        printf("\n");
                                
                }*/
                printf("capt count: %d  \n", 255-NML->captcount);
                printf("moves count: %d \n", NML->quietcount + NML->captcount - 218);
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
		nsetZobrist( &Ncb);
	} 
	else 
	if ( strstr(w,"set") != NULL )	
	{	
		scanf("%[^\n]", FEN);
		cb = importFEN(FEN);		
		Ncb = NimportFEN(FEN);
//    print_state( Ncb);
		setZobrist( &cb);	
		nsetZobrist( &Ncb);

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
		nsetZobrist( &Ncb);
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

void catch_alrm()
{
	//printf("alarm ---- PONDERING now\n");
	en_state = PONDERING;
}

int chess_engine_communication_protocol()
{
	FILE *features=NULL;
	char *buff;
	size_t len = 0;
	ssize_t read;

	getline(&buff, &len, stdin);
	int protover=0;
	if (sscanf(buff,"protover %d", &protover))
		if (protover > 1)
			features = fopen("/home/edita/Documents/domibu_chess/CECP_features","r");
	//SEND XBOARD FEATURES
	if (features)
	{
	while ((read=getline(&buff, &len, features)) != -1) 
	{
		fprintf(stdout, "feature %s", buff);

	}
		fclose(features);
		free(buff);
	}
	else
	{
		perror("Couldn't open CECP_features.txt\n");
		return 1;
	}
	//SEND DONE
	fprintf(stdout, "feature done=1\n");
while (1)
{
	//printf("waiting input\n");
	////////////////////CHECK PING////////////////////////////////
	read=getline(&buff, &len, stdin);
	//printf("getline:%s\n", buff);
	//fflush(stdin);	
	if ( strstr(buff,"new") != NULL )
	{
		//	reset history	///////////
		history.curr = 0;
		search_depth = SD_MAX;
		//cb = importFEN(starting_position);
		//cb = importFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");	
		Ncb = NimportFEN("rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1");	
		//setZobrist( &cb);	
		nsetZobrist( &Ncb);
		en_state = PONDERING;
		////////associate einge's clock with black and the opponent's clock with white/////////
	}
	else
	if (strstr(buff, "force") != NULL)
	{
		////STOP CLOCKS//////
		its.it_value.tv_sec = 0;
		its.it_value.tv_nsec = 0;
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
		timer_settime(timer_id, 0, &its, NULL);

		en_state = OBSERVING;

	}
	else
	if (strstr(buff,"go") != NULL)
	{
		t_manager.fm = 1;
		en_state = THINKING;

en_state_THINKING:
		//fprintf(stderr, "receive go\n");
	        color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
                /*int i;
		long t;
		*/
 		//gettimeofday(&start, NULL);

		//TIME MANAGMENT ROUTINE
		if (t_manager.ctrl_style == 0 )
		{
			unsigned fm;
			//printBits(8, &Ncb.info);
			fm = t_manager.fm;//(Ncb.info >> 16) & 0x0000000007FF;
			//printf("fm %u\n", fm);
			//	TOURNAMENT TIME CTRL
			t_manager.target = t_manager.en_time / (t_manager.moves_per_ctrl - (((fm-1) % t_manager.moves_per_ctrl)));
		}
		else if (t_manager.ctrl_style == 1)
		{
			//	FISHER TIME CTRL
			t_manager.target = (t_manager.en_time + 100*t_manager.increment*t_manager.est_moves_game) / t_manager.est_moves_game;
			t_manager.target = (t_manager.en_time < t_manager.target) ? t_manager.en_time : t_manager.target;
		}
		its.it_value.tv_sec = (t_manager.target-t_manager.lag) / 100;
		its.it_value.tv_nsec = ((t_manager.target-t_manager.lag) % 100)*10000000;
		its.it_interval.tv_sec = 0;
		its.it_interval.tv_nsec = 0;
		timer_settime(timer_id, 0, &its, NULL);
		pthread_create(&threads[0], NULL, Thinking, NULL);

		//printf("t_man CTRL_STYLE %u mp_ctrl %u base %u inc %u estmpg %u\n", t_manager.ctrl_style, t_manager.moves_per_ctrl, t_manager.en_time, t_manager.increment, t_manager.est_moves_game);
		printf("target %u\n", t_manager.target);

		//sleep(time/1000);
		//en_state = PONDERING;	

		//nTTextractPV( Ncb, i);
	}
	else
	if (strstr(buff,"playother") != NULL)
	{
		t_manager.fm = 1;
		en_state = PONDERING;
	}
	else
	if (strstr(buff,"level") != NULL)
	{
		unsigned mps=0, inc=0, min=0, sec=0;
		char s[28];
		if (strstr(buff, ":") != NULL)	sscanf(buff, "level %u %u:%u %u", &mps, &min, &sec, &inc);
		else sscanf(buff, "level %u %u %u", &mps, &min, &inc);

		if (mps)
		{
			t_manager.ctrl_style = 0;
			t_manager.moves_per_ctrl = mps;
			t_manager.en_time = (min * 60 + sec)* 100;
		}
		if (inc)
		{
			t_manager.ctrl_style = 1;
			t_manager.en_time = (min*60 + sec) * 100;
			t_manager.increment = inc;
		}
		printf("t_man CTRL_STYLE %d mp_ctrl %d base %u inc %d\n", t_manager.ctrl_style, t_manager.moves_per_ctrl, t_manager.en_time, t_manager.increment);
	}
	else
	if (strstr(buff, "st ") != NULL)
	{
		unsigned in_st=0;

		sscanf(buff, "st %u", &in_st);
		if (in_st)
		{  
			t_manager.ctrl_style = 2;
			t_manager.target = in_st * 100;
		}
	}
	else 
	if (strstr(buff, "time") != NULL)
	{
		unsigned t;
		sscanf(buff, "time %u", &t);
		t_manager.en_time = t;
	}
	else
	if (strstr(buff, "otim") != NULL)
	{
		unsigned t;
		sscanf(buff, "otim %u", &t);
		t_manager.op_time = t;
	}
	else
	if (strstr(buff, "setboard ") != NULL)
	{
		char fen[127];
		sscanf(buff, "setboard %[^\n]", fen);
		
		printf("buff:	%s\n", buff);
		printf("FEN:	%s\n", fen);

		en_state = OBSERVING;
		
		//	reset history	///////////
		history.curr = 0;

		Ncb = NimportFEN(fen);
		nsetZobrist( &Ncb);

		//	set FM u t_manager	////////////////
		//t_manager.fm;

		printNboard(Ncb);	
		
	}
	else
	if (strstr(buff, "memory ") != NULL)
	{
		unsigned in_mem;
		//	CHECK MEMORY MAX LIMIT	//////////////////
		sscanf(buff, "memory %d", &in_mem);
		memory = (in_mem > MEMORY_MAX) ? MEMORY_MAX : in_mem;
	}
	else
	if (strstr(buff, "sd") != NULL)
	{
		unsigned in_sd;
		sscanf(buff, "sd %u", &in_sd);
		search_depth = (in_sd > SD_MAX) ? SD_MAX : in_sd; 
	}
	else
	if (strstr(buff, "nopost") != NULL)
	{
		post = 0;
	}
	else
	if (strstr(buff, "post") != NULL)
	{
		post = 1;
	}
	else
	if (strstr(buff, "result ") != NULL)
	{
		en_state = OBSERVING;
		//	TRIGER LEARNING	///////////
	}
	else
	if ((strstr(buff, "undo") != NULL) && (history.curr))
	{
		en_state = OBSERVING;

		t_manager.fm -= ((Ncb.info >> 14) & 1ULL) ? 1 : 0;
		history.curr--;

		Nmove undo_move = history.tag[history.curr] & 0x00000000FFFFFFFF;
		unsigned undo_data = history.tag[history.curr] >> 32;
		
		Nmovelist dummy_move_list;
		dummy_move_list.undo = undo_data;
	
		Nundo_move(&Ncb, &dummy_move_list, undo_move);
	}
	else
	if ((strstr(buff, "remove ") != NULL) && (history.curr > 1))
	{
		en_state = OBSERVING;

		t_manager.fm--;
		history.curr--;

		Nmove undo_move = history.tag[history.curr] & 0x00000000FFFFFFFF;
		unsigned undo_data = history.tag[history.curr] >> 32;
		
		Nmovelist dummy_move_list;
		dummy_move_list.undo = undo_data;
	
		Nundo_move(&Ncb, &dummy_move_list, undo_move);

		history.curr--;

		undo_move = history.tag[history.curr] & 0x00000000FFFFFFFF;
		undo_data = history.tag[history.curr] >> 32;
		
		dummy_move_list.undo = undo_data;
	
		Nundo_move(&Ncb, &dummy_move_list, undo_move);
	}
	else
	if ((strstr(buff,"usermove ") != NULL) &&  ((en_state == PONDERING) || (en_state == OBSERVING)))
	{
		char s[16];
		int a, f, r;
		Nmove src, dst;
		
		sscanf(buff, "usermove %s", s);
		//printf("%s\n", w);
		//printf("%s\n", s);
		//printf("citam\n");
		//fflush(stdout);

		//fgets(s, 256, stdin);
		//printf("%s", s);

		switch (s[0])
		{
			case 'a': f = 7; break;
                        case 'b': f = 6; break;
                        case 'c': f = 5; break;
                        case 'd': f = 4; break;
                        case 'e': f = 3; break;
                        case 'f': f = 2; break;
                        case 'g': f = 1; break;
                        case 'h': f = 0; break;
		}
		switch (s[1])
		{
			case '1': r = 0; break;
                        case '2': r = 1; break;
                        case '3': r = 2; break;
                        case '4': r = 3; break;
                        case '5': r = 4; break;
                        case '6': r = 5; break;
                        case '7': r = 6; break;
                        case '8': r = 7; break;
		}
                //printf("---------s01=%s src=%u dst=%u-----------", s, src, dst);
		src = f + r*8;
		
		switch (s[2])
		{
			case 'a': f = 7; break;
                        case 'b': f = 6; break;
                        case 'c': f = 5; break;
                        case 'd': f = 4; break;
                        case 'e': f = 3; break;
                        case 'f': f = 2; break;
                        case 'g': f = 1; break;
                        case 'h': f = 0; break;
		}
		switch (s[3])
		{
			case '1': r = 0; break;
                        case '2': r = 1; break;
                        case '3': r = 2; break;
                        case '4': r = 3; break;
                        case '5': r = 4; break;
                        case '6': r = 5; break;
                        case '7': r = 6; break;
                        case '8': r = 7; break;
		}
		dst = f + r*8;
		
		//printf("---------s23=%s src=%u dst=%u-----------", s, src, dst);
		//fflush(stdout);

		//NML = malloc( sizeof(Nmovelist)*(n+1) );
		score = generate_movesN(NML, Ncb);
		capt = NML->captcount;
        	//printf("Bcapt : %d	\n", capt);
		int limes;
	        it = (capt > 218) ? 218 : 0;
        	limes = (capt > 218) ? capt : NML->quietcount;
		//printf("-------move-stats---------------capt=%d quiet %d score %d\n", capt, NML->quietcount, score);
        	//printf("Acapt : %d	\n", capt);
sxn_Legal_fo02r:	for (; it < limes ; it++)
		{
			//printf("i:%d ", it);
			Nmove tmp_src = (NML->mdata[it] >> 6) & 0x000000000000003F;
			Nmove tmp_dst = (NML->mdata[it] >> 12) & 0x000000000000003F;

			//printmoveN(&NML->mdata[it]);
			//printf("??src=%u st=%u\n", tmp_src, tmp_dst);
			//fflush(stdout);

			if ((tmp_src == src) && (tmp_dst == dst))
			{		//**do_move**
				//printf("found_move\n");

				//printmoveN(&NML->mdata[it]);

				t_manager.fm += (color == -1) ? 0 : 1;

				//	update history	//////////
				history.tag[history.curr] ^= 0ULL;
				history.tag[history.curr] = Ncb.info << 32;
				history.tag[history.curr] = NML->mdata[it];
				history.curr++;
	
				Ndo_move(&Ncb, NML->mdata[it]);
				if (en_state == PONDERING)
				{
					en_state = THINKING;
					goto en_state_THINKING;
				}
				else goto end_user_move;
			}
        	}

	        if (it == (capt ))
        	{
                	it = 0;
	                limes = NML->quietcount;
        	        goto sxn_Legal_fo02r;
	        }
		////////////////////////////// REPORT ILLEGALMOVE        ///////////////////
		printf("Illegal move: %s\n", s);
end_user_move: ;		
	}
	else if ( strstr(buff,"nLegal") != NULL )	
	{	
	        //objtoarr( &cb, &Ncb);
		NML = malloc( sizeof(Nmovelist)*(n+1) );
		score = generate_movesN(NML, Ncb);
		capt = NML->captcount;
        	//printf("Bcapt : %d	\n", capt);
		int limes;
	        it = (capt > 218) ? 218 : 0;
        	limes = (capt > 218) ? capt : NML->quietcount;

        	//printf("Acapt : %d	\n", capt);
xn_Legal_for:	for (; it < limes ; it++)
		{
        		printf("%d  ", it);
		        printmoveN( &NML->mdata[it]);
        		printf("\n");
				
        	}

	        if (it == (capt ))
        	{
                	it = 0;
	                limes = NML->quietcount;
        	        goto xn_Legal_for;
	        }
		
        	printf("quiet count: %d	\n", NML->quietcount);
        	
		/*for (n = 255; n > (NML->captcount); n--)     
		{
        		printf("%d  ", 255-n);
		        printmoveN( &NML->mdata[n]);
        		printf("\n");
				
        	}*/
	}
	else if (strstr(buff, "quit") != NULL)
	{
		printf("quiting :(\n");
		break;
	}
	
}

return 1;

}
