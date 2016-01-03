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
// old board_1, move_1, TT structures, for backtesting
#include "MoveGeneration-1.h"
#include "Search-1.h"
#include "TranspositionTable-1.h"


extern move_1 *marray;
extern line_1 pline;
extern Nline Npline;
extern int TThit;

U64 count = 0, mfree = 0;
double razmisljao;
struct timeval start, end;

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MEMORY_MAX 2048
int n=200, memory = 64, score, color;
#define SD_MAX 50
int search_depth = SD_MAX;
int post=1;
extern int stop;
int pondering=0;
int ping=0;
int computer=0, cores=0, egtpath=0, option=0;

int input_max_length = 512;
board_1 cb;
board Ncb;

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
	move pm;
	//stderr
	printf("mem %d\n", memory);

	count_nTT = setnTT( memory);

	for (i = 1; (en_state == THINKING) && (i <= search_depth); i++)
	{
		count = 0;

		score = search( &Ncb, &Npline, -WIN -300, +WIN +300, color, i, 0);

		timer_gettime(timer_id, &curr_tick);
		double search_time = its.it_value.tv_sec + (double)its.it_value.tv_nsec/1000000000;
		search_time -= curr_tick.it_value.tv_sec + (double)curr_tick.it_value.tv_nsec/1000000000;

		if (en_state == THINKING)
			if (post)
			{
				/////PRINTING THINKING OUTPUT/////////////
				fprintf(stdout, "%d	%d	%.2f	%llu	", i, score, search_time*100, count);
				pm = nTTextractPV( Ncb, i);
			}
			else	pm = TTfind_move( Ncb.zobrist); 

		if (((score  <= -WIN) || (score >= WIN)) && (en_state == THINKING))
		{
			en_state = PONDERING;
		}
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
		Ndo_move( &Ncb, pm);
		printf("move ");
		printmoveN(&pm);
		printf("\n");
	}
	freeTT();
	pthread_exit(NULL);
}

unsigned char capt, it;
move_1 *fst_pick, *list, *domove = NULL;
move *temp;
unsigned char ln[10], d = 0;
char w[512]; 

int main ( int argc, char *argv[])
{
	setbuf(stdout, NULL);
	setbuf(stdin, NULL);

	sigemptyset (&block_mask);
	/* Block other terminal-generated signals while handler runs. */
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

	Ncb = NimportFEN(START_FEN);
	nsetZobrist( &Ncb);
	NML = malloc( sizeof(Nmovelist)*search_depth);
	en_state = WAITING;

	fgets(w, input_max_length, stdin);

	if (strstr(w,"xboard") != NULL)	chess_engine_communication_protocol();
	else	chess_engine_testing( argc, argv);
	return 0;
}

