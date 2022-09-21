#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <signal.h>
#include <time.h>
#include "BitBoardMagic.h"
#include "Interface.h"

#include "MoveGeneration.h"
#include "Evaluation.h"
#include "Search.h"
#include "TranspositionTable.h"
// old board_1, move_1, TT structures, for backtesting
#include "MoveGeneration-1.h"
#include "Search-1.h"
#include "TranspositionTable-1.h"


extern move_1 *marray;
extern line_1 pline;
extern line Npline;
extern int TThit;

U64 count = 0, mfree = 0;
double razmisljao;
struct timeval start, end;

#define START_FEN "rnbqkbnr/pppppppp/8/8/8/8/PPPPPPPP/RNBQKBNR w KQkq - 0 1"
#define MEMORY_MAX 2048
int n=200, memory = 64, score, color;
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

	TTentry_count = set_TT( memory);

	for (i = 1; (en_state == THINKING) && (i <= search_depth); i++)
	{
		count = 0;

		score = search( &Ncb, &Npline, -WIN, +WIN, color, i, 0);

		timer_gettime(timer_id, &curr_tick);
		double search_time = its.it_value.tv_sec + (double)its.it_value.tv_nsec/1000000000;
		search_time -= curr_tick.it_value.tv_sec + (double)curr_tick.it_value.tv_nsec/1000000000;

		if (en_state == THINKING)
		{
			if (post)
			{
				/////PRINTING THINKING OUTPUT/////////////
				Print(1, "%2d %7d %6.f %12llu %6.f	%s", i, score, search_time*100, count, count/(search_time*100), print_TT_PV( Ncb, i));
			}
			pm = TTfind_move( Ncb.zobrist);
		}
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
		do_move( &Ncb, pm);
		Print(1, "move %s\n", print_smith_notation(&pm));
	}
	free_TT();
	pthread_exit(NULL);
}

unsigned char capt, it;
move_1 *fst_pick, *list, *domove = NULL;
move *temp;
unsigned char ln[10], d = 0;
char buff[2048];

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

	if (CreateLogDirectory(LOG_RELATIVE_PATH) < 0)
	{
		return -1;
	}

	sprintf(buff, "./%s/log.%03d", LOG_RELATIVE_PATH, InitializeLogId(LOG_RELATIVE_PATH));
	log_file = fopen(buff, "w");
	if (log_file == NULL)
	{
		perror("log file not open ");
		return -1;
	}

	InitializeMoveDatabase();
	initZobrist();
	init_psqt();

	Ncb = NimportFEN(START_FEN);
	set_zobrist_keys( &Ncb);
	NML = malloc( sizeof(node_move_list)*search_depth);
	en_state = WAITING;

	fgets(buff, input_max_length, stdin);

	if (strstr(buff,"xboard") != NULL)	chess_engine_communication_protocol();
	else	chess_engine_testing( argc, argv);
	return 0;
}

void catch_alrm()
{
	Print(0, "catch_alarm ---- PONDERING now\n");
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
			Print(1, "feature %s", buff);
		}
		fclose(features);
		free(buff);
	}
	else
	{
		Print(1, "Couldn't open CECP_features file, quiting\n");
		return 1;
	}
	//SEND DONE
	Print(1, "feature done=1\n");

	while (1)
	{
		////////////////////CHECK PING////////////////////////////////
		read=getline(&buff, &len, stdin);
		Print(2, "%s", buff);

		if ( strstr(buff,"new") != NULL )
		{
			//	reset history	///////////
			history.curr = 0;
			search_depth = SD_MAX;
			Ncb = NimportFEN(START_FEN);
			set_zobrist_keys( &Ncb);
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
			Print(0, "CTRL_STYLE %d mp_ctrl %d base %u inc %d\n", t_manager.ctrl_style, t_manager.moves_per_ctrl, t_manager.en_time, t_manager.increment);
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
		else if (strstr(buff, "setboard") != NULL)
		{
			char fen[127];
			sscanf(buff, "setboard %[^\n]", fen);

			en_state = OBSERVING;
			//	reset history	///////////
			history.curr = 0;

			Ncb = NimportFEN(fen);
			set_zobrist_keys( &Ncb);

			Print(0, "%s", printboard(Ncb));
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

			move undo_move_1 = history.tag[history.curr] & 0x00000000FFFFFFFF;
			unsigned undo_data = history.tag[history.curr] >> 32;

			node_move_list dummy_move_list;
			dummy_move_list.undo = undo_data;

			undo_move(&Ncb, &dummy_move_list, undo_move_1);
		}
		else if ((strstr(buff, "remove ") != NULL) && (history.curr > 1))
		{
			en_state = OBSERVING;

			t_manager.fm--;
			history.curr--;

			move undo_move_1 = history.tag[history.curr] & 0x00000000FFFFFFFF;
			unsigned undo_data = history.tag[history.curr] >> 32;

			node_move_list dummy_move_list;
			dummy_move_list.undo = undo_data;

			undo_move(&Ncb, &dummy_move_list, undo_move_1);

			history.curr--;

			undo_move_1 = history.tag[history.curr] & 0x00000000FFFFFFFF;
			undo_data = history.tag[history.curr] >> 32;

			dummy_move_list.undo = undo_data;

			undo_move(&Ncb, &dummy_move_list, undo_move_1);
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

			score = generate_moves(NML, Ncb);
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

					do_move(&Ncb, NML->mdata[it]);
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
			Print(1, "Illegal move ^& %s\n", s);
end_user_move: ;
		}
		else if ( strstr(buff,"nLegal") != NULL )
		{
			score = generate_moves(NML, Ncb);
			capt = NML->captcount;
			int limes;
			it = (capt > 218) ? 218 : 0;
			limes = (capt > 218) ? capt : NML->quietcount;

xn_Legal_for:
			for (; it < limes ; it++)
			{
				Print(1, "%d  %s\n", it, print_smith_notation( &NML->mdata[it]));
			}

			if (it == (capt ))
			{
				it = 0;
				limes = NML->quietcount;
				goto xn_Legal_for;
			}

			Print(1, "quiet count: %d\n", NML->quietcount);
		}
		else if (strstr(buff, "quit") != NULL)
		{
			Print(1, "quiting :(\n");
			break;
		}

	}
	return 1;
}

