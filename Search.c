#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Search.h"
#include "MoveGeneration.h"
#include "TranspositionTable.h"

move *marray = NULL;
line pline;
Nline Npline;
int stop = 0;

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

int eval( board *b)
{	
	int w_score = 0, b_score = 0;

	w_score += 9 * __builtin_popcountll( b->w.Q );
	w_score += 5 * __builtin_popcountll( b->w.R );
	w_score += 3 * __builtin_popcountll( b->w.B );
	w_score += 3 * __builtin_popcountll( b->w.N );
	w_score +=     __builtin_popcountll( b->w.P );

	b_score -= 9 * __builtin_popcountll( b->b.Q );
	b_score -= 5 * __builtin_popcountll( b->b.R );
	b_score -= 3 * __builtin_popcountll( b->b.B );
	b_score -= 3 * __builtin_popcountll( b->b.N );
	b_score -=     __builtin_popcountll( b->b.P );

	count++;
	return w_score + b_score;
}

int nnegamax( Nboard *pos, Nline *pline, int alpha, int beta, int color, int depth)
{
	int best, val, movecount, it;
	unsigned char quiet, capt;
	move *list;
	Nline nline;

	if ( !depth ) 
  	{
    		pline->cmove = 0;
	  return color * neval( pos);
  	}
  	
	best = -WIN;
        movecount = generate_movesN(&NML[depth] , *pos);
        quiet = NML[depth].quietcount;
        capt = NML[depth].captcount;
        for (it = 255; it > capt; it--)
   	{
        	Ndo_move(pos, NML[depth].mdata[it]);
	        val = -nnegamax( pos, &nline, -beta, -alpha, -color, depth - 1);
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
        for (it = 0; it < quiet; it++)
   	{
        	Ndo_move(pos, NML[depth].mdata[it]);
	        val = -nnegamax( pos, &nline, -beta, -alpha, -color, depth - 1);
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
	return best;
}


//node **arg, int dubina, int alfa, int beta, atom *igracset, atom *protivnikset, pozicija **pz, int player, int *izbor
int mnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth)
{
	int best, val;
	move *list, *it;
	line nline;

	if ( !depth ) 
  	{
    		pline->cmove = 0;
	  return color * eval( pos);
  	}
  	
	best = -WIN;
	list = generate_movesALLOC( *pos);

	for (it = list; it; it = it->next )
    	{

        	do_move(pos, it);
	        val = -mnegamax( pos, &nline, -beta, -alpha, -color, depth - 1);
        	undo_move(pos, it);
        	
        	/*//r = x ^ ((x ^ y) & -(x < y)); // max(x, y)
		best = best ^ (( best ^ val ) & -( best < val ));
		alpha = alpha ^ (( alpha ^ val ) & -( alpha < val ));
		/*if (alpha >= beta)
			break;*/
			
		//fail high implies lowerbound
		if ( val >= beta)
		{
    			delete_movelist( list);
			return val; //fail-soft
		}
			
		if ( val > best)
		{
			best = val;
			if ( val > alpha )
			{
				alpha = val;
        			 pline->argmove[0] = *it;
       				  memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(move));
       				  pline->cmove = nline.cmove +1;

				/*pick[ depth ] = *it;
				printf(" %d ", depth);
				printmove( &pick[depth] );*/
	 		}
		}
		
	}
	delete_movelist( list);
	return best;
}

int negamax( board *pos, line *pline, int alpha, int beta, int color, int depth)
{
	int best, val, movecount, it;
	move *list;
	line nline;

	if ( !depth ) 
  	{
    		pline->cmove = 0;
	  return color * eval( pos);
  	}
  	
	best = -WIN;
        movecount = generate_moves( *pos, &marray[ 216 * depth ]);
        for (it = 216*depth + movecount - 1; it >= 216*depth; it--)
   	{

        	do_move(pos, &marray[it]);
	        val = -negamax( pos, &nline, -beta, -alpha, -color, depth - 1);
        	undo_move(pos, &marray[it]);
        	
        	/*//r = x ^ ((x ^ y) & -(x < y)); // max(x, y)
		best = best ^ (( best ^ val ) & -( best < val ));
		alpha = alpha ^ (( alpha ^ val ) & -( alpha < val ));
		/*if (alpha >= beta)
			break;*/
			
		//fail high implies lowerbound
		if ( val >= beta)
		{
			return val; //fail-soft
		}
			
		if ( val > best)
		{
			best = val;
			if ( val > alpha )
			{
				alpha = val;
        			 pline->argmove[0] = marray[it];
       				  memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(move));
       				  pline->cmove = nline.cmove +1;

				/*pick[ depth ] = *it;
				printf(" %d ", depth);
				printmove( &pick[depth] );*/
	 		}
		}
		
	}
	return best;
}

int mTTnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth)
{
	int best, val, old_alpha;
	move *list, *it, *TTmove = NULL, *pick = NULL, *bm = NULL ;
	line nline;
	TTentry *entry;

	entry = TTlookup( pos->zobrist);
	if (entry)
	if (entry->depth >= depth)
	{
		//hash move
		//TTmove = &entry->pick;

		val = entry->score; //>> 2;
		//if (entry->pick.info)	dodaj_move( &TTmove, &entry->pick);

		//exact
		if //(entry->score & 0x0001)
		(entry->flag == 1)	return val;
		//lowerbound
		if //(entry->score & 0x0002)	
		(entry->flag == 2)
		{
			if ( val >= beta ) return val;
			if ( val > alpha ) alpha = val;
		}
		//higherbound
		if //(!(entry->score & 0x0003))
		(entry->flag == 3)
		{
			if ( val <= alpha ) return val;
			if ( val < beta ) beta = val;
		}
	}
	
	if ( !depth ) 
  	{
    		//pline->cmove = 0;
	  return color * eval( pos);
  	}
  	
  	old_alpha = alpha;
	best = -WIN;
	list = generate_movesALLOC( *pos);
	//dodaj hashmove
	//printf("countmoves %d\n", countmoves(list) );
	//if (TTmove)	order_moves( &list, TTmove);

	for (it = list; it; it = it->next )
    	{

        	do_move(pos, it);
	        val = -mTTnegamax( pos, &nline, -beta, -alpha, -color, depth - 1);
        	undo_move(pos, it);
        	
        	/*//r = x ^ ((x ^ y) & -(x < y)); // max(x, y)
		best = best ^ (( best ^ val ) & -( best < val ));
		alpha = alpha ^ (( alpha ^ val ) & -( alpha < val ));
		/*if (alpha >= beta)
			break;*/
			
		//fail high implies lowerbound
		if ( val >= beta)
		{
			//entry->pick = pline->argmove[0];

			//TTstore( pos->zobrist, depth, (0x0002 | (val << 2)));
			TTstore( pos->zobrist, it, depth, val, 2);

    			delete_movelist( list);
			return val; //fail-soft
		}
			
		if ( val > best)
		{
			best = val;
			bm = it;
			if ( val > alpha )
			{
				alpha = val;
				pick = it;
        			 /*pline->argmove[0] = *it;
       				  memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(move));
       				  pline->cmove = nline.cmove +1;*/

				/*pick[ depth ] = *it;
				printf(" %d ", depth);
				printmove( &pick[depth] );*/
	 		}
		}
		
	}
	
	//TTstore
	// exact score
	if (best > old_alpha)  
	{
			/*entry->zobrist = pos->zobrist;
			//entry->pick = pline->argmove[0];
			entry->score = 0ULL | 1 | val << 2;
			entry->depth = depth;*/

			//TTstore( pos->zobrist, /*NULL,*/ depth, (0x0001 | (val << 2)));
			TTstore( pos->zobrist, pick, depth, best, 1);
	
	}
	// fail low implies upperbound
	if (best <= old_alpha)
	{
			/*entry->zobrist = pos->zobrist;
			//entry->pick = pline->argmove[0];
			entry->score = 0ULL | val << 2;
			entry->depth = depth;*/

			//TTstore( pos->zobrist, /*NULL,*/ depth, (0ULL | (val << 2)));
			TTstore( pos->zobrist, NULL, depth, best, 3);
	}

	delete_movelist( list);
	return best;
}

int TTnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth)
{
	int best, val, old_alpha, movecount, it;
	move *list, *TTmove = NULL, *pick = NULL, *bm = NULL ;
	line nline;
	TTentry *entry;

	entry = TTlookup( pos->zobrist);
	if (entry)
	if (entry->depth >= depth)
	{
		//hash move
		//TTmove = &entry->pick;

		val = entry->score; //>> 2;
		//if (entry->pick.info)	dodaj_move( &TTmove, &entry->pick);

		//exact
		if //(entry->score & 0x0001)
		(entry->flag == 1)	return val;
		//lowerbound
		if //(entry->score & 0x0002)	
		(entry->flag == 2)
		{
			if ( val >= beta ) return val;
			if ( val > alpha ) alpha = val;
		}
		//higherbound
		if //(!(entry->score & 0x0003))
		(entry->flag == 3)
		{
			if ( val <= alpha ) return val;
			if ( val < beta ) beta = val;
		}
	}
	
	if ( !depth ) 
  	{
    		//pline->cmove = 0;
	  return color * eval( pos);
  	}
  	
  	old_alpha = alpha;
	best = -WIN;
        movecount = generate_moves( *pos, &marray[ 216 * depth ]);
	//dodaj hashmove
	//printf("countmoves %d\n", countmoves(list) );
	//if (TTmove)	order_moves( &list, TTmove);

        for (it = 216*depth + movecount - 1; it >= 216*depth; it--)
    	{

        	do_move(pos, &marray[it]);
	        val = -TTnegamax( pos, &nline, -beta, -alpha, -color, depth - 1);
        	undo_move(pos, &marray[it]);
        	
        	/*//r = x ^ ((x ^ y) & -(x < y)); // max(x, y)
		best = best ^ (( best ^ val ) & -( best < val ));
		alpha = alpha ^ (( alpha ^ val ) & -( alpha < val ));
		/*if (alpha >= beta)
			break;*/
			
		//fail high implies lowerbound
		if ( val >= beta)
		{
			//entry->pick = pline->argmove[0];

			//TTstore( pos->zobrist, depth, (0x0002 | (val << 2)));
			TTstore( pos->zobrist, &marray[it], depth, val, 2);

    			//delete_movelist( list);
			return val; //fail-soft
		}
			
		if ( val > best)
		{
			best = val;
			bm = &marray[it];
			if ( val > alpha )
			{
				alpha = val;
				pick = &marray[it];
        			 /*pline->argmove[0] = *it;
       				  memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(move));
       				  pline->cmove = nline.cmove +1;*/

				/*pick[ depth ] = *it;
				printf(" %d ", depth);
				printmove( &pick[depth] );*/
	 		}
		}
		
	}
	
	//TTstore
	// exact score
	if (best > old_alpha)  
	{
			/*entry->zobrist = pos->zobrist;
			//entry->pick = pline->argmove[0];
			entry->score = 0ULL | 1 | val << 2;
			entry->depth = depth;*/

			//TTstore( pos->zobrist, /*NULL,*/ depth, (0x0001 | (val << 2)));
			TTstore( pos->zobrist, pick, depth, best, 1);
	
	}
	// fail low implies upperbound
	if (best <= old_alpha)
	{
			/*entry->zobrist = pos->zobrist;
			//entry->pick = pline->argmove[0];
			entry->score = 0ULL | val << 2;
			entry->depth = depth;*/

			//TTstore( pos->zobrist, /*NULL,*/ depth, (0ULL | (val << 2)));
			TTstore( pos->zobrist, NULL, depth, best, 3);
	}

	//delete_movelist( list);
	return best;
}

int rootnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth)
{
	int best, val;
	move *list, *it;
	line nline;
	
	if ( !depth ) return 0;

	best = -WIN;
	list = generate_movesALLOC( *pos);
	
	for (it = list; it; it = it->next )
    	{

        	do_move(pos, it);
	        val = -negamax( pos, &nline, -beta, -alpha, -color, depth - 1);
        	undo_move(pos, it);
        	
        	/*//r = x ^ ((x ^ y) & -(x < y)); // max(x, y)
		best = best ^ (( best ^ val ) & -( best < val ));
		alpha = alpha ^ (( alpha ^ val ) & -( alpha < val ));
		/*if (alpha >= beta)
			break;*/
			
		if ( val >= beta)
		{
    			delete_movelist( list);
			return val; //fail-soft
		}
			
		if ( val > best)
		{
			best = val;
			if ( val > alpha )
			{
				/*pick[ depth ] = *it;
				*fstpick = *it;
//				pick[ depth ] = *it;
				printf( "*\n");
				printmove( fstpick);
				printf( "***best %d\n", best);*/
				alpha = val;
         			pline->argmove[0] = *it;
         			memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(move));
         			pline->cmove = nline.cmove +1;
			}
		}
		
	}
	delete_movelist( list);
	//printmoves_c( depth);
	return best;
}


