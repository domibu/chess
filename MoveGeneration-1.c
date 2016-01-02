#include <stdio.h>
#include <stdlib.h>
#include "BitBoardMagic.h"
#include "MoveGeneration-1.h"
#include "Search-1.h"

// a definition using static inline
static inline int max(int a, int b) {
	return a > b ? a : b;
}

move *generate_movesALLOC(board_1 arg)
{
	U64 ppb, ppr, bb, at_b, at_r, blank = 0LL, all_pp, mpb, mpr, piece_type, capture, stm, enp;
	U64 check_pieces, check_grid = ~0LL;
	U64 in_N, in_B, in_R, in_Q, in_K, in;
	U64 mN_q, mN_c, mB_q, mB_c, mR_q, mR_c, mQ_q, mQ_c, mK_q, mK_c;
	U64 mP1, mP2, mPE, mPW, mENP, mP1_prom, mPE_prom, mPW_prom, it_prom;
	U64 cas_bit_K, cas_bit_Q, cas_at_K, cas_at_Q, cas_occ_K, cas_occ_Q, promotion, ENProw, enp_P;
	U64 f, mask;
	piece_set *fr, *ho; 
	int in_at_b, in_at_r, king, in_at, in_k, at, pp, mpp, P1, P2, PE, PW;
	move *m_list = NULL, *m;

	stm = (arg.info & 1LL) << 24;
	if (stm )
	{
		fr = &arg.w;
		ho = &arg.b;
		cas_bit_K = 0x0000000000000002;
		cas_bit_Q = 0x0000000000000004;
		cas_at_K =  0x000000000000000E;
		cas_at_Q =  0x0000000000000038;
		cas_occ_K = 0x0000000000000006;
		cas_occ_Q = 0x0000000000000070;
		promotion = 0xFF00000000000000;
		P1 = 8;
		P2 = 16;
		PE = 7;
		PW = 9;
		enp = (arg.info & 0x000000000000FF00) << 32;
		enp_P = enp >> 8;
		ENProw = 0x000000FF00000000;
	}
	else
	{
		fr = &arg.b;
		ho = &arg.w;
		cas_bit_K = 0x0000000000000008;
		cas_bit_Q = 0x0000000000000010;
		cas_at_K =  0x0E00000000000000;
		cas_at_Q =  0x3800000000000000;
		cas_occ_K = 0x0600000000000000;
		cas_occ_Q = 0x7000000000000000;
		promotion = 0x00000000000000FF;
		P1 = -8;
		P2 = -16;
		PE = -9;
		PW = -7;
		enp = (arg.info & 0x000000000000FF00) << 8;
		enp_P = enp << 8;
		ENProw = 0x00000000FF000000;
	}

	king = __builtin_ffsll(fr->K)-1;
	//!!!!!!!!!!!!!!! building atack map pieces, later wont be necessary becuase it will be updated incrementally by make, unmake move!!!!!!!!!!!!!!!!!!!!!!!
	fr->pieces = fr->P ^ fr->N ^ fr->R ^ fr->B ^ fr->K ^ fr->Q;
	ho->pieces = ho->P ^ ho->N ^ ho->R ^ ho->B ^ ho->K ^ ho->Q;
	arg.all_p = fr->pieces ^ ho->pieces;
	ho->atack = gen_ho_atack(arg);
	// check if king is in CHECK
	if (fr->K & ho->atack)
	{
		if (stm)	in_K = (fr->K & 0xFEFEFEFEFEFEFEFE) << 7 & ho->P | (fr->K & 0x7F7F7F7F7F7F7F7F) << 9 & ho->P;  		
		else	in_K = (fr->K & 0xFEFEFEFEFEFEFEFE) >> 9 & ho->P | (fr->K & 0x7F7F7F7F7F7F7F7F) >> 7 & ho->P;  		

		in_at_b = ( (arg.all_p & occupancyMaskBishop[king]) * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho->B ^ ho->Q); //friendly pieces<$1>
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.all_p & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho->R ^ ho->Q); //friendly pieces<$1>

		in_N = movesNight[king] & ho->N;

		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.all_p & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); //friendly pieces<$1>
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.all_p & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
				//index of king can be outside of while loop	
				check_grid = magicMovesRook[king][in_at_r] & magicMovesRook[at][in_at] | (1LL << at);

			}
			else if (__builtin_popcountll(in_N))
			{
				check_grid = in_N;
			}
			else if (__builtin_popcountll(in_K))
			{
				check_grid = in_K;
			}

		}
		else check_grid = 0LL;
	}
	all_pp = 0LL;
	ppb = 0LL;
	ppr = 0LL;

	// looking for pieces pinned diagonaly 
	in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
	at_b = magicMovesBishop[king][in_at_b] & (ho->B ^ ho->Q); //friendly pieces<$1>
	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.all_p & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index of king can be outside of while loop	
		in_k = ((arg.all_p & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		ppb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at];
		all_pp ^= ppb;

		pp = __builtin_ffsll(ppb)-1;
		bb = arg.all_p & ~ (1LL << pp);
		in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		in_k = ((bb & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		mpb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at] &  ~ppb ^ (1LL << at); 
		mpb &= check_grid;

		if (__builtin_popcountll( (fr->Q ^ fr->B) & ppb )) // probably doesn't need popcount
		{
			while ( __builtin_popcountll(mpb))
			{
				mpp = __builtin_ffsll(mpb)-1;
				if (fr->B & ppb)	piece_type = (1LL << 3);
				else piece_type = (1LL << 1);
				//checking for check?<$1>
				capture = (1LL << mpp) & ho->pieces ? 1LL << 6 : 0LL;					
				//new move pp on mpp
				m = m_list;
				m_list = malloc(sizeof(move));
				m_list->next = m;
				m_list->from = ppb;
				m_list->dest = (1LL << mpp);
				m_list->info = piece_type ^ capture ^ stm; // check lacks?
				//determine captured piece
				f = (ho->Q >> mpp) & 1LL;
				mask = 1LL << 25;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				f = (ho->R >> mpp) & 1LL;
				mask <<= 1;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				f = (ho->B >> mpp) & 1LL;
				mask <<= 1;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				f = (ho->N >> mpp) & 1LL;
				mask <<= 1;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				f = (ho->P >> mpp) & 1LL;
				mask <<= 1;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				//save enp_sq, castle flags,<$1>
				m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				mpb &= ~m_list->dest; 
			}
		}
		//check for pawn
		if ( fr->P & ppb)
		{
			pp = __builtin_ffsll(ppb)-1;  //probably possible to use pp from above because one at has one pinned_p
			if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho->pieces & ~promotion & check_grid);
			else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho->pieces & ~promotion & check_grid );
			// add limit for a,h file depending of direction of capture, possible that limits aren't needed
			if (mpp)
			{
				// without PROMOTION
				m = m_list;
				m_list = malloc(sizeof(move));
				m_list->next = m;
				m_list->from = ppb;
				m_list->dest = (1LL << (mpp-1));
				m_list->info = (1LL << 5) ^ (1LL << 6) ^ stm;//check lacks write as const except stm
				//determine captured piece
				f = (ho->Q >> (mpp-1)) & 1LL;
				mask = 1LL << 25;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				f = (ho->R >> (mpp-1)) & 1LL;
				mask <<= 1;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				f = (ho->B >> (mpp-1)) & 1LL;
				mask <<= 1;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				f = (ho->N >> (mpp-1)) & 1LL;
				mask <<= 1;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				f = (ho->P >> (mpp-1)) & 1LL;
				mask <<= 1;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				//save enp_sq, castle flags<1$>
				m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				mpb &= ~m_list->dest; //do we really need that?<1$>
			}
			else 
			{
				//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>
				if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho->pieces & promotion & check_grid);
				else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho->pieces & promotion & check_grid );
				if (mpp)
				{
					//promotion 
					for (it_prom = 0x0000000000000100; it_prom ^ 0x0000000000001000; it_prom <<= 1)
					{
						in = __builtin_ffsll(mPW_prom)-1;
						m = m_list;
						m_list = malloc(sizeof(move));
						m_list->next = m;
						m_list->from = ppb;
						m_list->dest = (1LL << (mpp-1));
						m_list->info = (1LL << 5) ^ (1LL << 6) ^ (1LL << 15) ^ it_prom ^ stm;
						//determine captured piece<$1>
						f = (ho->Q >> (mpp-1)) & 1LL;
						mask = 1LL << 25;
						m_list->info = (m_list->info & ~mask) | ( -f & mask);
						f = (ho->R >> (mpp-1)) & 1LL;
						mask <<= 1;
						m_list->info = (m_list->info & ~mask) | ( -f & mask);
						f = (ho->B >> (mpp-1)) & 1LL;
						mask <<= 1;
						m_list->info = (m_list->info & ~mask) | ( -f & mask);
						f = (ho->N >> (mpp-1)) & 1LL;
						mask <<= 1;
						m_list->info = (m_list->info & ~mask) | ( -f & mask);
						f = (ho->P >> (mpp-1)) & 1LL;
						mask <<= 1;
						m_list->info = (m_list->info & ~mask) | ( -f & mask);
						//save enp_sq, castle flags<1$>
						m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					}
					//check is missing, write as const except stm<1$>
					mpb &= ~m_list->dest; //do we really need that?<1$>
				}
				else
				{
					if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho->pieces^enp) & check_grid);
					else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho->pieces ^ enp) & check_grid);
					if (mpp)
					{
						//promotion excluded when en'passan<1$>
						capture = 1LL << 6;
						m = m_list;
						m_list = malloc(sizeof(move));
						m_list->next = m;
						m_list->from = ppb;
						m_list->dest = (1LL << (mpp-1));
						m_list->info = (1LL << 5) ^ (1LL << 6) ^ (1LL << 12) ^ stm;
						//check is missing, write as const except stm<1$>
						//determine captured piece<$1>
						f = (ho->Q >> (mpp-1)) & 1LL;
						mask = 1LL << 25;
						m_list->info = (m_list->info & ~mask) | ( -f & mask);
						f = (ho->R >> (mpp-1)) & 1LL;
						mask <<= 1;
						m_list->info = (m_list->info & ~mask) | ( -f & mask);
						f = (ho->B >> (mpp-1)) & 1LL;
						mask <<= 1;
						m_list->info = (m_list->info & ~mask) | ( -f & mask);
						f = (ho->N >> (mpp-1)) & 1LL;
						mask <<= 1;
						m_list->info = (m_list->info & ~mask) | ( -f & mask);
						f = (ho->P >> (mpp-1)) & 1LL;
						mask <<= 1;
						m_list->info = (m_list->info & ~mask) | ( -f & mask);
						//save enp_sq, castle flags<1$>
						m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
						mpb &= ~m_list->dest; //do we really need that?<1$>
					}
				}
			}
		}
		//checking for pawn<1$>
		at_b &= ~(1LL << at); 
	}

	//looking for pieces pinned by file or rank<1$>
	in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
	at_r = magicMovesRook[king][in_at_r] & (ho->R ^ ho->Q); //friendly pieces<$1>
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.all_p & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.all_p & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
		all_pp ^= ppr;

		pp = __builtin_ffsll(ppr)-1;
		bb = arg.all_p & ~ (1LL << pp);
		in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		in_k = ((bb & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		mpr = magicMovesRook[king][in_k] & magicMovesRook[at][in_at] &  ~ppr ^ (1LL << at); 
		mpr &= check_grid;

		if (__builtin_popcountll( (fr->Q ^ fr->R) & ppr )) 
		{
			while ( __builtin_popcountll(mpr))
			{
				mpp = __builtin_ffsll(mpr)-1;
				if (fr->R & ppr)	piece_type = (1LL << 2);
				else piece_type = (1LL << 1);
				//checking for check?<$1>
				capture = (1LL << mpp) & ho->pieces ? 1LL << 6 : 0LL;					
				//new move pp on mpp<$1>
				m = m_list;
				m_list = malloc(sizeof(move));
				m_list->next = m;
				m_list->from = ppr;
				m_list->dest = (1LL << mpp);
				m_list->info = piece_type ^ capture ^ stm; 
				//determine captured piece<$1>
				f = (ho->Q >> mpp) & 1LL;
				mask = 1LL << 25;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				f = (ho->R >> mpp) & 1LL;
				mask <<= 1;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				f = (ho->B >> mpp) & 1LL;
				mask <<= 1;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				f = (ho->N >> mpp) & 1LL;
				mask <<= 1;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				f = (ho->P >> mpp) & 1LL;
				mask <<= 1;
				m_list->info = (m_list->info & ~mask) | ( -f & mask);
				//save enp_sq, castle flags<1$>
				m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				mpr &= ~m_list->dest; 
			}
		}
		//checking for pawn<1$>
		if ( fr->P & ppr)
		{
			pp = __builtin_ffsll(ppr)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
			if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.all_p) << 8) & ~arg.all_p & check_grid);
			else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.all_p) >> 8) & ~arg.all_p & check_grid);//include just two rank step<1$>
			//pawn advance or pawn capture<1$>
			if (mpp)
			{
				m = m_list;
				m_list = malloc(sizeof(move));
				m_list->next = m;
				m_list->from = ppr;
				m_list->dest = (1LL << (mpp-1));
				//check is missing, write as const except stm<1$>
				//save enp_sq, castle flags<1$>
				m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;

				mpb &= ~m_list->dest; 
			}
			{
				mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.all_p) & check_grid
					: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.all_p) & check_grid;//include just one rank step<1$>
				if (mpp)
				{
					//promotion excluded when en'passan<1$>
					m = m_list;
					m_list = malloc(sizeof(move));
					m_list->next = m;
					m_list->from = ppr;
					m_list->dest = (1LL << (mpp-1));
					m_list->info = (1LL << 5) ^ (1LL << 15) ^ stm;
					//check is missing, write as const except stm<1$>
					//save enp_sq, castle flags<1$>
					m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					mpb &= ~m_list->dest; 
				}
			}
		}
		at_r &= ~(1LL << at); 
	}
	//legal moves<1$>
	//PAWN
	fr->P &= ~ all_pp;
	if (stm)
	{
		mP2 = ((fr->P & 0x000000000000FF00) << 8 & ~arg.all_p) << 8 & ~arg.all_p & check_grid;
		mP1 = fr->P << 8 & ~arg.all_p & check_grid;
		mPE = (fr->P & 0xFEFEFEFEFEFEFEFE) << 7 & ho->pieces & check_grid;
		mPW = (fr->P & 0x7F7F7F7F7F7F7F7F) << 9 & ho->pieces & check_grid;
		mENP = (enp & 0xFEFEFEFEFEFEFEFE & check_grid) >> 9 & fr->P ^ (enp & 0x7F7F7F7F7F7F7F7F & check_grid) >> 7 & fr->P;
		mENP |= (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr->P ^ (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr->P;
		mP1_prom = mP1 & promotion;
		mPE_prom = mPE & promotion;
		mPW_prom = mPW & promotion;
		mP1 &= ~promotion;
		mPE &= ~promotion;
		mPW &= ~promotion;
	}
	else 
	{
		mP2 = ((fr->P & 0x00FF000000000000) >> 8 & ~arg.all_p) >> 8 & ~arg.all_p & check_grid;
		mP1 = fr->P >> 8 & ~arg.all_p & check_grid;
		mPE = (fr->P & 0xFEFEFEFEFEFEFEFE) >> 9 & ho->pieces & check_grid;
		mPW = (fr->P & 0x7F7F7F7F7F7F7F7F) >> 7 & ho->pieces & check_grid;
		mENP = (enp & 0x7F7F7F7F7F7F7F7F & check_grid) << 9 & fr->P ^ (enp & 0xFEFEFEFEFEFEFEFE & check_grid) << 7 & fr->P;
		mENP |= (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr->P ^ (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr->P;
		mP1_prom = mP1 & promotion;
		mPE_prom = mPE & promotion;
		mPW_prom = mPW & promotion;
		mP1 &= ~promotion;
		mPE &= ~promotion;
		mPW &= ~promotion;
	}
	while (__builtin_popcountll(mP2))
	{
		in = __builtin_ffsll(mP2)-1;
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->from = 1LL << (in - P2 );
		m_list->dest = 1LL << in;
		m_list->info = (1LL << 5) ^ (1LL << 15) ^ (1LL << (16 + in%8)) ^ stm;
		//save enp_sq, castle flags<1$>
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check is missing, write as const except stm<1$>
		mP2 &= ~m_list->dest; 
	}
	while (__builtin_popcountll(mP1))
	{
		in = __builtin_ffsll(mP1)-1;
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->from = 1LL << (in - P1 );
		m_list->dest = 1LL << in;
		m_list->info = (1LL << 5) ^ (1LL << 15) ^ stm;
		//save enp_sq, castle flags<1$>
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check is missing, write as const except stm<1$>
		mP1 &= ~m_list->dest; 
	}
	while (__builtin_popcountll(mPW))
	{
		in = __builtin_ffsll(mPW)-1;
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->from = 1LL << (in - PW);
		m_list->dest = 1LL << in;
		m_list->info = (1LL << 5) ^ (1LL << 6) ^ stm;
		//determine captured piece<$1>
		f = (ho->Q >> in) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//save enp_sq, castle flags<1$>
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check is missing, write as const except stm<1$>
		mPW &= ~m_list->dest; 
	}
	while (__builtin_popcountll(mPE))
	{
		in = __builtin_ffsll(mPE)-1;
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->from = 1LL << (in - PE);
		m_list->dest = 1LL << in;
		m_list->info = (1LL << 5) ^ (1LL << 6) ^ stm;
		//determine captured piece<$1>
		f = (ho->Q >> in) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//save enp_sq, castle flags<1$>
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check is missing, write as const except stm<1$>
		mPE &= ~m_list->dest; 
	}
	while (__builtin_popcountll(mENP))
	{
		in = __builtin_ffsll(mENP)-1;
		//enp check for 4-5 row pin
		bb = ~(1LL << in | ((enp << 8) >> (16*(stm >> 24)))) & arg.all_p & occupancyMaskRook[king];
		in_K = (bb * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		mR_q = magicMovesRook[king][in_K] & (ho->Q | ho-> R) & ENProw;

		if (!(mR_q))
		{
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = 1LL << in;
			m_list->dest = enp;
			m_list->info = (1LL << 5) ^ (1LL << 12) ^ stm;
			//save enp_sq, castle flags<1$>
			m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
		}
		mENP &= ~(1LL << in); 
	}
	while (__builtin_popcountll(mP1_prom))
	{
		for (it_prom = 0x0000000000000100; it_prom ^ 0x0000000000001000; it_prom <<= 1)
		{
			in = __builtin_ffsll(mP1_prom)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = 1LL << (in - P1 );
			m_list->dest = 1LL << in;
			m_list->info = (1LL << 5) ^ (1LL << 15) ^ it_prom ^ stm;
			//save enp_sq, castle flags<1$>
			m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
		}
		mP1_prom &= ~m_list->dest; 
	}
	while (__builtin_popcountll(mPW_prom))
	{
		for (it_prom = 0x0000000000000100; it_prom ^ 0x0000000000001000; it_prom <<= 1)
		{
			in = __builtin_ffsll(mPW_prom)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = 1LL << (in - PW);
			m_list->dest = 1LL << in;
			m_list->info = (1LL << 5) ^ (1LL << 6) ^ it_prom ^ stm;
			//determine captured piece<$1>
			f = (ho->Q >> in) & 1LL;
			mask = 1LL << 25;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->R >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->B >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->N >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->P >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			//save enp_sq, castle flags<1$>
			m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		}
		//check is missing, write as const except stm<1$>
		mPW_prom &= ~m_list->dest; 
	}
	while (__builtin_popcountll(mPE_prom))
	{
		for (it_prom = 0x0000000000000100; it_prom ^ 0x0000000000001000; it_prom <<= 1)
		{
			in = __builtin_ffsll(mPE_prom)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = 1LL << (in - PE);
			m_list->dest = 1LL << in;
			m_list->info = (1LL << 5) ^ (1LL << 6) ^ it_prom ^ stm;
			//determine captured piece<$1>
			f = (ho->Q >> in) & 1LL;
			mask = 1LL << 25;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->R >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->B >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->N >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->P >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			//save enp_sq, castle flags<1$>
			m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		}
		//check is missing, write as const except stm<1$>
		mPE_prom &= ~m_list->dest; 
	}

	//NIGHT
	fr->N &= ~ all_pp;
	while (__builtin_popcountll(fr->N))
	{
		in_N = __builtin_ffsll(fr->N)-1;
		mN_q = movesNight[in_N] & ~arg.all_p & check_grid;
		mN_c = movesNight[in_N] & ho->pieces & check_grid;

		while (__builtin_popcountll(mN_q))
		{
			in = __builtin_ffsll(mN_q)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_N);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 4) ^ stm;
			//save enp_sq, castle flags<1$>
			m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
			mN_q &= ~m_list->dest; 
		}
		while (__builtin_popcountll(mN_c))
		{
			in = __builtin_ffsll(mN_c)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_N);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 4) ^ (1LL << 6) ^ stm;
			//determine captured piece<$1>
			f = (ho->Q >> in) & 1LL;
			mask = 1LL << 25;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->R >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->B >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->N >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->P >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			//save enp_sq, castle flags<1$>
			m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
			mN_c &= ~m_list->dest; 
		}
		fr->N &= ~1LL << in_N; 
	}
	//BISHOP
	fr->B &= ~ all_pp;
	while (__builtin_popcountll(fr->B))
	{
		in_B = __builtin_ffsll(fr->B)-1;
		in = ((arg.all_p & occupancyMaskBishop[in_B]) * magicNumberBishop[in_B]) >> magicNumberShiftsBishop[in_B];
		mB_q = magicMovesBishop[in_B][in] & ~ fr->pieces;
		mB_c = mB_q & ho->pieces & check_grid;
		mB_q &= ~ ho->pieces & check_grid;

		while (__builtin_popcountll(mB_q))
		{
			in = __builtin_ffsll(mB_q)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_B);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 3) ^ stm;
			//save enp_sq, castle flags<1$>
			m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
			mB_q &= ~m_list->dest; 
		}
		while (__builtin_popcountll(mB_c))
		{
			in = __builtin_ffsll(mB_c)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_B);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 3) ^ (1LL << 6) ^ stm;
			//determine captured piece<$1>
			f = (ho->Q >> in) & 1LL;
			mask = 1LL << 25;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->R >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->B >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->N >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->P >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			//save enp_sq, castle flags<1$>
			m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
			mB_c &= ~m_list->dest; 
		}
		fr->B &= ~1LL << in_B; 
	}
	//ROOK
	fr->R &= ~ all_pp;
	while (__builtin_popcountll(fr->R))
	{
		in_R = __builtin_ffsll(fr->R)-1;
		in = ((arg.all_p & occupancyMaskRook[in_R]) * magicNumberRook[in_R]) >> magicNumberShiftsRook[in_R];
		mR_q = magicMovesRook[in_R][in] & ~ fr->pieces;
		mR_c = mR_q & ho->pieces & check_grid;
		mR_q &= ~ ho->pieces & check_grid;

		while (__builtin_popcountll(mR_q))
		{
			in = __builtin_ffsll(mR_q)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_R);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 2) ^ stm;
			//save enp_sq, castle flags<1$>
			m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
			mR_q &= ~m_list->dest; 
		}
		while (__builtin_popcountll(mR_c))
		{
			in = __builtin_ffsll(mR_c)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_R);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 2) ^ (1LL << 6) ^ stm;
			//determine captured piece<$1>
			f = (ho->Q >> in) & 1LL;
			mask = 1LL << 25;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->R >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->B >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->N >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->P >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			//save enp_sq, castle flags<1$>
			m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
			mR_c &= ~m_list->dest; 
		}
		fr->R &= ~1LL << in_R; 
	}
	//QUEEN
	fr->Q &= ~ all_pp;
	while (__builtin_popcountll(fr->Q))
	{
		in_Q = __builtin_ffsll(fr->Q)-1;
		in_R = ((arg.all_p & occupancyMaskRook[in_Q]) * magicNumberRook[in_Q]) >> magicNumberShiftsRook[in_Q];
		in_B = ((arg.all_p & occupancyMaskBishop[in_Q]) * magicNumberBishop[in_Q]) >> magicNumberShiftsBishop[in_Q];
		mQ_q = (magicMovesRook[in_Q][in_R] ^ magicMovesBishop[in_Q][in_B]) & ~ fr->pieces;
		mQ_c = mQ_q & ho->pieces & check_grid;
		mQ_q &= ~ ho->pieces & check_grid;

		while (__builtin_popcountll(mQ_q))
		{
			in = __builtin_ffsll(mQ_q)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_Q);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 1) ^ stm;
			//save enp_sq, castle flags<1$>
			m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
			mQ_q &= ~m_list->dest; 
		}
		while (__builtin_popcountll(mQ_c))
		{
			in = __builtin_ffsll(mQ_c)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->from = (1LL << in_Q);
			m_list->dest = (1LL << in);
			m_list->info = (1LL << 1) ^ (1LL << 6) ^ stm;
			//determine captured piece<$1>
			f = (ho->Q >> in) & 1LL;
			mask = 1LL << 25;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->R >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->B >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->N >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			f = (ho->P >> in) & 1LL;
			mask <<= 1;
			m_list->info = (m_list->info & ~mask) | ( -f & mask);
			//save enp_sq, castle flags<1$>
			m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
			mQ_c &= ~m_list->dest; 
		}
		fr->Q &= ~1LL << in_Q; 
	}
	//KING
	in_K = __builtin_ffsll(fr->K)-1;
	mK_q = movesKing[in_K] & ~fr->pieces & ~ho->atack;
	mK_c = mK_q & ho->pieces;
	mK_q &= ~ ho->pieces;
	while (__builtin_popcountll(mK_q))
	{
		in = __builtin_ffsll(mK_q)-1;
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->from = (1LL << in_K);
		m_list->dest = (1LL << in);
		m_list->info = 1LL  ^ stm;
		//save enp_sq, castle flags<1$>
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check is missing, write as const except stm<1$>
		mK_q &= ~m_list->dest; 
	}
	while (__builtin_popcountll(mK_c))
	{
		in = __builtin_ffsll(mK_c)-1;
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->from = (1LL << in_K);
		m_list->dest = (1LL << in);
		m_list->info = 1LL ^ (1LL << 6) ^ stm;
		//determine captured piece<$1>
		f = (ho->Q >> in) & 1LL;
		mask = 1LL << 25;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->R >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->B >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->N >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		f = (ho->P >> in) & 1LL;
		mask <<= 1;
		m_list->info = (m_list->info & ~mask) | ( -f & mask);
		//save enp_sq, castle flags<1$>
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check is missing, write as const except stm<1$>
		mK_c &= ~m_list->dest; 
	}
	//CASTLE
	if ( arg.info & cas_bit_K && !(ho->atack & cas_at_K) && !(arg.all_p & cas_occ_K) )
	{
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->info = 1LL << 13 ^ stm;
		//save enp_sq, castle flags<1$>
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
	}
	if ( arg.info & cas_bit_Q && !(ho->atack & cas_at_Q) && !(arg.all_p & cas_occ_Q) )
	{
		m = m_list;
		m_list = malloc(sizeof(move));
		m_list->next = m;
		m_list->info = 1LL << 14 ^ stm;
		//save enp_sq, castle flags<1$>
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
	}
	return m_list;
}

