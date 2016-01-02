#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Search-1.h"
#include "MoveGeneration-1.h"
#include "TranspositionTable-1.h"

move_1 *marray = NULL;
line pline;

int eval( board_1 *b)
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

int mnegamax( board_1 *pos, line *pline, int alpha, int beta, int color, int depth)
{
	int val, best;
	move_1 *list, *it;
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
				memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(move_1));
				pline->cmove = nline.cmove +1;
			}
		}
	}
	delete_movelist( list);
	return best;
}

int anegamax( board_1 *pos, line *pline, int alpha, int beta, int color, int depth)
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
				memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(move_1));
				pline->cmove = nline.cmove +1;
			}
		}
	}
	return best;
}

int mTTnegamax( board_1 *pos, line *pline, int alpha, int beta, int color, int depth)
{
	int best, val, old_alpha;
	move_1 *list, *it, *pick = NULL;
	line nline;
	TTentry *entry;

	entry = TTlookup( pos->zobrist);
	if (entry)
		if (entry->depth >= depth)
		{
			val = entry->score;
			//exact
			if	(entry->flag == 1)
				return val;
			//lowerbound
			if	(entry->flag == 2)
			{
				if ( val >= beta ) return val;
				if ( val > alpha ) alpha = val;
			}
			//higherbound
			if	(entry->flag == 3)
			{
				if ( val <= alpha ) return val;
				if ( val < beta ) beta = val;
			}
		}

	if ( !depth ) 
	{
		return color * eval( pos);
	}

	old_alpha = alpha;
	best = -WIN;
	list = generate_movesALLOC( *pos);

	for (it = list; it; it = it->next )
	{
		do_move(pos, it);
		val = -mTTnegamax( pos, &nline, -beta, -alpha, -color, depth - 1);
		undo_move(pos, it);
		//fail high implies lowerbound
		if ( val >= beta)
		{
			TTstore( pos->zobrist, it, depth, val, 2);
			delete_movelist( list);
			return val; //fail-soft
		}
		if ( val > best)
		{
			best = val;
			if ( val > alpha )
			{
				alpha = val;
				pick = it;
			}
		}
	}
	// exact score
	if (best > old_alpha)  
	{
		TTstore( pos->zobrist, pick, depth, best, 1);

	}
	// fail low implies upperbound
	if (best <= old_alpha)
	{
		TTstore( pos->zobrist, NULL, depth, best, 3);
	}

	delete_movelist( list);
	return best;
}

int aTTnegamax( board_1 *pos, line *pline, int alpha, int beta, int color, int depth)
{
	int best, val, old_alpha, movecount, it;
	move_1 *pick = NULL;
	line nline;
	TTentry *entry;

	entry = TTlookup( pos->zobrist);
	if (entry)
	{
		if (entry->depth >= depth)
		{
			//hash move
			val = entry->score; 
			//exact
			if	(entry->flag == 1)	return val;
			//lowerbound
			if	(entry->flag == 2)
			{
				if ( val >= beta ) return val;
				if ( val > alpha ) alpha = val;
			}
			//higherbound
			if	(entry->flag == 3)
			{
				if ( val <= alpha ) return val;
				if ( val < beta ) beta = val;
			}
		}
	}
	if ( !depth ) 
	{
		return color * eval( pos);
	}

	old_alpha = alpha;
	best = -WIN;
	movecount = generate_moves( *pos, &marray[ 216 * depth ]);

	for (it = 216*depth + movecount - 1; it >= 216*depth; it--)
	{
		do_move(pos, &marray[it]);
		val = -aTTnegamax( pos, &nline, -beta, -alpha, -color, depth - 1);
		undo_move(pos, &marray[it]);
		//fail high implies lowerbound
		if ( val >= beta)
		{
			TTstore( pos->zobrist, &marray[it], depth, val, 2);
			return val; //fail-soft
		}

		if ( val > best)
		{
			best = val;
			if ( val > alpha )
			{
				alpha = val;
				pick = &marray[it];
			}
		}
	}
	// exact score
	if (best > old_alpha)  
	{
		TTstore( pos->zobrist, pick, depth, best, 1);
	}
	// fail low implies upperbound
	if (best <= old_alpha)
	{
		TTstore( pos->zobrist, NULL, depth, best, 3);
	}
	return best;
}

int rootnegamax( board_1 *pos, line *pline, int alpha, int beta, int color, int depth)
{
	int best, val;
	move_1 *list, *it;
	line nline;

	if ( !depth ) return 0;

	best = -WIN;
	list = generate_movesALLOC( *pos);

	for (it = list; it; it = it->next )
	{

		do_move(pos, it);
		val = -anegamax( pos, &nline, -beta, -alpha, -color, depth - 1);
		undo_move(pos, it);

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
				memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(move_1));
				pline->cmove = nline.cmove +1;
			}
		}
	}
	delete_movelist( list);
	return best;
}


U64 mPerft(int depth, board_1 *arg)
{
	move_1 *move_list, *it;
	U64 nodes = 0;

	if (depth == 1) return delete_movelist( generate_movesALLOC( *arg));

	move_list = generate_movesALLOC( *arg);
	for (it = move_list; it; it = it->next)
	{
		do_move(arg, it);
		nodes += mPerft(depth - 1, arg);
		undo_move(arg, it);
	}
	delete_movelist( move_list);
	return nodes;
}

U64 mdivide_perft(int depth, board_1 *arg)
{
	move_1 *move_list, *it;
	U64 childs, nodes = 0;

	mfree = 0;

	if (depth == 0) return 1;

	move_list = generate_movesALLOC( *arg);
	for (it = move_list; it; it = it->next )
	{
		childs = 0;
		printmove(it);
		do_move(arg, it);

		childs += depth > 10 ? mdivide_perft(depth - 1, arg) : mPerft(depth - 1, arg);
		undo_move(arg, it);
		printf(" %llu\n", childs);
		nodes += childs;
	}
	delete_movelist(move_list);
	printf("count %llu obr %llun ", nodes, mfree);
	return nodes;
}

U64 Perft(int depth, board_1 *arg)
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
	return nodes;
}

U64 divide_perft(int depth, board_1 *arg)
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

		childs += Perft(depth - 1, arg);
		undo_move(arg, &marray[it]);
		printf(" %llu\n", childs);
		nodes += childs;
	}
	printf("count %llu obr %llu\n ", nodes, mfree);
	return nodes;
}

int delete_movelist(move_1 *arg)
{
	move_1 *d; 
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
