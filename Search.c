#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Search.h"
#include "MoveGeneration.h"
#include "TranspositionTable.h"

extern int en_state;

Nline Npline;
int stop = 0;

#define QUIESCE_DEPTH	-38
int Quiesce( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int draft)
{
	int score;

	int stand_pat = color*evaluate(*pos, draft, color, pos);

	//	OGRANICENJE PO DEPTH	////////////
	if (depth < QUIESCE_DEPTH)
	{
		printf("***QUIESCE_DEPTH***\n");
		return stand_pat;
	}

	if ( stand_pat >= beta)	return beta;
	if ( alpha < stand_pat )	alpha = stand_pat;

	pline->cmove = 0;

	unsigned capt_count;
	generate_movesN(&NML[draft+1], *pos);
	capt_count = NML[draft+1].captcount;
	int it;

//	printf("* cap=%d\n", capt_count);

	for (it = 218; (it < capt_count) && (en_state == THINKING); it++)
	{
		Ndo_move(pos, NML[draft+1].mdata[it]);
		score = -Quiesce( pos, pline, -beta, -alpha, -color, depth-1, draft+1);
		Nundo_move(pos, &NML[draft+1], NML[draft+1].mdata[it]);


		if( score >= beta )	return beta;

		if( score > alpha )
		{
			NML[draft+1].mdata[it];
			alpha = score;

			/*printf("%d = %d	a %d b %d col %d	", depth, score, alpha, beta, color);
			printmoveN( &NML[draft+1].mdata[it]);
			printf("\n");*/

			/*pline->argmove[0] = pick;
			memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(Nmove));
			pline->cmove = nline.cmove +1;*/
		}
    	}
	return alpha;
}

int search( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int draft)
{
	//if (en_state == PONDERING) printf("!");
	//printf("%d", en_state);
	
	int best, val, old_alpha, it, limes;
	Nmove pick = NULL, hash_move = 0, do_move;
	unsigned char quiet, capt, flag;
	Nline nline;
	nTTentry *entry;

        entry = nTTlookup( pos->zobrist);
        if (entry)
	{
        	if (((entry->data >> 32)  & 0x0000000000FF) >= depth)
	        {	
			val = (short)(entry->data >> 48);
			flag = entry->data >> 40 & 0x000000000000003;

	                //exact
        	        if (flag == 1)
			return val;
                	//lowerbound
	                if (flag == 2)
	                {
        	                if ( val >= beta ) return val;
                	        if ( val > alpha ) alpha = val;
	                }
        	        //higherbound
	                if (flag == 3)
        	        {
                	        if ( val <= alpha ) return val;
                        	if ( val < beta ) beta = val;
	                }
        	}
		hash_move = (flag == 3) ? 0 : entry->data;		
	}

	if ( !depth ) 
  	{
    		//pline->cmove = 0;
		//return color * neval( pos);
		//return color*evaluate( *pos , draft, color, pos);
		return Quiesce( pos, pline, alpha, beta, color, depth, draft); 
  	}
	  	
	old_alpha = alpha;
	best = -WIN-300;
        generate_movesN(&NML[depth] , *pos);
	//if (hash_move > 0) sortmoves( &NML[depth], hash_move);

        quiet = NML[depth].quietcount;
        capt = NML[depth].captcount;
	
	it = (capt > 218) ? 218 : 0;
	limes = (capt > 218) ? capt : quiet;
	
forpetlja: 
	for (; (it < limes) && (en_state == THINKING); it++  )
        {
		do_move = NML[depth].mdata[it];
		Ndo_move(pos, do_move);
		val = -search( pos, &nline, -beta, -alpha, -color, depth - 1, draft + 1);
                Nundo_move(pos, &NML[depth], do_move);
                
                if ( val >= beta)       
		{	
			U64 data = 0ULL;

                        data ^= (short)val;
                        data <<= 48;
                        data ^= do_move;
                        data ^= (U64)depth << 32;
                        data ^= (2ULL << 40);

			nTTstore( pos->zobrist, data);

			return val; //fail-soft
		}

                if ( val > best)
                {
                        best = val;
                        if ( val > alpha )
                        {
                                alpha = val;
				pick = NML[depth].mdata[it];
                                /*pline->argmove[0] = NML[depth].mdata[it];
                                memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(Nmove));
                                pline->cmove = nline.cmove +1;*/
				//	extract PV from TT	/////////////////
                        }
                }
		//if (en_state == PONDERING) printf("!");	
        }

	if (it == (capt ) && (en_state == THINKING))
	{
		it = 0;
		limes = quiet;
		goto forpetlja;
	}

        if (best > old_alpha)
        {
                        U64 data = 0ULL;
			data ^= (short)best;
			data <<= 48;
			data ^= pick;
			

 			data ^= (U64)depth << 32;
			
			data ^= (1ULL << 40);

			////	STORE PV node	/////
			nTTstore( pos->zobrist, data);
        }
        // fail low implies upperbound
        if (best <= old_alpha)
        {
                        U64 data = 0ULL;

                        data ^= (short)best;
                        data <<= 48;
                        data ^= (U64)depth << 32;
                        data ^= (3ULL << 40);

			nTTstore( pos->zobrist, data);
        }
	
	return best;
}