char generate_moves(board_1 arg, move *movearray)
{
	U64 ppb, ppr, bb, at_b, at_r, blank = 0LL, all_pp, mpb, mpr, piece_type, capture, stm, enp;
	U64 check_pieces, check_grid = ~0LL;
	U64 in_N, in_B, in_R, in_Q, in_K, in;
	U64 mN_q, mN_c, mB_q, mB_c, mR_q, mR_c, mQ_q, mQ_c, mK_q, mK_c;
	U64 mP1, mP2, mPE, mPW, mENP, mP1_prom, mPE_prom, mPW_prom, it_prom;
	U64 cas_bit_K, cas_bit_Q, cas_at_K, cas_at_Q, cas_occ_K, cas_occ_Q, promotion, ENProw, enp_P;
	U64 f, mask;
	piece_set *fr, *ho; 
	int in_at_b, in_at_r, king, in_at, in_k, at, pp, mpp, P1, P2, PE, PW;
	char  movecount = 0;
	stm = (arg.info & 1LL) << 24;
	if (stm )
	{
		fr = &arg.w;
		ho = &arg.b;
		cas_bit_K = 0x0000000000000002;
		cas_bit_Q = 0x0000000000000004;
		cas_at_K =  0x000000000000000E;
		cas_at_Q =  0x0000000000000038;
		cas_occ_K = 0x0000000000000006;
		cas_occ_Q = 0x0000000000000070;
		promotion = 0xFF00000000000000;
		P1 = 8;
		P2 = 16;
		PE = 7;
		PW = 9;
		enp = (arg.info & 0x000000000000FF00) << 32;
		enp_P = enp >> 8;
		ENProw = 0x000000FF00000000;
	}
	else
	{
		fr = &arg.b;
		ho = &arg.w;
		cas_bit_K = 0x0000000000000008;
		cas_bit_Q = 0x0000000000000010;
		cas_at_K =  0x0E00000000000000;
		cas_at_Q =  0x3800000000000000;
		cas_occ_K = 0x0600000000000000;
		cas_occ_Q = 0x7000000000000000;
		promotion = 0x00000000000000FF;
		P1 = -8;
		P2 = -16;
		PE = -9;
		PW = -7;
		enp = (arg.info & 0x000000000000FF00) << 8;
		enp_P = enp << 8;
		ENProw = 0x00000000FF000000;
	}
	king = __builtin_ffsll(fr->K)-1;
	//building atack map pieces, later wont be necessary becuase it will be updated incrementally by make, unmake move<$1>
	fr->pieces = fr->P ^ fr->N ^ fr->R ^ fr->B ^ fr->K ^ fr->Q;
	ho->pieces = ho->P ^ ho->N ^ ho->R ^ ho->B ^ ho->K ^ ho->Q;
	arg.all_p = fr->pieces ^ ho->pieces;
	ho->atack = gen_ho_atack(arg);
	//check if king is in CHECK<$1>
	if (fr->K & ho->atack)
	{
		if (stm)	in_K = (fr->K & 0xFEFEFEFEFEFEFEFE) << 7 & ho->P | (fr->K & 0x7F7F7F7F7F7F7F7F) << 9 & ho->P;  		
		else	in_K = (fr->K & 0xFEFEFEFEFEFEFEFE) >> 9 & ho->P | (fr->K & 0x7F7F7F7F7F7F7F7F) >> 7 & ho->P;  		

		in_at_b = ( (arg.all_p & occupancyMaskBishop[king]) * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho->B ^ ho->Q); //friendly pieces<$1>
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.all_p & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho->R ^ ho->Q); //friendly pieces<$1>

		in_N = movesNight[king] & ho->N;

		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.all_p & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); //friendly pieces<$1>
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.all_p & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
				//index of king can be outside of while loop<$1>
				check_grid = magicMovesRook[king][in_at_r] & magicMovesRook[at][in_at] | (1LL << at);
			}
			else if (__builtin_popcountll(in_N))
			{
				check_grid = in_N;
			}
			else if (__builtin_popcountll(in_K))
			{
				check_grid = in_K;
			}
		}
		else check_grid = 0LL;

		all_pp = 0LL;
		ppb = 0LL;
		ppr = 0LL;
		//looking for pieces pinned diagonaly<1$>
		in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho->B ^ ho->Q); //friendly pieces<$1>
		while (__builtin_popcountll(at_b) )
		{
			at = __builtin_ffsll(at_b)-1;
			in_at = ((arg.all_p & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
			//index of king can be outside of while loop<$1>
			in_k = ((arg.all_p & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
			ppb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at];
			all_pp ^= ppb;

			pp = __builtin_ffsll(ppb)-1;
			bb = arg.all_p & ~ (1LL << pp);
			in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
			in_k = ((bb & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
			mpb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at] &  ~ppb ^ (1LL << at); 
			mpb &= check_grid;

			if (__builtin_popcountll( (fr->Q ^ fr->B) & ppb )) //maybe popcount isn't neccessary<1$>
			{
				while ( __builtin_popcountll(mpb))
				{
					mpp = __builtin_ffsll(mpb)-1;
					if (fr->B & ppb)	piece_type = (1LL << 3);
					else piece_type = (1LL << 1);
					//checking for check?<$1>
					capture = (1LL << mpp) & ho->pieces ? 1LL << 6 : 0LL;					
					//new move pp on mpp<$1>
					movearray[movecount].from = ppb;
					movearray[movecount].dest = (1LL << mpp);
					movearray[movecount].info = piece_type ^ capture ^ stm; 
					//determine captured piece<$1>
					f = (ho->Q >> mpp) & 1LL;
					mask = 1LL << 25;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					f = (ho->R >> mpp) & 1LL;
					mask <<= 1;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					f = (ho->B >> mpp) & 1LL;
					mask <<= 1;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					f = (ho->N >> mpp) & 1LL;
					mask <<= 1;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					f = (ho->P >> mpp) & 1LL;
					mask <<= 1;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					//save enp_sq, castle flags<1$>
					movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					mpb &= ~movearray[movecount].dest; 
					movecount++;
				}
			}
			//checking for pawn<1$>
			if ( fr->P & ppb)
			{
				pp = __builtin_ffsll(ppb)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
				if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho->pieces & ~promotion & check_grid);
				else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho->pieces & ~promotion & check_grid );
				//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>

				if (mpp)
				{
					//without promotion<1$>
					movearray[movecount].from = ppb;
					movearray[movecount].dest = (1LL << (mpp-1));
					//check is missing, write as const except stm<1$>
					//determine captured piece<$1>
					f = (ho->Q >> (mpp-1)) & 1LL;
					mask = 1LL << 25;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					f = (ho->R >> (mpp-1)) & 1LL;
					mask <<= 1;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					f = (ho->B >> (mpp-1)) & 1LL;
					mask <<= 1;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					f = (ho->N >> (mpp-1)) & 1LL;
					mask <<= 1;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					f = (ho->P >> (mpp-1)) & 1LL;
					mask <<= 1;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					//save enp_sq, castle flags<1$>
					movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					mpb &= ~movearray[movecount].dest; //do we really need that?<1$>
					movecount++;
				}
				else 
				{
					//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>
					if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho->pieces & promotion & check_grid);
					else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho->pieces & promotion & check_grid );
					if (mpp)
					{
						//promotion 
						for (it_prom = 0x0000000000000100; it_prom ^ 0x0000000000001000; it_prom <<= 1)
						{
							in = __builtin_ffsll(mPW_prom)-1;
							movearray[movecount].from = ppb;
							movearray[movecount].dest = (1LL << (mpp-1));
							movearray[movecount].info = (1LL << 5) ^ (1LL << 6) ^ (1LL << 15) ^ it_prom ^ stm;
							//determine captured piece<$1>
							f = (ho->Q >> (mpp-1)) & 1LL;
							mask = 1LL << 25;
							movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
							f = (ho->R >> (mpp-1)) & 1LL;
							mask <<= 1;
							movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
							f = (ho->B >> (mpp-1)) & 1LL;
							mask <<= 1;
							movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
							f = (ho->N >> (mpp-1)) & 1LL;
							mask <<= 1;
							movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
							f = (ho->P >> (mpp-1)) & 1LL;
							mask <<= 1;
							movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
							//save enp_sq, castle flags<1$>
							movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
							movecount++;
						}
						//check is missing, write as const except stm<1$>
						mpb &= ~movearray[movecount-1].dest; //do we really need that?<1$>
					}
					else
					{
						if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho->pieces^enp) & check_grid);
						else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho->pieces ^ enp) & check_grid);
						if (mpp)
						{
							//promotion excluded when en'passan<1$>
							capture = 1LL << 6;
							movearray[movecount].from = ppb;
							movearray[movecount].dest = (1LL << (mpp-1));
							movearray[movecount].info = (1LL << 5) ^ (1LL << 6) ^ (1LL << 12) ^ stm;
							//check is missing, write as const except stm<1$>
							//determine captured piece<$1>
							f = (ho->Q >> (mpp-1)) & 1LL;
							mask = 1LL << 25;
							movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
							f = (ho->R >> (mpp-1)) & 1LL;
							mask <<= 1;
							movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
							f = (ho->B >> (mpp-1)) & 1LL;
							mask <<= 1;
							movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
							f = (ho->N >> (mpp-1)) & 1LL;
							mask <<= 1;
							movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
							f = (ho->P >> (mpp-1)) & 1LL;
							mask <<= 1;
							movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
							//save enp_sq, castle flags<1$>
							movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
							mpb &= ~movearray[movecount].dest; //do we really need that?<1$>
							movecount++;
						}
					}
				}
			}
			//checking for pawn<1$>
			at_b &= ~(1LL << at); 
		}
		//looking for pieces pinned by file or rank<1$>
		in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho->R ^ ho->Q); //friendly pieces<$1>
		while (__builtin_popcountll(at_r) )
		{
			at = __builtin_ffsll(at_r)-1;
			in_at = ((arg.all_p & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
			//index of king can be outside of while loop<$1>
			in_k = ((arg.all_p & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
			ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
			all_pp ^= ppr;

			pp = __builtin_ffsll(ppr)-1;
			bb = arg.all_p & ~ (1LL << pp);
			in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
			in_k = ((bb & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
			mpr = magicMovesRook[king][in_k] & magicMovesRook[at][in_at] &  ~ppr ^ (1LL << at); 
			mpr &= check_grid;

			if (__builtin_popcountll( (fr->Q ^ fr->R) & ppr )) 
			{
				while ( __builtin_popcountll(mpr))
				{
					mpp = __builtin_ffsll(mpr)-1;
					if (fr->R & ppr)	piece_type = (1LL << 2);
					else piece_type = (1LL << 1);
					//checking for check?<$1>
					capture = (1LL << mpp) & ho->pieces ? 1LL << 6 : 0LL;					
					//new move pp on mpp<$1>
					movearray[movecount].from = ppr;
					movearray[movecount].dest = (1LL << mpp);
					movearray[movecount].info = piece_type ^ capture ^ stm; 
					//determine captured piece<$1>
					f = (ho->Q >> mpp) & 1LL;
					mask = 1LL << 25;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					f = (ho->R >> mpp) & 1LL;
					mask <<= 1;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					f = (ho->B >> mpp) & 1LL;
					mask <<= 1;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					f = (ho->N >> mpp) & 1LL;
					mask <<= 1;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					f = (ho->P >> mpp) & 1LL;
					mask <<= 1;
					movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
					//save enp_sq, castle flags<1$>
					movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					mpr &= ~movearray[movecount].dest; 
					movecount++;
				}
			}
			//checking for pawn<1$>
			if ( fr->P & ppr)
			{
				pp = __builtin_ffsll(ppr)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
				if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.all_p) << 8) & ~arg.all_p & check_grid);//include just two rank step<1$>
				else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.all_p) >> 8) & ~arg.all_p & check_grid);//include just two rank step<1$>
				//pawn advance or pawn capture<1$>
				if (mpp)
				{
					movearray[movecount].from = ppr;
					movearray[movecount].dest = (1LL << (mpp-1));
					//check is missing, write as const except stm<1$>
					//save enp_sq, castle flags<1$>
					movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;

					mpb &= ~movearray[movecount].dest; 
					movecount++;
				}
				//else 
				{
					mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.all_p) & check_grid
						: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.all_p) & check_grid;//include just one rank step<1$>
					if (mpp)
					{
						//promotion excluded when en'passan<1$>
						movearray[movecount].from = ppr;
						movearray[movecount].dest = (1LL << (mpp-1));
						movearray[movecount].info = (1LL << 5) ^ (1LL << 15) ^ stm;
						//check is missing, write as const except stm<1$>
						//save enp_sq, castle flags<1$>
						movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
						mpb &= ~movearray[movecount].dest; 
						movecount++;
					}
				}
			}
			at_r &= ~(1LL << at); 
		}
		//legal moves<1$>
		//PAWN
		fr->P &= ~ all_pp;
		if (stm)
		{
			mP2 = ((fr->P & 0x000000000000FF00) << 8 & ~arg.all_p) << 8 & ~arg.all_p & check_grid;
			mP1 = fr->P << 8 & ~arg.all_p & check_grid;
			mPE = (fr->P & 0xFEFEFEFEFEFEFEFE) << 7 & ho->pieces & check_grid;
			mPW = (fr->P & 0x7F7F7F7F7F7F7F7F) << 9 & ho->pieces & check_grid;
			mENP = (enp & 0xFEFEFEFEFEFEFEFE & check_grid) >> 9 & fr->P ^ (enp & 0x7F7F7F7F7F7F7F7F & check_grid) >> 7 & fr->P;
			mENP |= (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr->P ^ (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr->P;
			mP1_prom = mP1 & promotion;
			mPE_prom = mPE & promotion;
			mPW_prom = mPW & promotion;
			mP1 &= ~promotion;
			mPE &= ~promotion;
			mPW &= ~promotion;
		}
		else 
		{
			mP2 = ((fr->P & 0x00FF000000000000) >> 8 & ~arg.all_p) >> 8 & ~arg.all_p & check_grid;
			mP1 = fr->P >> 8 & ~arg.all_p & check_grid;
			mPE = (fr->P & 0xFEFEFEFEFEFEFEFE) >> 9 & ho->pieces & check_grid;
			mPW = (fr->P & 0x7F7F7F7F7F7F7F7F) >> 7 & ho->pieces & check_grid;
			mENP = (enp & 0x7F7F7F7F7F7F7F7F & check_grid) << 9 & fr->P ^ (enp & 0xFEFEFEFEFEFEFEFE & check_grid) << 7 & fr->P;
			mENP |= (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr->P ^ (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr->P;
			mP1_prom = mP1 & promotion;
			mPE_prom = mPE & promotion;
			mPW_prom = mPW & promotion;
			mP1 &= ~promotion;
			mPE &= ~promotion;
			mPW &= ~promotion;
		}

		while (__builtin_popcountll(mP2))
		{
			in = __builtin_ffsll(mP2)-1;
			movearray[movecount].from = 1LL << (in - P2 );
			movearray[movecount].dest = 1LL << in;
			movearray[movecount].info = (1LL << 5) ^ (1LL << 15) ^ (1LL << (16 + in%8)) ^ stm;
			//save enp_sq, castle flags<1$>
			movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
			mP2 &= ~movearray[movecount].dest; 
			movecount++;
		}
		while (__builtin_popcountll(mP1))
		{
			in = __builtin_ffsll(mP1)-1;
			movearray[movecount].from = 1LL << (in - P1 );
			movearray[movecount].dest = 1LL << in;
			movearray[movecount].info = (1LL << 5) ^ (1LL << 15) ^ stm;
			//save enp_sq, castle flags<1$>
			movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
			mP1 &= ~movearray[movecount].dest; 
			movecount++;
		}
		while (__builtin_popcountll(mPW))
		{
			in = __builtin_ffsll(mPW)-1;
			movearray[movecount].from = 1LL << (in - PW);
			movearray[movecount].dest = 1LL << in;
			movearray[movecount].info = (1LL << 5) ^ (1LL << 6) ^ stm;
			//determine captured piece<$1>
			f = (ho->Q >> in) & 1LL;
			mask = 1LL << 25;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			f = (ho->R >> in) & 1LL;
			mask <<= 1;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			f = (ho->B >> in) & 1LL;
			mask <<= 1;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			f = (ho->N >> in) & 1LL;
			mask <<= 1;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			f = (ho->P >> in) & 1LL;
			mask <<= 1;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			//save enp_sq, castle flags<1$>
			movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
			mPW &= ~movearray[movecount].dest; 
			movecount++;
		}
		while (__builtin_popcountll(mPE))
		{
			in = __builtin_ffsll(mPE)-1;
			movearray[movecount].from = 1LL << (in - PE);
			movearray[movecount].dest = 1LL << in;
			movearray[movecount].info = (1LL << 5) ^ (1LL << 6) ^ stm;
			//determine captured piece<$1>
			f = (ho->Q >> in) & 1LL;
			mask = 1LL << 25;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			f = (ho->R >> in) & 1LL;
			mask <<= 1;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			f = (ho->B >> in) & 1LL;
			mask <<= 1;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			f = (ho->N >> in) & 1LL;
			mask <<= 1;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			f = (ho->P >> in) & 1LL;
			mask <<= 1;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			//save enp_sq, castle flags<1$>
			movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
			mPE &= ~movearray[movecount].dest; 
			movecount++;
		}
		while (__builtin_popcountll(mENP))
		{
			in = __builtin_ffsll(mENP)-1;
			//enp check for 4-5 row pin
			bb = ~(1LL << in | ((enp << 8) >> (16*(stm >> 24)))) & arg.all_p & occupancyMaskRook[king];
			in_K = (bb * magicNumberRook[king]) >> magicNumberShiftsRook[king];
			mR_q = magicMovesRook[king][in_K] & (ho->Q | ho-> R) & ENProw;

			if (!(mR_q))
			{
				movearray[movecount].from = 1LL << in;
				movearray[movecount].dest = enp;
				movearray[movecount].info = (1LL << 5) ^ (1LL << 12) ^ stm;
				//save enp_sq, castle flags<1$>
				movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				//check is missing, write as const except stm<1$>
				movecount++;
			}
			mENP &= ~(1LL << in); 
		}
		while (__builtin_popcountll(mP1_prom))
		{
			for (it_prom = 0x0000000000000100; it_prom ^ 0x0000000000001000; it_prom <<= 1)
			{
				in = __builtin_ffsll(mP1_prom)-1;
				movearray[movecount].from = 1LL << (in - P1 );
				movearray[movecount].dest = 1LL << in;
				movearray[movecount].info = (1LL << 5) ^ (1LL << 15) ^ it_prom ^ stm;
				//save enp_sq, castle flags<1$>
				movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				//check is missing, write as const except stm<1$>
				movecount++;
			}
			mP1_prom &= ~movearray[movecount-1].dest; 
		}
		while (__builtin_popcountll(mPW_prom))
		{
			for (it_prom = 0x0000000000000100; it_prom ^ 0x0000000000001000; it_prom <<= 1)
			{
				in = __builtin_ffsll(mPW_prom)-1;
				movearray[movecount].from = 1LL << (in - PW);
				movearray[movecount].dest = 1LL << in;
				movearray[movecount].info = (1LL << 5) ^ (1LL << 6) ^ it_prom ^ stm;
				//determine captured piece<$1>
				f = (ho->Q >> in) & 1LL;
				mask = 1LL << 25;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->R >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->B >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->N >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->P >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				//save enp_sq, castle flags<1$>
				movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				movecount++;
			}
			//check is missing, write as const except stm<1$>
			mPW_prom &= ~movearray[movecount-1].dest; 
		}
		while (__builtin_popcountll(mPE_prom))
		{
			for (it_prom = 0x0000000000000100; it_prom ^ 0x0000000000001000; it_prom <<= 1)
			{
				in = __builtin_ffsll(mPE_prom)-1;
				movearray[movecount].from = 1LL << (in - PE);
				movearray[movecount].dest = 1LL << in;
				movearray[movecount].info = (1LL << 5) ^ (1LL << 6) ^ it_prom ^ stm;
				//determine captured piece<$1>
				f = (ho->Q >> in) & 1LL;
				mask = 1LL << 25;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->R >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->B >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->N >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->P >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				//save enp_sq, castle flags<1$>
				movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				movecount++;
			}
			//check is missing, write as const except stm<1$>
			mPE_prom &= ~movearray[movecount-1].dest; 
		}
		//NIGHT
		fr->N &= ~ all_pp;
		while (__builtin_popcountll(fr->N))
		{
			in_N = __builtin_ffsll(fr->N)-1;
			mN_q = movesNight[in_N] & ~arg.all_p & check_grid;
			mN_c = movesNight[in_N] & ho->pieces & check_grid;

			while (__builtin_popcountll(mN_q))
			{
				in = __builtin_ffsll(mN_q)-1;
				movearray[movecount].from = (1LL << in_N);
				movearray[movecount].dest = (1LL << in);
				movearray[movecount].info = (1LL << 4) ^ stm;
				//save enp_sq, castle flags<1$>
				movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				//check is missing, write as const except stm<1$>
				mN_q &= ~movearray[movecount].dest; 
				movecount++;
			}
			while (__builtin_popcountll(mN_c))
			{
				in = __builtin_ffsll(mN_c)-1;
				movearray[movecount].from = (1LL << in_N);
				movearray[movecount].dest = (1LL << in);
				movearray[movecount].info = (1LL << 4) ^ (1LL << 6) ^ stm;
				//determine captured piece<$1>
				f = (ho->Q >> in) & 1LL;
				mask = 1LL << 25;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->R >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->B >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->N >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->P >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				//save enp_sq, castle flags<1$>
				movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				//check is missing, write as const except stm<1$>
				mN_c &= ~movearray[movecount].dest; 
				movecount++;
			}
			fr->N &= ~1LL << in_N; 
		}
		//BISHOP
		fr->B &= ~ all_pp;
		while (__builtin_popcountll(fr->B))
		{
			in_B = __builtin_ffsll(fr->B)-1;
			in = ((arg.all_p & occupancyMaskBishop[in_B]) * magicNumberBishop[in_B]) >> magicNumberShiftsBishop[in_B];
			mB_q = magicMovesBishop[in_B][in] & ~ fr->pieces;
			mB_c = mB_q & ho->pieces & check_grid;
			mB_q &= ~ ho->pieces & check_grid;

			while (__builtin_popcountll(mB_q))
			{
				in = __builtin_ffsll(mB_q)-1;
				movearray[movecount].from = (1LL << in_B);
				movearray[movecount].dest = (1LL << in);
				movearray[movecount].info = (1LL << 3) ^ stm;
				//save enp_sq, castle flags<1$>
				movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				//check is missing, write as const except stm<1$>
				mB_q &= ~movearray[movecount].dest; 
				movecount++;
			}
			while (__builtin_popcountll(mB_c))
			{
				in = __builtin_ffsll(mB_c)-1;
				movearray[movecount].from = (1LL << in_B);
				movearray[movecount].dest = (1LL << in);
				movearray[movecount].info = (1LL << 3) ^ (1LL << 6) ^ stm;
				//determine captured piece<$1>
				f = (ho->Q >> in) & 1LL;
				mask = 1LL << 25;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->R >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->B >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->N >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->P >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				//save enp_sq, castle flags<1$>
				movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				//check is missing, write as const except stm<1$>
				mB_c &= ~movearray[movecount].dest; 
				movecount++;
			}
			fr->B &= ~1LL << in_B; 
		}
		//ROOK
		fr->R &= ~ all_pp;
		while (__builtin_popcountll(fr->R))
		{
			in_R = __builtin_ffsll(fr->R)-1;
			in = ((arg.all_p & occupancyMaskRook[in_R]) * magicNumberRook[in_R]) >> magicNumberShiftsRook[in_R];
			mR_q = magicMovesRook[in_R][in] & ~ fr->pieces;
			mR_c = mR_q & ho->pieces & check_grid;
			mR_q &= ~ ho->pieces & check_grid;

			while (__builtin_popcountll(mR_q))
			{
				in = __builtin_ffsll(mR_q)-1;
				movearray[movecount].from = (1LL << in_R);
				movearray[movecount].dest = (1LL << in);
				movearray[movecount].info = (1LL << 2) ^ stm;
				//save enp_sq, castle flags<1$>
				movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				//check is missing, write as const except stm<1$>
				mR_q &= ~movearray[movecount].dest; 
				movecount++;
			}
			while (__builtin_popcountll(mR_c))
			{
				in = __builtin_ffsll(mR_c)-1;
				movearray[movecount].from = (1LL << in_R);
				movearray[movecount].dest = (1LL << in);
				movearray[movecount].info = (1LL << 2) ^ (1LL << 6) ^ stm;
				//determine captured piece<$1>
				f = (ho->Q >> in) & 1LL;
				mask = 1LL << 25;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->R >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->B >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->N >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->P >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				//save enp_sq, castle flags<1$>
				movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				//check is missing, write as const except stm<1$>
				mR_c &= ~movearray[movecount].dest; 
				movecount++;
			}
			fr->R &= ~1LL << in_R; 
		}
		//QUEEN
		fr->Q &= ~ all_pp;
		while (__builtin_popcountll(fr->Q))
		{
			in_Q = __builtin_ffsll(fr->Q)-1;
			in_R = ((arg.all_p & occupancyMaskRook[in_Q]) * magicNumberRook[in_Q]) >> magicNumberShiftsRook[in_Q];
			in_B = ((arg.all_p & occupancyMaskBishop[in_Q]) * magicNumberBishop[in_Q]) >> magicNumberShiftsBishop[in_Q];
			mQ_q = (magicMovesRook[in_Q][in_R] ^ magicMovesBishop[in_Q][in_B]) & ~ fr->pieces;
			mQ_c = mQ_q & ho->pieces & check_grid;
			mQ_q &= ~ ho->pieces & check_grid;

			while (__builtin_popcountll(mQ_q))
			{
				in = __builtin_ffsll(mQ_q)-1;
				movearray[movecount].from = (1LL << in_Q);
				movearray[movecount].dest = (1LL << in);
				movearray[movecount].info = (1LL << 1) ^ stm;
				//save enp_sq, castle flags<1$>
				movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				//check is missing, write as const except stm<1$>
				mQ_q &= ~movearray[movecount].dest; 
				movecount++;
			}
			while (__builtin_popcountll(mQ_c))
			{
				in = __builtin_ffsll(mQ_c)-1;
				movearray[movecount].from = (1LL << in_Q);
				movearray[movecount].dest = (1LL << in);
				movearray[movecount].info = (1LL << 1) ^ (1LL << 6) ^ stm;
				//determine captured piece<$1>
				f = (ho->Q >> in) & 1LL;
				mask = 1LL << 25;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->R >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->B >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->N >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				f = (ho->P >> in) & 1LL;
				mask <<= 1;
				movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
				//save enp_sq, castle flags<1$>
				movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
				//check is missing, write as const except stm<1$>
				mQ_c &= ~movearray[movecount].dest; 
				movecount++;
			}
			fr->Q &= ~1LL << in_Q; 
		}
		//KING
		in_K = __builtin_ffsll(fr->K)-1;
		mK_q = movesKing[in_K] & ~fr->pieces & ~ho->atack;
		mK_c = mK_q & ho->pieces;
		mK_q &= ~ ho->pieces;

		while (__builtin_popcountll(mK_q))
		{
			in = __builtin_ffsll(mK_q)-1;
			movearray[movecount].from = (1LL << in_K);
			movearray[movecount].dest = (1LL << in);
			movearray[movecount].info = 1LL  ^ stm;
			//save enp_sq, castle flags<1$>
			movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
			mK_q &= ~movearray[movecount].dest; 
			movecount++;
		}
		while (__builtin_popcountll(mK_c))
		{
			in = __builtin_ffsll(mK_c)-1;
			movearray[movecount].from = (1LL << in_K);
			movearray[movecount].dest = (1LL << in);
			movearray[movecount].info = 1LL ^ (1LL << 6) ^ stm;
			//determine captured piece<$1>
			f = (ho->Q >> in) & 1LL;
			mask = 1LL << 25;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			f = (ho->R >> in) & 1LL;
			mask <<= 1;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			f = (ho->B >> in) & 1LL;
			mask <<= 1;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			f = (ho->N >> in) & 1LL;
			mask <<= 1;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			f = (ho->P >> in) & 1LL;
			mask <<= 1;
			movearray[movecount].info = (movearray[movecount].info & ~mask) | ( -f & mask);
			//save enp_sq, castle flags<1$>
			movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check is missing, write as const except stm<1$>
			mK_c &= ~movearray[movecount].dest; 
			movecount++;
		}
		//CASTLE
		if ( arg.info & cas_bit_K && !(ho->atack & cas_at_K) && !(arg.all_p & cas_occ_K) )
		{
			movearray[movecount].info = 1LL << 13 ^ stm;
			//save enp_sq, castle flags<1$>
			movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			movecount++;
		}
		if ( arg.info & cas_bit_Q && !(ho->atack & cas_at_Q) && !(arg.all_p & cas_occ_Q) )
		{
			movearray[movecount].info = 1LL << 14 ^ stm;
			//save enp_sq, castle flags<1$>
			movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			movecount++;
		}
		return movecount;
	}
}

U64 gen_ho_atack(board_1 arg)
{
	U64 in_N, in_B, in_R, in_Q, in_K, in, at;
	piece_set *fr, *ho; 

	at = 0LL;

	if ((arg.info & 1LL) << 24)  //stm
	{
		fr = &arg.w;
		ho = &arg.b;
		at |= (ho->P & 0xFEFEFEFEFEFEFEFE) >> 9;
		at |= (ho->P & 0x7F7F7F7F7F7F7F7F) >> 7;
	}
	else
	{
		fr = &arg.b;
		ho = &arg.w;
		at |= (ho->P & 0xFEFEFEFEFEFEFEFE) << 7;
		at |= (ho->P & 0x7F7F7F7F7F7F7F7F) << 9;
	}
	//search squares without king cause of possible check<1$>
	arg.all_p ^= fr->K;  
	//NIGHT
	while (__builtin_popcountll(ho->N))
	{
		in_N = __builtin_ffsll(ho->N)-1;
		at |= movesNight[in_N];
		ho->N &= ~1LL << in_N; 
	}
	//BISHOP
	while (__builtin_popcountll(ho->B))
	{
		in_B = __builtin_ffsll(ho->B)-1;
		in = ((arg.all_p & occupancyMaskBishop[in_B]) * magicNumberBishop[in_B]) >> magicNumberShiftsBishop[in_B];
		at |= magicMovesBishop[in_B][in];
		ho->B &= ~1LL << in_B; 
	}
	//ROOK
	while (__builtin_popcountll(ho->R))
	{
		in_R = __builtin_ffsll(ho->R)-1;
		in = ((arg.all_p & occupancyMaskRook[in_R]) * magicNumberRook[in_R]) >> magicNumberShiftsRook[in_R];
		at |= magicMovesRook[in_R][in] ;
		ho->R &= ~1LL << in_R; 
	}
	//QUEEN
	while (__builtin_popcountll(ho->Q))
	{
		in_Q = __builtin_ffsll(ho->Q)-1;
		in_R = ((arg.all_p & occupancyMaskRook[in_Q]) * magicNumberRook[in_Q]) >> magicNumberShiftsRook[in_Q];
		in_B = ((arg.all_p & occupancyMaskBishop[in_Q]) * magicNumberBishop[in_Q]) >> magicNumberShiftsBishop[in_Q];
		at |= (magicMovesRook[in_Q][in_R] ^ magicMovesBishop[in_Q][in_B]);
		ho->Q &= ~1LL << in_Q; 
	}
	//KING
	in_K = __builtin_ffsll(ho->K)-1;
	at |= movesKing[in_K];
	return at;
}

void do_move(board_1 *b, move *m)
{
	U64 *p, *capt_p, *prom_p, stm, castle, cast_diff, hm, fm, m_cas, f_cas, empty = 0ULL, CK, CQ, KS, KR, QR, hKR, hQR, t;
	piece_set *fr, *ho; 
	int p_type, capt_type, prom_type, sq_from, sq_dest, enp;

	stm = b->info & 0x0000000000000001;
	fr = stm ? &b->w : &b->b;
	ho = stm ? &b->b : &b->w;
	KR = 0x0100000000000000 >> (stm*56);
	QR = 0x8000000000000000 >> (stm*56);
	//ULL used to escape left shirt warning<1$>
	hKR = 0x0000000000000001ULL << (stm*56);
	hQR = 0x0000000000000080ULL << (stm*56);
	castle = 0LL;

	if (!(m->info & 0x0000000000007000))
	{
		p = m->info & 1LL      ? &fr->K : p;
		p = m->info & 1LL << 1 ? &fr->Q : p;
		p = m->info & 1LL << 2 ? &fr->R : p;
		p = m->info & 1LL << 3 ? &fr->B : p;
		p = m->info & 1LL << 4 ? &fr->N : p;
		p = m->info & 1LL << 5 ? &fr->P : p;

		p_type = __builtin_ffsll( m->info )-1;
		sq_from = __builtin_ffsll( m->from)-1;
		sq_dest = __builtin_ffsll( m->dest)-1;

		b->zobrist ^= zobrist[ stm * 6*64 + p_type * 64 + sq_from];
		b->zobrist ^= zobrist[ stm * 6*64 + p_type * 64 + sq_dest];
		//loosing castle rights if king plays or castling, posible improvement with bitwise operator<1$>
		m_cas = 0x000000000000018 >> (stm<<1);
		f_cas = (m->info & 1LL) | (m->info & 0x000000000006000) >> 13;
		castle |= (castle & ~m_cas) | (-f_cas & m_cas);
		//if rook plays, loosing appropriate castling rights<1$>
		m_cas = 0x000000000000008 >> (stm<<1);
		f_cas = (m->info & 1LL << 2) && ( m->from & KR); 
		castle |= (castle & ~m_cas) | (-f_cas & m_cas);
		m_cas = 0x000000000000010 >> (stm<<1);
		f_cas = (m->info & 1LL << 2) && ( m->from & QR);
		castle |= (castle & ~m_cas) | (-f_cas & m_cas);
		//loosing castle rights when rook is captured<1$>
		//not checked if mask is bigger than condition<1$>
		m_cas = 0x000000000000002 << (stm<<1);
		f_cas = (m->dest & hKR) && ( m->info & 1LL << 26 );
		castle |= (castle & ~m_cas) | (-f_cas & m_cas);

		m_cas = 0x000000000000004 << (stm<<1);
		f_cas = (m->dest & hQR) && ( m->info & 1LL << 26 );
		castle |= (castle & ~m_cas) | (-f_cas & m_cas);
		//doing move
		*p &= ~m->from;
		*p |= m->dest;
		//removing captured piece
		capt_p = &empty;
		capt_p = m->info & 1LL << 25 ? &ho->Q : capt_p;
		capt_p = m->info & 1LL << 26 ? &ho->R : capt_p;
		capt_p = m->info & 1LL << 27 ? &ho->B : capt_p;
		capt_p = m->info & 1LL << 28 ? &ho->N : capt_p;
		capt_p = m->info & 1LL << 29 ? &ho->P : capt_p;
		*capt_p &= ~m->dest;	
		//zobrist captured piece
		capt_type = __builtin_ffsll( m->info >> 25 & 0x000000000000001F);
		b->zobrist ^= capt_type ? zobrist[ !stm * 6*64 + capt_type * 64 + 64 + sq_dest] : 0ULL;
		//promotion
		prom_p = &empty;
		prom_p = m->info & 1LL << 8 ? &fr->Q : prom_p;
		prom_p = m->info & 1LL << 9 ? &fr->R : prom_p;
		prom_p = m->info & 1LL << 10 ? &fr->B : prom_p;
		prom_p = m->info & 1LL << 11 ? &fr->N : prom_p;
		*prom_p ^= m->dest;
		fr->P ^= m->info & 0x0000000000000F00 ? m->dest : 0ULL; 
		//zobrist promotion piece
		prom_type = __builtin_ffsll( m->info >> 8 & 0x000000000000000F);
		b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + prom_type * 64 + 64 + sq_dest] : 0ULL;
		b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + 5 * 64 + sq_dest] : 0ULL;
		//update all_p
		b->all_p &= ~m->from;
		b->all_p |= m->dest;
	}
	else if (m->info & 0x0000000000001000)
	{
		sq_from = __builtin_ffsll( m->from)-1;
		sq_dest = __builtin_ffsll( m->dest)-1;

		fr->P &= ~m->from;
		fr->P |= m->dest;
		ho->P &= stm ? ~m->dest >> 8 : ~m->dest << 8;	

		b->all_p &= ~m->from;
		b->all_p |= m->dest;
		b->all_p &= stm ? ~m->dest >> 8 : ~m->dest << 8;
		//zobrist
		b->zobrist ^= zobrist[ stm * 6*64 + 5 * 64 + sq_dest];
		b->zobrist ^= zobrist[ stm * 6*64 + 5 * 64 + sq_from];
		b->zobrist ^= zobrist[ !stm * 6*64 + 5 * 64 + sq_dest + 8 - stm*16 ];
	}
	else if (m->info & 0x0000000000002000)
	{
		CK = 0x0200000000000000 >> stm*56;
		CQ = 0x2000000000000000 >> stm*56;
		KS = 0x0800000000000000 >> stm*56;

		castle = 0x000000000000018 >> (stm*2);
		fr->K &= ~KS;
		fr->K |= CK;
		fr->R &= ~KR;
		fr->R |= CK << 1;

		b->all_p &= ~KS;
		b->all_p |= CK;
		b->all_p &= ~KR;
		b->all_p |= CK << 1;
		//zobrist
		b->zobrist ^= zobrist[ stm * 6*64 + 57 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 59 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 56 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 58 - stm*56];
	}
	else if (m->info & 0x0000000000006000)
	{
		CK = 0x0200000000000000 >> stm*56;
		CQ = 0x2000000000000000 >> stm*56;
		KS = 0x0800000000000000 >> stm*56;

		castle = 0x000000000000018 >> (stm*2);
		fr->K &= ~KS;
		fr->K |= CQ;
		fr->R &= ~QR;
		fr->R |= CQ >> 1;

		b->all_p &= ~KS;
		b->all_p |= CQ;
		b->all_p &= ~QR;
		b->all_p |= CQ >> 1;
		//zobrist
		b->zobrist ^= zobrist[ stm * 6*64 + 61 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 59 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 63 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 60 - stm*56];
	}
	//increase hm if not capture or pawn move<1$>
	hm = m->info & 0x000000003E008000 ? -(b->info & 0x0000000000FF0000) : (1LL << 16);
	//zobrist
	cast_diff = b->info & castle;
	b->zobrist ^= zobrist[ 778] * ((cast_diff & 0x0000000000000002) >> 1);  
	b->zobrist ^= zobrist[ 779] * ((cast_diff & 0x0000000000000004) >> 2);  
	b->zobrist ^= zobrist[ 780] * ((cast_diff & 0x0000000000000008) >> 3);  
	b->zobrist ^= zobrist[ 781] * ((cast_diff & 0x0000000000000010) >> 4); 

	enp = __builtin_ffsll(b->info >> 8 & 0x00000000000000FF);
	b->zobrist ^= enp ? zobrist[ 768 + enp] : 0ULL;

	enp = __builtin_ffsll(m->info >> 16 & 0x00000000000000FF);
	b->zobrist ^= enp ? zobrist[ 768 + enp] : 0ULL;

	b->zobrist ^= zobrist[ 777];

	b->info &= ~castle; 

	b->info ^= 0x0000000000000001;
	b->info ^= (b->info & 0x000000000000FF00) ^ ( m->info & 0x0000000000FF0000) >> 8;
	b->info += hm;
	b->info += (~ stm & 1LL) << 32;
}

