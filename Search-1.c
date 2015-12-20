#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Search-1.h"
#include "MoveGeneration-1.h"
#include "TranspositionTable-1.h"

move *marray = NULL;
line pline;

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

//node **arg, int dubina, int alfa, int beta, atom *igracset, atom *protivnikset, pozicija **pz, int player, int *izbor
int mnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth)
{
	int val, best;
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
		if (alpha >= beta)
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

int anegamax( board *pos, line *pline, int alpha, int beta, int color, int depth)
{
	int best, val, it;
	line nline;

	if ( !depth ) 
  	{
    		pline->cmove = 0;
	  return color * eval( pos);
  	}
  	
	best = -WIN;
        unsigned movecount = generate_moves( *pos, &marray[ 216 * depth ]);
        for (it = 216*depth + movecount - 1; it >= 216*depth; it--)
   	{

        	do_move(pos, &marray[it]);
	        val = -anegamax( pos, &nline, -beta, -alpha, -color, depth - 1);
        	undo_move(pos, &marray[it]);
        	
        	/*//r = x ^ ((x ^ y) & -(x < y)); // max(x, y)
		best = best ^ (( best ^ val ) & -( best < val ));
		alpha = alpha ^ (( alpha ^ val ) & -( alpha < val ));
		if (alpha >= beta)
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
	move *list, *it, *pick = NULL;
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
		if (alpha >= beta)
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
			//bm = it;
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

int aTTnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth)
{
	int best, val, old_alpha, movecount, it;
	move *pick = NULL;
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
	        val = -aTTnegamax( pos, &nline, -beta, -alpha, -color, depth - 1);
        	undo_move(pos, &marray[it]);
        	
        	/*//r = x ^ ((x ^ y) & -(x < y)); // max(x, y)
		best = best ^ (( best ^ val ) & -( best < val ));
		alpha = alpha ^ (( alpha ^ val ) & -( alpha < val ));
		if (alpha >= beta)
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
			//bm = &marray[it];
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
	        val = -anegamax( pos, &nline, -beta, -alpha, -color, depth - 1);
        	undo_move(pos, it);
        	
        	/*//r = x ^ ((x ^ y) & -(x < y)); // max(x, y)
		best = best ^ (( best ^ val ) & -( best < val ));
		alpha = alpha ^ (( alpha ^ val ) & -( alpha < val ));
		if (alpha >= beta)
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
    printf("count %llu obr %llun ", nodes, mfree);
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
    printf("count %llu obr %llu\n ", nodes, mfree);
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