int neval( Nboard *b)
{	
	int w_score = 0, b_score = 0;

	w_score += 9 * __builtin_popcountll( b->pieceset[1] );
	w_score += 5 * __builtin_popcountll( b->pieceset[2] );
	w_score += 3 * __builtin_popcountll( b->pieceset[3] );
	w_score += 3 * __builtin_popcountll( b->pieceset[4] );
	w_score +=     __builtin_popcountll( b->pieceset[5] );

	b_score -= 9 * __builtin_popcountll( b->pieceset[9] );
	b_score -= 5 * __builtin_popcountll( b->pieceset[10] );
	b_score -= 3 * __builtin_popcountll( b->pieceset[11] );
	b_score -= 3 * __builtin_popcountll( b->pieceset[12] );
	b_score -=     __builtin_popcountll( b->pieceset[13] );

	count++;
	return w_score + b_score;
}

int nnegamax( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int draft)
{
	int best, val, it, limes;
	unsigned char quiet, capt;
	Nline nline;

	if ( !depth ) 
	{
		pline->cmove = 0;
	  return color*evaluate( *pos, draft, color, pos);
	}

	best = -WIN-300;

	generate_movesN(&NML[depth] , *pos);
	quiet = NML[depth].quietcount;
	capt = NML[depth].captcount;

	it = (capt > 218) ? 218 : 0;
	limes = (capt > 218) ? capt : quiet;

	forpetlja: 
	for (; it < limes ; it++  )
	{
		Ndo_move(pos, NML[depth].mdata[it]);
		val = -nnegamax( pos, &nline, -beta, -alpha, -color, depth - 1, draft +1);
		Nundo_move(pos, &NML[depth], NML[depth].mdata[it]);
		
		if ( val >= beta)       return val; //fail-soft

		if ( val > best)
		{
			best = val;
			if ( val > alpha )
			{
				alpha = val;
				 pline->argmove[0] = NML[depth].mdata[it];
				  memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(Nmove));
				  pline->cmove = nline.cmove +1;
			//	printNline(*pline);
			}
		}	
	}

	if (it == (capt ))
	{
		it = 0;
		limes = quiet;
		goto forpetlja;
	}

	return best;
}