void undo_move(board_1 *b, move *m)
{
	U64 *p, *capt_p, *prom_p, stm, empty = 0LL, hm, CK, CQ, KS, KR, QR, cast_diff;
	piece_set *fr, *ho; 
	int p_type, sq_from, sq_dest, capt_type, prom_type, enp;

	stm = m->info >> 24 & 0x0000000000000001 ;
	fr = stm ? &b->w : &b->b;
	ho = stm ? &b->b : &b->w;

	if (!(m->info & 0x0000000000007000))
	{
		p = m->info & 1LL      ? &fr->K : p;
		p = m->info & 1LL << 1 ? &fr->Q : p;
		p = m->info & 1LL << 2 ? &fr->R : p;
		p = m->info & 1LL << 3 ? &fr->B : p;
		p = m->info & 1LL << 4 ? &fr->N : p;
		p = m->info & 1LL << 5 ? &fr->P : p;

		p_type = __builtin_ffsll( m->info )-1;
		sq_from = __builtin_ffsll( m->from)-1;
		sq_dest = __builtin_ffsll( m->dest)-1;

		b->zobrist ^= zobrist[ stm * 6*64 + p_type * 64 + sq_from];
		b->zobrist ^= zobrist[ stm * 6*64 + p_type * 64 + sq_dest];
		//undoing move
		*p &= ~m->dest;
		*p |= m->from;
		//returning captured piece
		capt_p = &empty;
		capt_p = m->info & 1LL << 25 ? &ho->Q : capt_p;
		capt_p = m->info & 1LL << 26 ? &ho->R : capt_p;
		capt_p = m->info & 1LL << 27 ? &ho->B : capt_p;
		capt_p = m->info & 1LL << 28 ? &ho->N : capt_p;
		capt_p = m->info & 1LL << 29 ? &ho->P : capt_p;
		*capt_p |= m->dest;	
		//zobrist captured piece
		capt_type = __builtin_ffsll( m->info >> 25 & 0x000000000000001F);
		b->zobrist ^= capt_type ? zobrist[ !stm * 6*64 + capt_type * 64 + 64  + sq_dest] : 0ULL;
		//promotion
		prom_p = &empty;
		prom_p = m->info & 1LL << 8 ? &fr->Q : prom_p;
		prom_p = m->info & 1LL << 9 ? &fr->R : prom_p;
		prom_p = m->info & 1LL << 10 ? &fr->B : prom_p;
		prom_p = m->info & 1LL << 11 ? &fr->N : prom_p;
		*prom_p ^= m->dest;
		fr->P |= m->info & 0x0000000000000F00 ? m->from : 0ULL; 
		//zobrist promotion piece
		prom_type = __builtin_ffsll( m->info >> 8 & 0x000000000000000F);
		b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + prom_type * 64 + 64 + sq_dest] : 0ULL;
		b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + 5 * 64 + sq_dest] : 0ULL;
		//update all_p
		b->all_p &= m->info & 0x000000003E000000 ? ~0LL : ~m->dest;
		b->all_p |= m->from;
	}
	else if (m->info & 0x0000000000001000)
	{
		sq_from = __builtin_ffsll( m->from)-1;
		sq_dest = __builtin_ffsll( m->dest)-1;

		fr->P &= ~m->dest;
		fr->P |= m->from;
		ho->P |= stm ? m->dest >> 8 : m->dest << 8;	

		b->all_p &= ~m->dest;
		b->all_p |= m->from;
		b->all_p |= stm ? m->dest >> 8 : m->dest << 8;
		//zobrist
		b->zobrist ^= zobrist[ stm * 6*64 + 5 * 64 + sq_dest];
		b->zobrist ^= zobrist[ stm * 6*64 + 5 * 64 + sq_from];
		b->zobrist ^= zobrist[ !stm * 6*64 + 5 * 64 + sq_dest + 8 - stm*16 ];
	}
	else if (m->info & 0x0000000000002000)
	{
		KR = 0x0100000000000000 >> stm*56;
		QR = 0x8000000000000000 >> stm*56;
		CK = 0x0200000000000000 >> stm*56;
		CQ = 0x2000000000000000 >> stm*56;
		KS = 0x0800000000000000 >> stm*56;

		fr->K |= KS;
		fr->K &= ~CK;
		fr->R |= KR;
		fr->R &= ~(CK << 1);

		b->all_p |= KS;
		b->all_p &= ~CK;
		b->all_p |= KR;
		b->all_p &= ~(CK << 1);

		//zobrist
		b->zobrist ^= zobrist[ stm * 6*64 + 57 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 59 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 56 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 58 - stm*56];
	}
	else if (m->info & 0x0000000000006000)
	{
		//maybe possible to use && or ?<1$>
		KR = 0x0100000000000000 >> stm*56;
		QR = 0x8000000000000000 >> stm*56;
		CK = 0x0200000000000000 >> stm*56;
		CQ = 0x2000000000000000 >> stm*56;
		KS = 0x0800000000000000 >> stm*56;

		fr->K |= KS;
		fr->K &= ~CQ;
		fr->R |= QR;
		fr->R &= ~(CQ >> 1);

		b->all_p |= KS;
		b->all_p &= ~CQ;
		b->all_p |= QR;
		b->all_p &= ~(CQ >> 1);
		//zobrist
		b->zobrist ^= zobrist[ stm * 6*64 + 61 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 59 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 63 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 60 - stm*56];
	}
	//zobrist
	cast_diff = m->info >> 32 ^ b->info;
	b->zobrist ^= zobrist[ 778] * (cast_diff >> 1 & 1ULL);  
	b->zobrist ^= zobrist[ 779] * (cast_diff >> 2 & 1ULL);  
	b->zobrist ^= zobrist[ 780] * (cast_diff >> 3 & 1ULL);  
	b->zobrist ^= zobrist[ 781] * (cast_diff >> 4 & 1ULL);  

	enp = __builtin_ffsll(b->info >> 8 & 0x00000000000000FF);
	b->zobrist ^= enp ? zobrist[ 768 + enp] : 0ULL;

	enp = __builtin_ffsll(m->info >> 40 & 0x00000000000000FF);
	b->zobrist ^= enp ? zobrist[ 768 + enp] : 0ULL;

	b->zobrist ^= zobrist[ 777];


	b->info -= (b->info & 1LL) << 32;
	b->info ^= 0x0000000000000001;
	b->info = (m->info & 0x00FFFF1E00000000) >> 32 | (b->info & 0x000000FF00000001);
}


void printmoves(move *m_l)
{
	int count = 0;
	while ( m_l && (count < 50))
	{
		printf("%d  ", count);
		printmove( m_l);
		printf("\n");

		m_l = m_l->next;
		count ++;
	}
	printf("moves count: %d	\n", count);
}

void printmove(move *m_l)
{
	int piece_type, count = 0;
	short d_pa;
	char promotion, fr[3], dst[3];

	piece_type = __builtin_ffsll(m_l->info)-1;
	square( __builtin_ffsll(m_l->dest)-1, dst);
	square( __builtin_ffsll(m_l->from)-1, fr);

	if (piece_type == 13 || piece_type == 14) 	printf("%s ", piece(piece_type));
	else	printf("%s%s%s ", piece(piece_type), fr, dst);
}