U64 mPerft(int depth, board *arg)
{
    move *move_list, *it;
    U64 nodes = 0;
 
    if (depth == 1) return delete_movelist( generate_movesALLOC( *arg));
 
    move_list = generate_movesALLOC( *arg);
    for (it = move_list; it; it = it->next)
    {
        do_move(arg, it);
        nodes += mPerft(depth - 1, arg);
        undo_move(arg, it);
    }
    //
    delete_movelist( move_list);
    //printf("obr %d\n", i);
    
    return nodes;
}

U64 mdivide_perft(int depth, board *arg)
{
    move *move_list, *it;
    U64 childs, nodes = 0;
    
    mfree = 0;
 
    if (depth == 0) return 1;

    move_list = generate_movesALLOC( *arg);
    for (it = move_list; it; it = it->next )
    {
    	childs = 0;
	printmove(it);
        do_move(arg, it);
		//printBits(1, &arg->B);
		//printBits(8, &arg->b.K);
        
        childs += depth > 10 ? mdivide_perft(depth - 1, arg) : mPerft(depth - 1, arg);
        undo_move(arg, it);
	//fr = it;
	printf(" %llu\n", childs);
	nodes += childs;
    }
    delete_movelist(move_list);
    printf("count %llu obr %d\n ", nodes, mfree);
    return nodes;
}

U64 Perft(int depth, board *arg)
{
    int it, movecount;
    U64 nodes = 0;
 
    if (depth == 1) return generate_moves( *arg, &marray[ 216 * depth ]);
 
    movecount = generate_moves( *arg, &marray[ 216 * depth ]);
    for (it = 216 * depth; it < 216*depth + movecount; it++)
    {
        do_move(arg, &marray[it]);
        nodes += Perft(depth - 1, arg);
        undo_move(arg, &marray[it]);
    }
    //
    //printf("obr %d\n", i);
    
    return nodes;
}

U64 divide_perft(int depth, board *arg)
{
    move *move_list;
    int it;
    char movecount;
    U64 childs, nodes = 0;
    
    mfree = 0;
 
    if (depth == 0) return 1;

    movecount = generate_moves( *arg, &marray[ 216 * depth ]);
    for (it = 216 * depth; it < 216*depth + movecount; it++)
    {
    	childs = 0;
	printmove(&marray[it]);
        do_move(arg, &marray[it]);
		//printBits(1, &arg->B);
		//printBits(8, &arg->b.K);
        
        childs += Perft(depth - 1, arg);
        undo_move(arg, &marray[it]);
	//fr = it;
	printf(" %llu\n", childs);
	nodes += childs;
    }
    printf("count %llu obr %d\n ", nodes, mfree);
    return nodes;
}

U64 NPerft(int depth, Nboard *arg, Nmovelist *ml)
{
    unsigned char movecount, capt, quiet, it;
    Nmove ff;
    U64 nodes = 0;
 
    //if (depth == 0) return 1;
    if (depth == 1) return generate_movesN( &ml[depth], *arg);
 
    movecount = generate_movesN( &ml[depth], *arg);
    quiet = ml[depth].quietcount;
    capt = ml[depth].captcount;
    //capt++;
    for (it = 255; it > capt && !stop; it--)
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
    unsigned char movecount, capt, quiet, it;
    //Nmove ff;
    U64 childs, nodes = 0;
    
    mfree = 0;
 
    //if (depth == 0) return generate_movesN( &ml[depth], *arg);

    movecount = generate_movesN( &ml[depth], *arg);
    quiet = ml[depth].quietcount;
    capt = ml[depth].captcount;
    //capt++;
    //it = capt;
    //for (it = 0; (it < quiet) || (it > ml[depth].captcount); it++)
    //for (it = 0; (it < 20) ; it++)
    for (it = 255; it > capt && !stop; it--)
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
    printf("count %llu obr %d\n ", nodes, mfree);
    return nodes;
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