int  pvs_02(Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int draft)
{
	int best, val, old_alpha, it, limes;
	Nmove pick = NULL, hash_move = 0;
	unsigned char quiet, capt, flag;
	Nline nline;
	nTTentry *entry;

        entry = nTTlookup( pos->zobrist);
        if (entry)
	{
        if (((entry->data >> 32)  & 0x0000000000FF) >= depth)
        {
                //hash move
                //TTmove = &entry->pick;
                //val = entry->data >> 8  & 0x000000FFFFF; //>> 2;
		val = (short)(entry->data >> 48);
		flag = entry->data >> 40 & 0x000000000000003;

		//val -= 1000;
 		//flag = entry->data >> 24 & 0x00000000000003;
               //if (entry->pick.info) dodaj_move( &TTmove, &entry->pick);


		//!!!!!!!ovdje mozda vracati i putanju uz val !!!!!!!
                //exact
                if //(entry->score & 0x0001)
                (flag == 1)      return val;
                //lowerbound
                if //(entry->score & 0x0002)    
                (flag == 2)
                {
                        if ( val >= beta ) return val;
                        if ( val > alpha ) alpha = val;
                }
                //higherbound
                if //(!(entry->score & 0x0003))
                (flag == 3)
                {
                        if ( val <= alpha ) return val;
                        if ( val < beta ) beta = val;
                }
        }
			hash_move = entry->data;		

	}
	

	if ( !depth ) 
  	{
    		pline->cmove = 0;
	  //return color * neval( pos);
	  return color*evaluate( *pos , draft, color, pos);
  	}
	  	
	old_alpha = alpha;
	best = -WIN-300;
        generate_movesN(&NML[depth] , *pos);
	//if (hash_move > 0) sortmoves( &NML[depth], hash_move);

        quiet = NML[depth].quietcount;
        capt = NML[depth].captcount;
	
	it = (capt > 218) ? 218 : 0;
	limes = (capt > 218) ? capt : quiet;
	
forpetlja: 
	for (; it < limes ; it++  )
        {
		Ndo_move(pos, NML[depth].mdata[it]);
		val = -pvs_02( pos, &nline, -beta, -alpha, -color, depth - 1, draft + 1);
                Nundo_move(pos, &NML[depth], NML[depth].mdata[it]);
                
                if ( val >= beta)       
		{	
			U64 data = 0ULL;
			/*data ^= NML[depth].mdata[it];
			data <<= 26;
			data ^= depth ^ ((val + 1000) << 8) ^ (2 << 24);	*/

                        data ^= (short)val;
                        data <<= 48;
                        data ^= NML[depth].mdata[it];


                        data ^= (U64)depth << 32;

                        data ^= (2ULL << 40);

			nTTstore( pos->zobrist, data);

			return val; //fail-soft
		}

                if ( val > best)
                {
                        best = val;
                        if ( val > alpha )
                        {
                                alpha = val;
				pick = NML[depth].mdata[it];
				/*printf("**");
				printmoveN( &pick);*/
                                 //pline->argmove[0] = NML[depth].mdata[it];
                                  //memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(Nmove));
                                  //pline->cmove = nline.cmove +1;
                        }
                }	
        }

	if (it == (capt ))
	{
		it = 0;
		limes = quiet;
		goto forpetlja;
	}

        if (best > old_alpha)
        {
                        U64 data = 0ULL;
			/*printf("\n+TTstore+%llu", pos->zobrist);
			printmovedetailsN(&pick);*/
			data ^= (short)best;
			data <<= 48;
			data ^= pick;
			

 			data ^= (U64)depth << 32;
			
			data ^= (1ULL << 40);
               //printBits( sizeof(U64), &data);
               //printNboard( pos);



			nTTstore( pos->zobrist, data);
                        /*entry->zobrist = pos->zobrist;
                        //entry->pick = pline->argmove[0];
                        entry->score = 0ULL | 1 | val << 2;
                        entry->depth = depth;*/

                        //TTstore( pos->zobrist, /*NULL,*/ depth, (0x0001 | (val << 2)));
                        //TTstore( pos->zobrist, pick, depth, best, 1);

        }
        // fail low implies upperbound
        if (best <= old_alpha)
        {
                        U64 data = 0ULL;
                        //data ^= depth ^ ((best + 1000) << 8) ^ (0x0000000000000003  << 24);
                        data ^= (short)best;
                        data <<= 48;


                        data ^= (U64)depth << 32;

                        data ^= (3ULL << 40);

			nTTstore( pos->zobrist, data);
                        /*entry->zobrist = pos->zobrist;
                        //entry->pick = pline->argmove[0];
                        entry->score = 0ULL | val << 2;
                        entry->depth = depth;*/

                        //TTstore( pos->zobrist, /*NULL,*/ depth, (0ULL | (val << 2)));
                        //TTstore( pos->zobrist, NULL, depth, best, 3);
        }
	

	return best;
}