int chess_engine_testing(int argc, char *argv)
{
	char *buff;
	size_t len = 0;
	ssize_t read;

	TTentry_count = set_TT( memory);

	while (1)
	{
		read=getline(&buff, &len, stdin);
		Print(2, "<%s", buff);

		if (strstr(buff, "resetnTT") != NULL)
		{
			free_TT();
			TTentry_count= set_TT( memory);
		}
		else if (strstr(buff,"pvs03") != NULL)
		{
			sscanf(buff, "pvs03 %d", &n);
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;
			TThit = 0;
			TTowr = 0;
			TTwr = 0;

			en_state = THINKING;
			gettimeofday(&start, NULL);
			score = search( &Ncb, &Npline, -WIN, +WIN, color, n, 0);
			gettimeofday(&end, NULL);
			en_state = OBSERVING;

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "%2d %7d %6.2fs %12llu %6.f	%s", n, score * color, razmisljao*100, count, count/(razmisljao*100), print_TT_PV( Ncb, n));
		}
		else if (strstr(buff,"pvs02") != NULL)
		{
			sscanf(buff, "pvs02 %d", &n);
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;
			TThit = 0;
			TTowr = 0;
			TTwr = 0;

			en_state = THINKING;
			gettimeofday(&start, NULL);
			score = pvs_02( &Ncb, &Npline, -WIN, +WIN, color, n, 0);
			gettimeofday(&end, NULL);
			en_state = OBSERVING;

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "%2d %7d %6.2f %12llu %s", n, score, razmisljao*100, count, print_TT_PV( Ncb, n));
		}
		else if (strstr(buff,"nTT") != NULL)
		{
			sscanf(buff, "nTT %d", &n);
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;
			TThit = 0;
			TTowr = 0;
			TTwr = 0;

			gettimeofday(&start, NULL);
			score = nTTnegamax( &Ncb, &Npline, -WIN, +WIN, color, n);
			gettimeofday(&end, NULL);

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "%2d %7d %6.2f %12llu %s", n, score, razmisljao*100, count, print_TT_PV( Ncb, n));
		}
		else if (strstr(buff,"pvs01") != NULL)
		{
			sscanf(buff, "pvs01 %d", &n);
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;

			gettimeofday(&start, NULL);

			score = pvs_01( &Ncb, &Npline, -WIN, +WIN, color, n, 0, 0);

			gettimeofday(&end, NULL);

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "%2d %7d %6.2f %12llu %s", n, score, razmisljao*100, count, print_line_Smith_notation( Npline));
		}

		else if (strstr(buff,"nsearch") != NULL)
		{
			sscanf(buff, "nsearch %d", &n);
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;

			gettimeofday(&start, NULL);
			score = nnegamax( &Ncb, &Npline, -WIN-300, +WIN+300, color, n, 0);
			gettimeofday(&end, NULL);

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "%2d %7d %6.2f %12llu %s", n, score, razmisljao*100, count, print_line_Smith_notation( Npline));
		}
		else if (strstr(buff,"ndivide") != NULL)
		{
			sscanf(buff, "ndivide %d", &n);
			gettimeofday(&start, NULL);
			count = Ndivide_perft(n, &Ncb, NML);
			gettimeofday(&end, NULL);
			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "%2d %6.2f %12llu\n", n, razmisljao, count);
		}
		else if (strstr(buff,"mTT") != NULL)
		{
			sscanf(buff, "mTT %d", &n);
			color = -1 + ((cb.info & 1ULL) << 1 );
			count = 0;
			TThit = 0;

			gettimeofday(&start, NULL);
			score = mTTnegamax( &cb, &pline, -WIN, +WIN, color, n);
			gettimeofday(&end, NULL);

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "%2d %7d %6.2f %12llu %s", n, score, razmisljao*100, count, TTextractPV_1( cb, n));
		}
		else if (strstr(buff,"msearch") != NULL)
		{
			sscanf(buff, "msearch %d", &n);
			color = -1 + ((cb.info & 1ULL) << 1 );
			count = 0;

			gettimeofday(&start, NULL);
			score = mnegamax( &cb, &pline, -WIN, +WIN, color, n);
			gettimeofday(&end, NULL);

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "%2d %7d %6.2f %12llu %s", n, score, razmisljao*100, count, printline_1( pline));
		}
		else if (strstr(buff,"aTT") != NULL)
		{
			sscanf(buff, "aTT %d", &n);
			color = -1 + ((cb.info & 1ULL) << 1 );
			count = 0;
			TThit = 0;
			TTwr = 0;

			gettimeofday(&start, NULL);
			marray = malloc( sizeof(move_1)*216*(n+1) );
			score = aTTnegamax( &cb, &pline, -WIN, +WIN, color, n);
			free( marray);
			gettimeofday(&end, NULL);

			TTextractPV_1( cb, n);
			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "%2d %7d %6.2f %12llu %s", n, score, razmisljao*100, count, TTextractPV_1( cb, n));
		}
		else if (strstr(buff,"quesc") != NULL)
		{
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;

			int i;
			en_state = THINKING;
			score = Quiesce( &Ncb, &Npline, -WIN, +WIN, color, 0, 0);
			en_state = OBSERVING;

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "%2d %7d %6.2f %12llu\n", n, color*score, razmisljao*100, count);
		}
		else if (strstr(buff,"ntestsearch") != NULL)
		{
			sscanf(buff, "ntestsearch %d", &n);
			Print(0, "%s", printboard(Ncb));
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;

			gettimeofday(&start, NULL);
			score = ntestnegamax( &Ncb, &Npline, -WIN, +WIN, color, n);
			gettimeofday(&end, NULL);

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "%2d %7d %6.2f %12llu %s", n, score, razmisljao*100, count, print_line_Smith_notation( Npline));
		}
		else if (strstr(buff,"asearch") != NULL)
		{
			sscanf(buff, "asearch %d", &n);
			color = -1 + ((cb.info & 1ULL) << 1 );
			count = 0;

			gettimeofday(&start, NULL);
			marray = malloc( sizeof(move_1)*216*(n+1) );
			score = anegamax( &cb, &pline, -WIN, +WIN, color, n);
			free( marray);
			gettimeofday(&end, NULL);

			printline_1( pline);
			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "%2d %7d %6.2f %12llu %s", n, score, razmisljao*100, count, printline_1( pline));
		}
		else if (strstr(buff,"evaluate") != NULL)
		{
			color = -1 + (((Ncb.info >> 14) & 1ULL) << 1 );
			count = 0;

			board_1 b;
			int i;

			en_state = THINKING;
			score = evaluate(Ncb, 0, color, &Ncb);
			en_state = OBSERVING;

			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "score = %7d\n", color * score);
		}		
		else if (strstr(buff,"mdivide") != NULL)
		{
			sscanf(buff, "mdivide %d", &n);
			gettimeofday(&start, NULL);
			count = mdivide_perft(n, &cb);
			gettimeofday(&end, NULL);
			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "%2d %6.2f %12llu", n, razmisljao*100, count);
		}
		else if (strstr(buff,"adivide") != NULL)
		{
			sscanf(buff, "adivide %d", &n);
			gettimeofday(&start, NULL);
			marray = malloc( sizeof(move_1)*216*(n+1) );
			count = divide_perft(n, &cb);
			free( marray);
			gettimeofday(&end, NULL);
			razmisljao = (end.tv_sec - start.tv_sec) + (end.tv_usec - start.tv_usec) / 1000000.0;
			Print(1, "%2d %6.2f %12llu", n, razmisljao*100, count);
		}
		else if ( strstr(buff,"sortmoves") != 0 )
		{
			score = generate_moves(NML, Ncb);
			sortmoves(NML, 0);
			capt = NML->captcount;
			int limes;
			it = (capt > 218) ? 218 : 0;
			limes = (capt > 218) ? capt : NML->quietcount;

sortnLegal_for:
			for (; it < limes ; it++)
			{
				Print(1, "%d %s\n", it, print_smith_notation( &NML->mdata[it]));
			}

			if (it == (capt ))
			{
				it = 0;
				limes = NML->quietcount;
				goto sortnLegal_for;
			}

			Print(1, "quiet count:%3d\n", NML->quietcount);
			Print(1, "capt count:%4d\n", NML->captcount-218);
			Print(1, "moves count:%3d\n", NML->quietcount + NML->captcount - 218);
		}
		else if ( strstr(buff,"nLegal") != NULL )
		{
			score = generate_moves(NML, Ncb);
			capt = NML->captcount;
			int limes;
			it = (capt > 218) ? 218 : 0;
			limes = (capt > 218) ? capt : NML->quietcount;

nLegal_for:
			for (; it < limes ; it++)
			{
				Print(1, "%d %s\n", it, print_smith_notation( &(NML->mdata[it])));
			}

			if (it == (capt ))
			{
				it = 0;
				limes = NML->quietcount;
				goto nLegal_for;
			}

			Print(1, "quiet count:%3d\n", NML->quietcount);
			Print(1, "capt count:%4d\n", 218-NML->captcount);
			Print(1, "moves count:%3d\n", NML->quietcount + NML->captcount - 218);
		}
		else if ( strstr(buff,"ntestLegal") != 0 )
		{
			score = gm1(NML, Ncb);
			capt = NML->captcount;
			int limes;
			it = (capt > 218) ? 218 : 0;
			limes = (capt > 218) ? capt : NML->quietcount;

ntestLegal_for:
			for (; it < limes ; it++)
			{
				Print(1, "%d %s\n", it, print_smith_notation( &NML->mdata[it]));
			}

			if (it == (capt ))
			{
				it = 0;
				limes = NML->quietcount;
				goto ntestLegal_for;
			}

			Print(1, "quiet count:%3d\n", NML->quietcount);
			Print(1, "capt count:%4d\n", 255-NML->captcount);
			Print(1, "moves count:%3d\n", NML->quietcount + NML->captcount - 218);
		}
		else if ( strstr(buff,"legal") != 0 )
		{
			marray = malloc( sizeof(move_1)*256);
			score = generate_moves_1(cb, marray);
			for (n = 0; n < score; n++)
			{
				Print(1, "%d %s\n", n+1, printmove_1( &marray[n]));
			}
			Print(1, "moves count: %d\n", score);
			free(marray);
		}
		else if (strstr(buff, "setboard") != NULL)
		{
			char fen[127];
			sscanf(buff, "setboard %[^\n]", fen);

			Ncb = NimportFEN(fen);
			cb = importFEN(fen);
			set_zobrist_1( &cb);
			set_zobrist_keys(&Ncb);

			Print(0, "%s", printboard(Ncb));
		}
		else if (strstr(buff,"do_move") != NULL )
		{
			sscanf(buff, "do_move %d", &n);
			ln[d] = n;
			score = generate_moves(&NML[d], Ncb);

			Print(0, "%s", printBits(8, &NML[d].mdata[n]));
			do_move( &Ncb, NML[d].mdata[n]);
			Print(0, "%s", printboard(Ncb));
			d++;
		}
		else if ( strstr(buff,"undomove") != NULL )
		{
			d--;
			undo_move( &Ncb, &NML[d], NML[d].mdata[ln[d]]);
			Print(0, "%s", printboard(Ncb));

		}
		else if ( strstr(buff,"state") != NULL )
		{
			print_state( Ncb);
		}
		else if ( strstr(buff,"Ndisp") != NULL )
		{
			Print(1, "%s", printboard(Ncb));
		}
		else if ( strstr(buff,"disp") != NULL )
		{
			Print(1, "%s", printboard_1(cb));
		}
		else if ( strstr(buff,"change_stm") != NULL )
		{
			cb.info ^= 1LL;
			Ncb.info ^= 1LL << 14;
			set_zobrist_1( &cb);
			Print(0, "%s", printboard_1(cb));
		}
		else if ( strstr(buff,"new") != NULL )
		{
			cb = importFEN(START_FEN);
			Ncb = NimportFEN(START_FEN);
			set_zobrist_1( &cb);
			set_zobrist_keys( &Ncb);
		}
		else if ( strstr(buff,"quit") != NULL )
		{
			break;
		}
	}
}