void catch_alrm()
{
	//stderr ----- printf("alarm ---- PONDERING now\n");
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
			features = fopen("CECP_features","r");
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
		////////////////////CHECK PING////////////////////////////////
		read=getline(&buff, &len, stdin);

		if ( strstr(buff,"new") != NULL )
		{
			//	reset history	///////////
			history.curr = 0;
			search_depth = SD_MAX;
			Ncb = NimportFEN(START_FEN);	
			nsetZobrist( &Ncb);
			en_state = PONDERING;
			////////associate einge's clock with black and the opponent's clock with white/////////
		}
		else if (strstr(buff, "force") != NULL)
		{
			////STOP CLOCKS//////
			its.it_value.tv_sec = 0;
			its.it_value.tv_nsec = 0;
			its.it_interval.tv_sec = 0;
			its.it_interval.tv_nsec = 0;
			timer_settime(timer_id, 0, &its, NULL);

			en_state = OBSERVING;
		}
		else if (strstr(buff,"go") != NULL)
		{
			t_manager.fm = 1;
			en_state = THINKING;

en_state_THINKING:
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			//TIME MANAGMENT ROUTINE
			if (t_manager.ctrl_style == 0 )
			{
				//	TOURNAMENT TIME CTRL
				unsigned fm;
				fm = t_manager.fm;
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

			printf("target %u\n", t_manager.target);
		}
		else if (strstr(buff,"playother") != NULL)
		{
			t_manager.fm = 1;
			en_state = PONDERING;
		}
		else if (strstr(buff,"level") != NULL)
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
		else if (strstr(buff, "st ") != NULL)
		{
			unsigned in_st=0;

			sscanf(buff, "st %u", &in_st);
			if (in_st)
			{  
				t_manager.ctrl_style = 2;
				t_manager.target = in_st * 100;
			}
		}
		else if (strstr(buff, "time") != NULL)
		{
			unsigned t;
			sscanf(buff, "time %u", &t);
			t_manager.en_time = t;
		}
		else if (strstr(buff, "otim") != NULL)
		{
			unsigned t;
			sscanf(buff, "otim %u", &t);
			t_manager.op_time = t;
		}
		else if (strstr(buff, "setboard_1 ") != NULL)
		{
			char fen[127];
			sscanf(buff, "setboard_1 %[^\n]", fen);

			//stderr
			printf("buff:	%s\n", buff);
			printf("FEN:	%s\n", fen);

			en_state = OBSERVING;
			//	reset history	///////////
			history.curr = 0;

			Ncb = NimportFEN(fen);
			nsetZobrist( &Ncb);

			printboard(Ncb);	
		}
		else if (strstr(buff, "memory ") != NULL)
		{
			unsigned in_mem;
			//	CHECK MEMORY MAX LIMIT	//////////////////
			sscanf(buff, "memory %d", &in_mem);
			memory = (in_mem > MEMORY_MAX) ? MEMORY_MAX : in_mem;
		}
		else if (strstr(buff, "sd") != NULL)
		{
			unsigned in_sd;
			sscanf(buff, "sd %u", &in_sd);
			search_depth = (in_sd > SD_MAX) ? SD_MAX : in_sd; 
		}
		else if (strstr(buff, "nopost") != NULL)
		{
			post = 0;
		}
		else if (strstr(buff, "post") != NULL)
		{
			post = 1;
		}
		else if (strstr(buff, "result ") != NULL)
		{
			en_state = OBSERVING;
			//stderr	TRIGER LEARNING	///////////
		}
		else if ((strstr(buff, "undo") != NULL) && (history.curr))
		{
			en_state = OBSERVING;

			t_manager.fm -= ((Ncb.info >> 14) & 1ULL) ? 1 : 0;
			history.curr--;

			move undo_move = history.tag[history.curr] & 0x00000000FFFFFFFF;
			unsigned undo_data = history.tag[history.curr] >> 32;

			Nmovelist dummy_move_list;
			dummy_move_list.undo = undo_data;

			Nundo_move(&Ncb, &dummy_move_list, undo_move);
		}
		else if ((strstr(buff, "remove ") != NULL) && (history.curr > 1))
		{
			en_state = OBSERVING;

			t_manager.fm--;
			history.curr--;

			move undo_move = history.tag[history.curr] & 0x00000000FFFFFFFF;
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
		else if ((strstr(buff,"usermove ") != NULL) &&  ((en_state == PONDERING) || (en_state == OBSERVING)))
		{
			char s[16];
			int a, f, r;
			move src, dst;

			sscanf(buff, "usermove %s", s);

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

			score = generate_movesN(NML, Ncb);
			capt = NML->captcount;
			int limes;
			it = (capt > 218) ? 218 : 0;
			limes = (capt > 218) ? capt : NML->quietcount;
sxn_Legal_fo02r:	for (; it < limes ; it++)
			{
				move tmp_src = (NML->mdata[it] >> 6) & 0x000000000000003F;
				move tmp_dst = (NML->mdata[it] >> 12) & 0x000000000000003F;

				if ((tmp_src == src) && (tmp_dst == dst))
				{
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
			NML = malloc( sizeof(Nmovelist)*(n+1) );
			score = generate_movesN(NML, Ncb);
			capt = NML->captcount;
			int limes;
			it = (capt > 218) ? 218 : 0;
			limes = (capt > 218) ? capt : NML->quietcount;

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
		}
		else if (strstr(buff, "quit") != NULL)
		{
			printf("quiting :(\n");
			break;
		}

	}
	return 1;
}

int chess_engine_testing(int argc, char *argv)
{
	count_nTT = setnTT( memory);

	while (1)
	{

		scanf("%s",w);

		if (strstr(w, "resetnTT") != NULL)
		{
			freeTT();
			count_nTT= setnTT( memory);
		}
		else if (strstr(w,"pvs03") != NULL)
		{
			scanf("%d", &n);
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;
			TThit = 0;
			TTowr = 0;
			TTwr = 0;

			gettimeofday(&start, NULL);
			score = search( &Ncb, &Npline, -WIN -300, +WIN +300, color, n, 0);
			gettimeofday(&end, NULL);

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			printf( "~~pvs03   	%d	%.2f	%llu	", n, razmisljao, count); 
			nTTextractPV( Ncb, n);
		}
		else if (strstr(w,"pvs02") != NULL)
		{
			scanf("%d", &n);
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;
			TThit = 0;
			TTowr = 0;
			TTwr = 0;

			gettimeofday(&start, NULL);
			score = pvs_02( &Ncb, &Npline, -WIN -300, +WIN +300, color, n, 0);
			gettimeofday(&end, NULL);

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			printf( "~~pvs02   	%d	%.2f	%llu	", n, razmisljao, count); 
			nTTextractPV( Ncb, n);
		}
		else if (strstr(w,"nTT") != NULL)
		{
			scanf("%d", &n);
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;
			TThit = 0;
			TTowr = 0;
			TTwr = 0;

			gettimeofday(&start, NULL);
			score = nTTnegamax( &Ncb, &Npline, -WIN, +WIN, color, n);
			gettimeofday(&end, NULL);

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			printf( "~~nTT    	%d	%.2f	%llu	", n, razmisljao, count); 

			nTTextractPV( Ncb, n);
		}
		else if (strstr(w,"pvs01") != NULL)
		{
			scanf("%d", &n);
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;

			gettimeofday(&start, NULL);

			score = pvs_01( &Ncb, &Npline, -WIN, +WIN, color, n, 0, 0);

			gettimeofday(&end, NULL);

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			printf( "~~pvs01   	%d	%.2f	%llu	", n, razmisljao, count); 
			printNline( Npline);
		}

		else if (strstr(w,"nsearch") != NULL) 
		{	
			scanf("%d", &n);
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;

			gettimeofday(&start, NULL);	
			score = nnegamax( &Ncb, &Npline, -WIN-300, +WIN+300, color, n, 0);
			gettimeofday(&end, NULL);	

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			printf( "~~nsearch	%d	%.2f	%llu	", n, razmisljao, count); 

			printNline( Npline);
		}
		else if (strstr(w,"ndivide") != NULL) 
		{	
			scanf("%d", &n);
			gettimeofday(&start, NULL);
			count = Ndivide_perft(n, &Ncb, NML);
			gettimeofday(&end, NULL);
			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			printf( "~~ndivide	%d	%.2f	%llu\n", n, razmisljao, count); 
		}
		else if (strstr(w,"mTT") != NULL) 
		{	
			scanf("%d", &n);
			printboard_1(cb);		
			color = -1 + ((cb.info & 1ULL) << 1 );
			count = 0;
			TThit = 0;

			gettimeofday(&start, NULL);	
			score = mTTnegamax( &cb, &pline, -WIN, +WIN, color, n);
			gettimeofday(&end, NULL);	

			TTextractPV( cb, n);
			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu, hits=%d\n", score, razmisljao, count/razmisljao,  count, TThit); 
		}
		else if (strstr(w,"msearch") != NULL) 
		{	
			scanf("%d", &n);
			printboard_1(cb);		
			color = -1 + ((cb.info & 1ULL) << 1 );
			count = 0;

			gettimeofday(&start, NULL);	
			score = mnegamax( &cb, &pline, -WIN, +WIN, color, n);
			gettimeofday(&end, NULL);	

			printline( pline);
			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu\n", score, razmisljao, count/razmisljao,  count); 
		}
		else if (strstr(w,"aTT") != NULL) 
		{	
			scanf("%d", &n);
			printboard_1(cb);		
			color = -1 + ((cb.info & 1ULL) << 1 );
			count = 0;
			TThit = 0;
			TTwr = 0;

			gettimeofday(&start, NULL);	
			marray = malloc( sizeof(move_1)*216*(n+1) );
			score = aTTnegamax( &cb, &pline, -WIN, +WIN, color, n);
			free( marray);
			gettimeofday(&end, NULL);	

			TTextractPV( cb, n);
			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu, hits=%d\n", score, razmisljao, count/razmisljao,  count, TThit); 
		}
		else if (strstr(w,"quesc") != NULL)
		{
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;

			int i;
			printf("quesctest\n");
			en_state = THINKING;
			score = Quiesce( &Ncb, &Npline, -WIN, +WIN, color, 0, 0);
			en_state = OBSERVING;
			printf("q_result:	%d", score);

			printNline( Npline);
			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu\n", score, razmisljao, count/razmisljao,  count);
			printf("\n");
		}
		else if (strstr(w,"ntestsearch") != NULL)
		{
			scanf("%d", &n);
			printboard(Ncb);
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;

			gettimeofday(&start, NULL);
			score = ntestnegamax( &Ncb, &Npline, -WIN, +WIN, color, n);
			gettimeofday(&end, NULL);

			printNline( Npline);
			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu\n", score, razmisljao, count/razmisljao,  count);
		}
		else if (strstr(w,"asearch") != NULL) 
		{	
			scanf("%d", &n);
			printboard_1(cb);		
			color = -1 + ((cb.info & 1ULL) << 1 );
			count = 0;

			gettimeofday(&start, NULL);	
			marray = malloc( sizeof(move_1)*216*(n+1) );
			score = anegamax( &cb, &pline, -WIN, +WIN, color, n);
			free( marray);
			gettimeofday(&end, NULL);	

			printline( pline);
			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			fprintf(stderr, "==%d  time=%.2f v=%.3e c=%llu\n", score, razmisljao, count/razmisljao,  count); 
		}
		else if (strstr(w,"mdivide") != NULL) 
		{	
			scanf("%d", &n);
			gettimeofday(&start, NULL);	
			count = mdivide_perft(n, &cb);
			gettimeofday(&end, NULL);
			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			fprintf(stderr, "time=%.2f v=%.3e c=%llu\n", razmisljao, count/razmisljao,  count); 
		}
		else if (strstr(w,"adivide") != NULL) 
		{	
			scanf("%d", &n);
			gettimeofday(&start, NULL);
			marray = malloc( sizeof(move_1)*216*(n+1) );
			count = divide_perft(n, &cb);
			free( marray);
			gettimeofday(&end, NULL);
			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			fprintf(stderr, "time=%.2f v=%.3e c=%llu\n", razmisljao, count/razmisljao,  count); 
		}
		else if ( strcmp(w,"sortmoves") == 0 )
		{
			NML = malloc( sizeof(Nmovelist)*(n+1) );
			score = generate_movesN(NML, Ncb);
			sortmoves(NML, 0);
			capt = NML->captcount;
			int limes;
			it = (capt > 218) ? 218 : 0;
			limes = (capt > 218) ? capt : NML->quietcount;

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
		    printf("capt count: %d  \n", NML->captcount-218);
		    printf("moves count: %d \n", NML->quietcount + NML->captcount - 218);
		    free( NML);
		}
		else if ( strcmp(w,"nLegal") == 0 )	
		{	
			score = generate_movesN(NML, Ncb);
			capt = NML->captcount;
			int limes;
			it = (capt > 218) ? 218 : 0;
			limes = (capt > 218) ? capt : NML->quietcount;

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
		printf("capt count: %d	\n", 255-NML->captcount);
		printf("moves count: %d	\n", NML->quietcount + NML->captcount - 218);
		} 
		else if ( strcmp(w,"ntestLegal") == 0 )
		{
			NML = malloc( sizeof(Nmovelist)*(n+1) );
			score = generate_movesN_test(NML, Ncb);
			capt = NML->captcount;
			int limes;
			it = (capt > 218) ? 218 : 0;
			limes = (capt > 218) ? capt : NML->quietcount;

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
		    printf("capt count: %d  \n", 255-NML->captcount);
		    printf("moves count: %d \n", NML->quietcount + NML->captcount - 218);
		    free( NML);
		}
		else if ( strcmp(w,"legal") == 0 )	
		{	
			marray = malloc( sizeof(move_1)*256);
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
		else if ( strstr(w,"set") != NULL )	
		{	
			char FEN[512];
			scanf("%[^\n]", FEN);
			cb = importFEN(FEN);		
			Ncb = NimportFEN(FEN);
			setZobrist( &cb);	
			nsetZobrist( &Ncb);

			printboard(Ncb);	
		} 
		else if ( strstr(w,"do_move") != NULL )	
		{	
			scanf("%d", &n);
			ln[d] = n;
			score = generate_movesN(&NML[d], Ncb);

			printf("m\n");
			printBits(8, &NML[d].mdata[n]);
			Ndo_move( &Ncb, NML[d].mdata[n]);
			printboard(Ncb);
			d++;		

		}
		else if ( strstr(w,"undomove") != NULL )
		{
			d--;
			Nundo_move( &Ncb, &NML[d], NML[d].mdata[ln[d]]);
			printboard(Ncb);		

		}
		else if ( strstr(w,"state") != NULL )
		{
			print_state( Ncb);
		}
		else if ( strstr(w,"Ndisp") != NULL )
		{
			printboard(Ncb);	
		}
		else if ( strstr(w,"disp") != NULL )
		{
			printboard_1(cb);		
		}
		else if ( strstr(w,"change_stm") != NULL )
		{
			cb.info ^= 1LL;
			Ncb.info ^= 1LL << 14;
			setZobrist( &cb);
			printboard_1(cb);		
		}
		else if ( strstr(w,"new") != NULL )
		{
			cb = importFEN(START_FEN);	
			Ncb = NimportFEN(START_FEN);	
			setZobrist( &cb);	
			nsetZobrist( &Ncb);
		}
		else if ( strstr(w,"quit") != NULL )
		{
			break;	
		}
	}	
}