//IMA BUG
void sortmoves(Nmovelist *m_list, Nmove PV_move)
{
	//if (PV_move == 0)
	//	return;

        int start= (m_list->captcount > 218) ? 218 : 0;
        int limes = (m_list->captcount > 218) ? m_list->captcount : m_list->quietcount;
	int it = start;
	unsigned piece, capt;
        sortforpetlja:
        for (; it < limes ; it++  )
        {
		if (PV_move > 0)	if (m_list->mdata[it] == PV_move)	{
			m_list->mdata[it] = m_list->mdata[start];
			m_list->mdata[start] = PV_move;
			/*printf("!!");
			printmoveN(&m_list->mdata[start]); */
			
		}
		if (it >= 218)
		{
			piece = m_list->mdata[it] & 0x00000007;
			capt = (m_list->mdata[it] >> 3) & 0x000000007;
			switch (piece){
				case 0: piece = 30; break;
				case 1: piece = 17; break;
				case 2: piece = 13; break;
				case 3: piece = 11; break;
				case 4: piece = 11; break;
				case 5: piece = 9; break;
			}
			switch (capt){
				case 1: capt = 9; break;
				case 2: capt = 5; break;
				case 3: capt = 3; break;
				case 4: capt = 3; break;
				case 5: capt = 1; break;
			}
			m_list->mdata[it] ^= (piece - capt) << 26;
			
			//printmoveN(&m_list->mdata[it]);
			//printf("mvv-lvv %u\n", m_list->mdata[it] >> 26);
	
		}
		
        }

        if (it == (m_list->captcount ))
        {
                it = 0;
                limes = m_list->quietcount;
                goto sortforpetlja;
        }

	unsigned value, j;
	if (start > 0)
	for (; it < m_list->captcount; it++)
	{
		//printf("*");
		value = m_list->mdata[it] ;
		j = it - 1;
		while ((j >= start) && ((m_list->mdata[j] >> 26) > (value >> 26)))
		{
			m_list->mdata[j+1] = m_list->mdata[j];
			j--;
		}
		m_list->mdata[j+1] = value;
	}


	
}

int pvs_01( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth, int is_PV, int draft)
{
	int best, val, it, limes;
	unsigned char quiet, capt;
	Nline nline;

	if ( !depth ) 
	{
		pline->cmove = 0;
	  return color * neval( pos);
	}

	best = -WIN;
	generate_movesN(&NML[depth] , *pos);
	/*printf("--");
	printf("draft %d Pv.cmove %d ", draft, PV.cmove);
	printmoveN(&PV.argmove[draft]);*/
	//sortmoves(&NML[depth], (draft < PV.cmove) ? PV.argmove[draft] : 0);
	quiet = NML[depth].quietcount;
	capt = NML[depth].captcount;

	it = (capt > 218) ? 218 : 0;
	limes = (capt > 218) ? capt : quiet;

	forpetlja: 
	for (; it < limes ; it++  )
	{
		Ndo_move(pos, NML[depth].mdata[it]);
		val = -pvs_01( pos, &nline, -beta, -alpha, -color, depth - 1, is_PV, draft + 1);
		Nundo_move(pos, &NML[depth], NML[depth].mdata[it]);
		
		//if ( val >= beta)       return val; //fail-soft

		if ( val > best)
		{
			best = val;
			if ( val > alpha )
			{
				alpha = val;
				 pline->argmove[0] = NML[depth].mdata[it];
				  memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(Nmove));
				  pline->cmove = nline.cmove +1;
			}
		}	
	}

	if (it == (capt ))
	{
		it = 0;
		limes = quiet;
		goto forpetlja;
	}

	return best;
}

