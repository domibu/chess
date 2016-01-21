#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "Search.h"
#include "MoveGeneration.h"
#include "TranspositionTable.h"

extern int en_state;

line Npline;
int stop = 0;

#define QUIESCE_DEPTH	-38
int Quiesce( board *pos, line *pline, int alpha, int beta, int color, int depth, int draft)
{
	int score;
	int stand_pat = color*evaluate(*pos, draft, color, pos);

	if (depth < QUIESCE_DEPTH)
	{
		Print(0,"***QUIESCE_DEPTH*** reached\n");
		return stand_pat;
	}

	if ( stand_pat >= beta)	return beta;
	if ( alpha < stand_pat )	alpha = stand_pat;

	unsigned capt_count;
	generate_moves(&NML[draft+1], *pos);
	capt_count = NML[draft+1].captcount;
	int it;

	for (it = 218; (it < capt_count) && (en_state == THINKING); it++)
	{
		do_move(pos, NML[draft+1].mdata[it]);
		score = -Quiesce( pos, pline, -beta, -alpha, -color, depth-1, draft+1);
		undo_move(pos, &NML[draft+1], NML[draft+1].mdata[it]);

		if( score >= beta )	return beta;
		if( score > alpha )
		{
			alpha = score;
		}
	}
	return alpha;
}

int search( board *pos, line *pline, int alpha, int beta, int color, int depth, int draft)
{
	int best, val, old_alpha, it, limes;
	move pick = NULL, hash_move = 0, do_move_1;
	unsigned char quiet, capt, flag;
	line nline;
	TTentry *entry;

	entry = lookup_TT( pos->zobrist);
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
		return Quiesce( pos, pline, alpha, beta, color, depth, draft);
	}

	old_alpha = alpha;
	best = -WIN-300;
	generate_moves(&NML[depth] , *pos);
	//if (hash_move > 0) sortmoves( &NML[depth], hash_move);
	quiet = NML[depth].quietcount;
	capt = NML[depth].captcount;

	it = (capt > 218) ? 218 : 0;
	limes = (capt > 218) ? capt : quiet;

forpetlja:
	for (; (it < limes) && (en_state == THINKING); it++  )
	{
		do_move_1 = NML[depth].mdata[it];
		do_move(pos, do_move_1);
		val = -search( pos, &nline, -beta, -alpha, -color, depth - 1, draft + 1);
		undo_move(pos, &NML[depth], do_move_1);

		if ( val >= beta)
		{
			U64 data = 0ULL;
			data ^= (short)val;
			data <<= 48;
			data ^= do_move_1;
			data ^= (U64)depth << 32;
			data ^= (2ULL << 40);
			store_TT( pos->zobrist, data);
			return val; //fail-soft
		}
		if ( val > best)
		{
			best = val;
			if ( val > alpha )
			{
				alpha = val;
				pick = NML[depth].mdata[it];
			}
		}
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
		store_TT( pos->zobrist, data);
	}
	// fail low implies upperbound
	if (best <= old_alpha)
	{
		U64 data = 0ULL;
		data ^= (short)best;
		data <<= 48;
		data ^= (U64)depth << 32;
		data ^= (3ULL << 40);
		store_TT( pos->zobrist, data);
	}
	return best;
}

int neval( board *b)
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

int nnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth, int draft)
{
	int best, val, it, limes;
	unsigned char quiet, capt;
	line nline;

	if ( !depth )
	{
		pline->cmove = 0;
		return color*evaluate( *pos, draft, color, pos);
	}
	best = -WIN-300;

	generate_moves(&NML[depth] , *pos);
	quiet = NML[depth].quietcount;
	capt = NML[depth].captcount;

	it = (capt > 218) ? 218 : 0;
	limes = (capt > 218) ? capt : quiet;

forpetlja:
	for (; it < limes ; it++  )
	{
		do_move(pos, NML[depth].mdata[it]);
		val = -nnegamax( pos, &nline, -beta, -alpha, -color, depth - 1, draft +1);
		undo_move(pos, &NML[depth], NML[depth].mdata[it]);

		if ( val >= beta)       return val; //fail-soft
		if ( val > best)
		{
			best = val;
			if ( val > alpha )
			{
				alpha = val;
				pline->argmove[0] = NML[depth].mdata[it];
				memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(move));
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

int  pvs_02(board *pos, line *pline, int alpha, int beta, int color, int depth, int draft)
{
	int best, val, old_alpha, it, limes;
	move pick = NULL, hash_move = 0;
	unsigned char quiet, capt, flag;
	line nline;
	TTentry *entry;

	entry = lookup_TT( pos->zobrist);
	if (entry)
	{
		if (((entry->data >> 32)  & 0x0000000000FF) >= depth)
		{
			val = (short)(entry->data >> 48);
			flag = entry->data >> 40 & 0x000000000000003;
			//exact
			if	(flag == 1)      return val;
			//lowerbound
			if	(flag == 2)
			{
				if ( val >= beta ) return val;
				if ( val > alpha ) alpha = val;
			}
			//higherbound
			if	(flag == 3)
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
		return color*evaluate( *pos , draft, color, pos);
	}

	old_alpha = alpha;
	best = -WIN-300;
	generate_moves(&NML[depth] , *pos);
	//if (hash_move > 0) sortmoves( &NML[depth], hash_move);
	quiet = NML[depth].quietcount;
	capt = NML[depth].captcount;

	it = (capt > 218) ? 218 : 0;
	limes = (capt > 218) ? capt : quiet;

forpetlja:
	for (; it < limes ; it++  )
	{
		do_move(pos, NML[depth].mdata[it]);
		val = -pvs_02( pos, &nline, -beta, -alpha, -color, depth - 1, draft + 1);
		undo_move(pos, &NML[depth], NML[depth].mdata[it]);

		if ( val >= beta)
		{
			U64 data = 0ULL;
			data ^= (short)val;
			data <<= 48;
			data ^= NML[depth].mdata[it];
			data ^= (U64)depth << 32;
			data ^= (2ULL << 40);
			store_TT( pos->zobrist, data);
			return val; //fail-soft
		}
		if ( val > best)
		{
			best = val;
			if ( val > alpha )
			{
				alpha = val;
				pick = NML[depth].mdata[it];
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
		data ^= (short)best;
		data <<= 48;
		data ^= pick;
		data ^= (U64)depth << 32;
		data ^= (1ULL << 40);
		store_TT( pos->zobrist, data);
	}
	// fail low implies upperbound
	if (best <= old_alpha)
	{
		U64 data = 0ULL;
		data ^= (short)best;
		data <<= 48;
		data ^= (U64)depth << 32;
		data ^= (3ULL << 40);
		store_TT( pos->zobrist, data);
	}
	return best;
}

//BUG
void sortmoves(node_move_list *m_list, move PV_move)
{
	int start= (m_list->captcount > 218) ? 218 : 0;
	int limes = (m_list->captcount > 218) ? m_list->captcount : m_list->quietcount;
	int it = start;
	unsigned piece, capt;
sortforpetlja:
	for (; it < limes ; it++  )
	{
		if (PV_move > 0)
			if (m_list->mdata[it] == PV_move)	{
				m_list->mdata[it] = m_list->mdata[start];
				m_list->mdata[start] = PV_move;
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

int pvs_01( board *pos, line *pline, int alpha, int beta, int color, int depth, int is_PV, int draft)
{
	int best, val, it, limes;
	unsigned char quiet, capt;
	line nline;

	if ( !depth )
	{
		pline->cmove = 0;
		return color * neval( pos);
	}

	best = -WIN;
	generate_moves(&NML[depth] , *pos);
	quiet = NML[depth].quietcount;
	capt = NML[depth].captcount;

	it = (capt > 218) ? 218 : 0;
	limes = (capt > 218) ? capt : quiet;

forpetlja:
	for (; it < limes ; it++  )
	{
		do_move(pos, NML[depth].mdata[it]);
		val = -pvs_01( pos, &nline, -beta, -alpha, -color, depth - 1, is_PV, draft + 1);
		undo_move(pos, &NML[depth], NML[depth].mdata[it]);

		if ( val >= beta)       return val; //fail-soft
		if ( val > best)
		{
			best = val;
			if ( val > alpha )
			{
				alpha = val;
				pline->argmove[0] = NML[depth].mdata[it];
				memcpy( pline->argmove + 1, nline.argmove, nline.cmove * sizeof(move));
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

int nTTnegamax( board *pos, line *pline, int alpha, int beta, int color, int depth)
{
	int best, val, old_alpha, it, limes;
	move pick = NULL;
	unsigned char quiet, capt, flag;
	line nline;
	TTentry *entry;

	entry = lookup_TT( pos->zobrist);
	if (entry)
		if (((entry->data >> 32)  & 0x0000000000FF) >= depth)
		{
			val = (short)(entry->data >> 48);
			flag = entry->data >> 40 & 0x000000000000003;
			//exact
			if	(flag == 1)      return val;
			//lowerbound
			if	(flag == 2)
			{
				if ( val >= beta ) return val;
				if ( val > alpha ) alpha = val;
			}
			//higherbound
			if	(flag == 3)
			{
				if ( val <= alpha ) return val;
				if ( val < beta ) beta = val;
			}
		}

	if ( !depth )
	{
		return color * neval( pos);
	}

	old_alpha = alpha;
	best = -WIN;
	generate_moves(&NML[depth] , *pos);
	quiet = NML[depth].quietcount;
	capt = NML[depth].captcount;

	it = (capt > 218) ? 218 : 0;
	limes = (capt > 218) ? capt : quiet;

forpetlja:
	for (; it < limes ; it++  )
	{
		do_move(pos, NML[depth].mdata[it]);
		val = -nTTnegamax( pos, &nline, -beta, -alpha, -color, depth - 1);
		undo_move(pos, &NML[depth], NML[depth].mdata[it]);

		if ( val >= beta)
		{
			U64 data = 0ULL;
			data ^= (short)val;
			data <<= 48;
			data ^= NML[depth].mdata[it];
			data ^= (U64)depth << 32;
			data ^= (2ULL << 40);
			store_TT( pos->zobrist, data);
			return val; //fail-soft
		}

		if ( val > best)
		{
			best = val;
			if ( val > alpha )
			{
				alpha = val;
				pick = NML[depth].mdata[it];
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
		data ^= (short)best;
		data <<= 48;
		data ^= pick;
		data ^= (U64)depth << 32;
		data ^= (1ULL << 40);
		store_TT( pos->zobrist, data);

	}
	// fail low implies upperbound
	if (best <= old_alpha)
	{
		U64 data = 0ULL;
		data ^= (short)best;
		data <<= 48;
		data ^= (U64)depth << 32;
		data ^= (3ULL << 40);
		store_TT( pos->zobrist, data);
	}
	return best;
}

U64 NPerft(int depth, board *arg, node_move_list *ml)
{
	unsigned char capt, quiet, it;
	U64 nodes = 0;

	//if (depth == 0) return 1;
	if (depth == 1) return generate_moves( &ml[depth], *arg);

	generate_moves( &ml[depth], *arg);
	quiet = ml[depth].quietcount;
	capt = ml[depth].captcount;
	for (it = 218; it < capt && !stop; it++)
	{
		do_move(arg, ml[depth].mdata[it]);
		nodes += NPerft(depth - 1, arg, ml);
		undo_move(arg, &ml[depth], ml[depth].mdata[it]);
	}
	for (it = 0; it < quiet && !stop; it++)
	{
		do_move(arg, ml[depth].mdata[it]);
		nodes += NPerft(depth - 1, arg, ml);
		undo_move(arg, &ml[depth], ml[depth].mdata[it]);
	}
	return nodes;
}

U64 Ndivide_perft(int depth, board *arg, node_move_list *ml)
{
	unsigned char capt, quiet, it;
	U64 childs, nodes = 0;
	mfree = 0;

	generate_moves( &ml[depth], *arg);
	quiet = ml[depth].quietcount;
	capt = ml[depth].captcount;
	for (it = 218; it < capt && !stop; it++)
	{
		childs = 0;
		Print(0, "%s", print_smith_notation(&ml[depth].mdata[it]));
		do_move(arg, ml[depth].mdata[it]);

		childs += NPerft(depth - 1, arg, ml);
		Print(0, " %llu\n", childs);
		undo_move(arg, &ml[depth], ml[depth].mdata[it]);
		nodes += childs;
	}
	for (it = 0; it < quiet && !stop; it++)
	{
		childs = 0;
		Print(0, "%s", print_smith_notation(&ml[depth].mdata[it]));
		do_move(arg, ml[depth].mdata[it]);

		childs += NPerft(depth - 1, arg, ml);
		Print(0, " %llu\n", childs);
		undo_move(arg, &ml[depth], ml[depth].mdata[it]);
		nodes += childs;
	}
	return nodes;
}