int ntestnegamax( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth)
{
        int best, val, it;
        unsigned char quiet;
        Nline nline; 

        if ( !depth )
        {
                pline->cmove = 0;
          return color * neval( pos); 
        }                
                 
        best = -WIN;
        generate_movesN_test(&NML[depth] , *pos);
        quiet = NML[depth].quietcount;
        //capt = NML[depth].captcount;

        /*it = (capt > 218) ? 218 : 0;
        limes = (capt > 218) ? capt : quiet;*/
	
        for (it = quiet-1; it >= 0 ; it--  )
        {
		//printf("%d",it);
                Ndo_move(pos, NML[depth].mdata[it]);
                val = -ntestnegamax( pos, &nline, -beta, -alpha, -color, depth - 1);
                Nundo_move(pos, &NML[depth], NML[depth].mdata[it]);

                if ( val >= beta)       return val; //fail-soft

                if ( val > best)
                {
                        best = val;
                        if ( val > alpha )
                        {
                                alpha = val;
                                 pline->argmove[0] = NML[depth].mdata[it];
                                  memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(Nmove));
                                  pline->cmove = nline.cmove +1;
                        }
                }       
        }

        /*if (it == (capt ))
        {
                it = 0;
                limes = quiet;
                goto forpetlja;
        }*/
        
        return best;
}

int nTTnegamax( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth)
{
	int best, val, old_alpha, it, limes;
	Nmove pick = NULL;
	unsigned char quiet, capt, flag;
	Nline nline;
	nTTentry *entry;

        entry = nTTlookup( pos->zobrist);
        if (entry)
        if (((entry->data >> 32)  & 0x0000000000FF) >= depth)
        {
                //hash move
                //TTmove = &entry->pick;
                //val = entry->data >> 8  & 0x000000FFFFF; //>> 2;
		val = (short)(entry->data >> 48);
		flag = entry->data >> 40 & 0x000000000000003;
		//val -= 1000;
 		//flag = entry->data >> 24 & 0x00000000000003;
               //if (entry->pick.info) dodaj_move( &TTmove, &entry->pick);


		//!!!!!!!ovdje mozda vracati i putanju uz val !!!!!!!
                //exact
                if //(entry->score & 0x0001)
                (flag == 1)      return val;
                //lowerbound
                if //(entry->score & 0x0002)    
                (flag == 2)
                {
                        if ( val >= beta ) return val;
                        if ( val > alpha ) alpha = val;
                }
                //higherbound
                if //(!(entry->score & 0x0003))
                (flag == 3)
                {
                        if ( val <= alpha ) return val;
                        if ( val < beta ) beta = val;
                }
        }

	if ( !depth ) 
  	{
    		//pline->cmove = 0;
	  return color * neval( pos);
  	}
	  	
	old_alpha = alpha;
	best = -WIN;
        generate_movesN(&NML[depth] , *pos);
        quiet = NML[depth].quietcount;
        capt = NML[depth].captcount;
	
	it = (capt > 218) ? 218 : 0;
	limes = (capt > 218) ? capt : quiet;
	
forpetlja: 
	for (; it < limes ; it++  )
        {
		Ndo_move(pos, NML[depth].mdata[it]);
		val = -nTTnegamax( pos, &nline, -beta, -alpha, -color, depth - 1);
                Nundo_move(pos, &NML[depth], NML[depth].mdata[it]);
                
                if ( val >= beta)       
		{	
			U64 data = 0ULL;
			/*data ^= NML[depth].mdata[it];
			data <<= 26;
			data ^= depth ^ ((val + 1000) << 8) ^ (2 << 24);	*/

                        data ^= (short)val;
                        data <<= 48;
                        data ^= NML[depth].mdata[it];


                        data ^= (U64)depth << 32;

                        data ^= (2ULL << 40);

			nTTstore( pos->zobrist, data);

			return val; //fail-soft
		}

                if ( val > best)
                {
                        best = val;
                        if ( val > alpha )
                        {
                                alpha = val;
				pick = NML[depth].mdata[it];
				/*printf("**");
				printmoveN( &pick);*/
                                 /*pline->argmove[0] = NML[depth].mdata[it];
                                  memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(Nmove));
                                  pline->cmove = nline.cmove +1;*/
                        }
                }	
        }

	if (it == (capt ))
	{
		it = 0;
		limes = quiet;
		goto forpetlja;
	}

        if (best > old_alpha)
        {
                        U64 data = 0ULL;
			/*printf("\n+TTstore+%llu", pos->zobrist);
			printmovedetailsN(&pick);*/
			data ^= (short)best;
			data <<= 48;
			data ^= pick;
			

 			data ^= (U64)depth << 32;
			
			data ^= (1ULL << 40);
               //printBits( sizeof(U64), &data);
               //printNboard( pos);



			nTTstore( pos->zobrist, data);
                        /*entry->zobrist = pos->zobrist;
                        //entry->pick = pline->argmove[0];
                        entry->score = 0ULL | 1 | val << 2;
                        entry->depth = depth;*/

                        //TTstore( pos->zobrist, /*NULL,*/ depth, (0x0001 | (val << 2)));
                        //TTstore( pos->zobrist, pick, depth, best, 1);

        }
        // fail low implies upperbound
        if (best <= old_alpha)
        {
                        U64 data = 0ULL;
                        //data ^= depth ^ ((best + 1000) << 8) ^ (0x0000000000000003  << 24);
                        data ^= (short)best;
                        data <<= 48;


                        data ^= (U64)depth << 32;

                        data ^= (3ULL << 40);

			nTTstore( pos->zobrist, data);
                        /*entry->zobrist = pos->zobrist;
                        //entry->pick = pline->argmove[0];
                        entry->score = 0ULL | val << 2;
                        entry->depth = depth;*/

                        //TTstore( pos->zobrist, /*NULL,*/ depth, (0ULL | (val << 2)));
                        //TTstore( pos->zobrist, NULL, depth, best, 3);
        }
	

	return best;
}

U64 NPerft(int depth, Nboard *arg, Nmovelist *ml)
{
    unsigned char capt, quiet, it;
    U64 nodes = 0;
 
    //if (depth == 0) return 1;
    if (depth == 1) return generate_movesN( &ml[depth], *arg);
 
    generate_movesN( &ml[depth], *arg);
    quiet = ml[depth].quietcount;
    capt = ml[depth].captcount;
    //capt++;
    for (it = 218; it > capt && !stop; it--)
    //for (it = 0; (it < 20) ; it++)
    {
 	//printmoveN(&ml[depth].mdata[it]);
	//printf("depth %d it %d\n", depth, it);
        Ndo_move(arg, ml[depth].mdata[it]);
        nodes += NPerft(depth - 1, arg, ml);
        Nundo_move(arg, &ml[depth], ml[depth].mdata[it]);
    }
    for (it = 0; it < quiet && !stop; it++)
    //for (it = 0; (it < 20) ; it++)
    {
 	//printmoveN(&ml[depth].mdata[it]);
	//printf("depth %d it %d\n", depth, it);
        Ndo_move(arg, ml[depth].mdata[it]);
        nodes += NPerft(depth - 1, arg, ml);
       /* ff = ml[depth].mdata[it];
           	p_type = ml[depth].mdata[it] & 0x00000007;
              	capt_type = ml[depth].mdata[it] >> 3 & 0x0000000000000007;
	        sq_from = ml[depth].mdata[it] >> 6 & 0x0000003F;
	        sq_dest = ml[depth].mdata[it] >> 12 & 0x0000003F;
        if ((p_type == 1) &&  (sq_from == 60) &&  (sq_dest == 24))
{
	        printf("SU it %d fr %d dst %d\n", it, sq_from, sq_dest);
	        printf("m\n");
	        printBits(4, &ml[depth].mdata[it]);
                printf("p %d capt %d\n", p_type, capt_type);
                printmoveN(&ml[depth].mdata[it]);
}                */
        Nundo_move(arg, &ml[depth], ml[depth].mdata[it]);
	/*if (arg->pieceset[5] & 1LL)
	{
           	p_type = ml[depth].mdata[it] & 0x00000007;
              	capt_type = ml[depth].mdata[it] >> 3 & 0x0000000000000007;
	        sq_from = ml[depth].mdata[it] >> 6 & 0x0000003F;
	        sq_dest = ml[depth].mdata[it] >> 12 & 0x0000003F;
	        printf("SUfr %d dst %d\n", sq_from, sq_dest);
	        printf("m\n");
	        printBits(4, &ml[depth].mdata[it]);
                printf("p %d capt %d\n", p_type, capt_type);
                printmoveN(&ml[depth].mdata[it]);
 	        
               
	
	        }
	if (stop)
	{
           	p_type = ff & 0x00000007;
              	capt_type = ff >> 3 & 0x0000000000000007;
	        sq_from = ff >> 6 & 0x0000003F;
	        sq_dest = ff >> 12 & 0x0000003F;
	        printf("SSSSSSSSSfr %d dst %d \n", sq_from, sq_dest);
	        printf("m\n");
	        printBits(4, &ff);
                printf("p %d capt %d\n", p_type, capt_type);
                printmoveN(&ff);
      
               
	
	        }*/
        
    }
    //
    //printf("obr %d\n", i);
    
    return nodes;
}
U64 Ndivide_perft(int depth, Nboard *arg, Nmovelist *ml)
{
    unsigned char capt, quiet, it;
    //Nmove ff;
    U64 childs, nodes = 0;
    
    mfree = 0;
 
    //if (depth == 0) return generate_movesN( &ml[depth], *arg);

     generate_movesN( &ml[depth], *arg);
    quiet = ml[depth].quietcount;
    capt = ml[depth].captcount;
    //capt++;
    //it = capt;
    //for (it = 0; (it < quiet) || (it > ml[depth].captcount); it++)
    //for (it = 0; (it < 20) ; it++)
    for (it = 218; it > capt && !stop; it++)
    {
    	childs = 0;
	//printf("depth %d it %d\n", depth, it);
	printmoveN(&ml[depth].mdata[it]);
        Ndo_move(arg, ml[depth].mdata[it]);
		//printBits(1, &arg->B);
		//printBits(8, &arg->b.K);
        
        childs += NPerft(depth - 1, arg, ml);
	//printf("depth %d\n", depth);
	//printmoveN(&ml[depth].mdata[it]);
	printf(" %llu\n", childs);
        Nundo_move(arg, &ml[depth], ml[depth].mdata[it]);
	//fr = it;
	nodes += childs;
    }
    for (it = 0; it < quiet && !stop; it++)
    //for (it = 0; (it < 20) ; it++)
    {
	//printf("depth %d it %d\n", depth, it);
    	childs = 0;
	printmoveN(&ml[depth].mdata[it]);
        Ndo_move(arg, ml[depth].mdata[it]);
		//printBits(1, &arg->B);
		//printBits(8, &arg->b.K);
        
        childs += NPerft(depth - 1, arg, ml);
	//printf("depth %d\n", depth);
	//printmoveN(&ml[depth].mdata[it]);
	printf(" %llu\n", childs);
        //ff = ml[depth].mdata[it];
        Nundo_move(arg, &ml[depth], ml[depth].mdata[it]);
	//fr = it;
	/*if (stop)
	{
           	p_type = ff & 0x00000007;
              	capt_type = ff >> 3 & 0x0000000000000007;
	        sq_from = ff >> 6 & 0x0000003F;
	        sq_dest = ff >> 12 & 0x0000003F;
	        printf("SSSSSSSSSfr %d dst %d \n", sq_from, sq_dest);
	        printf("m\n");
	        printBits(4, &ff);
                printf("p %d capt %d\n", p_type, capt_type);
                printmoveN(&ff);
      
               
	
	        }*/
	nodes += childs;
    }
	//printf("count %llu obr %llu\n ", nodes, mfree);
    return nodes;
}
