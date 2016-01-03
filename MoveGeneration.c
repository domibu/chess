#include <stdio.h>
#include <stdlib.h>
#include "BitBoardMagic.h"
#include "MoveGeneration.h"
#include "Search.h"

static inline void fill_move( move *move, U64 p_type, U64 source, U64 destination);
static inline void capt_type( U64 *ho, U64 in, move *move, U64 f, U64 mask);

U64 generate_captures_2(Nmovelist *ZZZ, board arg)
{
	//if (stop) return 0;
	U64 ppb, ppr, bb, at_b, at_r, blank = 0LL, all_pp, mpb, mpr, capture, stm, enp;
	U64 check_pieces, check_grid = ~0LL;
	U64 in_N, in_B, in_R, in_Q, in_K, in;
	U64 mN_q, mN_c, mB_q, mB_c, mR_q, mR_c, mQ_q, mQ_c, mK_q, mK_c;
	U64 mP1, mP2, mPE, mPW, mENP, mP1_prom, mPE_prom, mPW_prom, it_prom;
	U64 cas_bit_K, cas_bit_Q, cas_at_K, cas_at_Q, cas_occ_K, cas_occ_Q, promotion, ENProw, enp_P;
	U64 f, mask;
	U64 *fr, *ho; 
	int in_at_b, in_at_r, king, in_at, in_k, at, pp, mpp, P1, P2, PE, PW;
	unsigned char quietcount = 0,  tmp, piecetype, captcount = 218, enp_sq;

	stm = (arg.info >> 11) & 0x0000000000000008;
	if (stm )
	{
		fr = &arg.pieceset[0];
		ho = &arg.pieceset[8];
		promotion = 0xFF00000000000000;
		P1 = 8;
		PE = 7;
		PW = 9;
		enp_sq = (arg.info >> 1 & 0x0000000000000007) + 40;
		enp = (1LL << enp_sq)*(arg.info & 1LL);
		enp_P = enp >> 8;
		ENProw = 0x000000FF00000000;
	}
	else
	{
		fr = &arg.pieceset[8];
		ho = &arg.pieceset[0];
		promotion = 0x00000000000000FF;
		P1 = -8;
		PE = -9;
		PW = -7;
		enp_sq = (arg.info >> 1 & 0x0000000000000007) + 16;
		enp = (1LL << enp_sq)*(arg.info & 1LL);
		enp_P = enp << 8;
		ENProw = 0x00000000FF000000;
	}
	king = __builtin_ffsll(fr[0])-1;
	//building atack map pieces, later wont be necessary becuase it will be updated incrementally by make, unmake move<$1>
	fr[6] = fr[5] ^ fr[4] ^ fr[2] ^ fr[3] ^ fr[0] ^ fr[1];
	ho[6] = ho[5] ^ ho[4] ^ ho[2] ^ ho[3] ^ ho[0] ^ ho[1];
	arg.pieceset[16] = fr[6] ^ ho[6];
	ho[7] = gen_ho_atackN(arg);
	//save (enp, cast, hm, stm) for undo<1$>
	ZZZ->undo = arg.info; 
	ZZZ->old_zobrist = arg.zobrist;

	//check if king is in CHECK<$1>
	if (fr[0] & ho[7])
	{
		if (stm)	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[5];  		
		else	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[5];  		

		in_at_b = ( (arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); //friendly pieces<$1>
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); //friendly pieces<$1>

		in_N = movesNight[king] & ho[4];

		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); //friendly pieces<$1>
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
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
	}
	all_pp = 0LL;
	ppb = 0LL;
	ppr = 0LL;

	//looking for pieces pinned diagonaly<1$>
	in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
	at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); //friendly pieces<$1>

	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		ppb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at];
		//ppb = magicMovesBishop[king][in_k] ;
		all_pp ^= ppb;
		pp = __builtin_ffsll(ppb)-1;
		bb = arg.pieceset[16] & ~ (1LL << pp);
		in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		in_k = ((bb & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		mpb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at] &  ~ppb ^ (1LL << at); 
		mpb &= check_grid;

		if (__builtin_popcountll( (fr[1] ^ fr[3]) & ppb )) //maybe popcount isn't neccessary<1$>
		{
			while ( __builtin_popcountll(mpb))
			{
				mpp = __builtin_ffsll(mpb)-1;
				if (fr[3] & ppb)	piecetype = 3;
				else piecetype = 1;
				//checking for check?<$1>
				tmp = (1LL << mpp) & ho[6] ? captcount : quietcount;					
				//new move pp on mpp<$1>
				ZZZ->mdata[tmp] &= 0LL;
				ZZZ->mdata[tmp] ^= piecetype; //piecetype
				ZZZ->mdata[tmp] ^= pp << 6; //source
				ZZZ->mdata[tmp] ^= mpp << 12; //destination
				//determine captured piece<$1>
				f = (ho[1] >> mpp) & 1LL;
				mask = 0x0000000000000008;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[2] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[3] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[4] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[5] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);

				mpb &= ~(1LL << (mpp)); 
				(tmp == captcount) ? captcount++ : quietcount++;
			}
		}
		//checking for pawn<1$>
		if ( fr[5] & ppb)
		{
			pp = __builtin_ffsll(ppb)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
			if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & ~promotion & check_grid);
			else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & ~promotion & check_grid );
			//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>

			if (mpp)
			{
				//without promotion<1$>
				ZZZ->mdata[captcount] &= 0LL;
				ZZZ->mdata[captcount] ^= 5; //piecetype
				ZZZ->mdata[captcount] ^= pp << 6; //source
				ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
				//determine captured piece<$1>
				f = (ho[1] >> (mpp-1)) & 1LL;
				mask = 0x0000000000000008;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
				f = (ho[2] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
				f = (ho[3] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
				f = (ho[4] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
				f = (ho[5] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

				mpb &= ~(1LL << (mpp-1)); //do we really need that?<1$>
				captcount++;
			}
			else 
			{
				//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>
				if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & promotion & check_grid);
				else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & promotion & check_grid );
				if (mpp)
				{
					//promotion 
					for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
					{
						ZZZ->mdata[captcount] &= 0LL;
						ZZZ->mdata[captcount] ^= 5; //piecetype
						ZZZ->mdata[captcount] ^= pp << 6; //source
						ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
						ZZZ->mdata[captcount] ^= it_prom ^ 0x0000000000040000; //promotion

						//determine captured piece<$1>
						f = (ho[1] >> (mpp-1)) & 1LL;
						mask = 0x0000000000000008;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[2] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[3] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[4] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[5] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

						captcount++;
					}
					//check is missing, write as const except stm<1$>
					mpb &= ~(1LL << (mpp-1)); //do we really need that?<1$>
				}
				else
				{
					if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho[6]^enp) & check_grid);
					else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho[6] ^ enp) & check_grid);
					if (mpp)
					{
						//promotion excluded when en'passan<1$>
						ZZZ->mdata[captcount] &= 0LL;
						ZZZ->mdata[captcount] ^= 7; //piecetype
						ZZZ->mdata[captcount] ^= pp << 6; //source
						ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
						//determine captured piece<$1>
						f = (ho[1] >> (mpp-1)) & 1LL;
						mask = 0x0000000000000008;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[2] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[3] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[4] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[5] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

						mpb &= ~(1LL << (mpp-1)); //do we really need that?<1$>
						captcount++;
					}
				}
			}
		}
		at_b &= ~(1LL << at); 
	}

	//looking for pieces pinned by file or rank<1$>
	in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
	at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); //friendly pieces<$1>
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
		all_pp ^= ppr;

		pp = __builtin_ffsll(ppr)-1;
		bb = arg.pieceset[16] & ~ (1LL << pp);
		in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		in_k = ((bb & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		mpr = magicMovesRook[king][in_k] & magicMovesRook[at][in_at] &  ~ppr ^ (1LL << at); 
		mpr &= check_grid;

		if (__builtin_popcountll( (fr[1] ^ fr[2]) & ppr )) 
		{
			while ( __builtin_popcountll(mpr))
			{
				mpp = __builtin_ffsll(mpr)-1;
				if (fr[2] & ppr)	piecetype = 2;
				else piecetype = 1;
				//checking for check?<$1>
				tmp = (1LL << mpp) & ho[6] ? captcount : quietcount;					

				//new move pp on mpp<$1>
				ZZZ->mdata[tmp] &= 0LL;
				ZZZ->mdata[tmp] ^= piecetype; //piecetype
				ZZZ->mdata[tmp] ^= pp << 6; //source
				ZZZ->mdata[tmp] ^= mpp << 12; //destination
				//determine captured piece<$1>
				f = (ho[1] >> mpp) & 1LL;
				mask = 0x0000000000000008;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[2] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[3] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[4] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[5] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);

				mpr &= ~(1LL << (mpp)); 
				(tmp == captcount) ? captcount++ : quietcount++;;
			}
		}
	}
	//checking for pawn<1$>
	if ( fr[5] & ppr)
	{
		pp = __builtin_ffsll(ppr)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
		if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.pieceset[16]) << 8) & ~arg.pieceset[16] & check_grid);//include just two rank step<1$>
		else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.pieceset[16]) >> 8) & ~arg.pieceset[16] & check_grid);//include just two rank step<1$>
		//pawn advance or pawn capture<1$>
		//without promotion<1$>
		if (mpp)
		{
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 5; //piecetype
			ZZZ->mdata[quietcount] ^= pp << 6; //source
			ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination
			ZZZ->mdata[quietcount] ^= (((mpp - 1)%8) << 22) ^ (1LL << 21); //enp_file

			mpr &= ~(1LL << (mpp-1)); //do we really need that?<1$>
			quietcount++;
		}
		mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.pieceset[16]) & check_grid
			: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.pieceset[16]) & check_grid;//include just one rank step<1$>
		if (mpp)
		{
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 5; //piecetype
			ZZZ->mdata[quietcount] ^= pp << 6; //source
			ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination

			mpr &= ~(1LL << (mpp-1)); //do we really need that?<1$>
			quietcount++;
		}
		at_r &= ~(1LL << at); 
	}
	//legal moves<1$>
	//PAWN
	fr[5] &= ~ all_pp;
	if (stm)
	{
		mP1 = fr[5] << 8 & ~arg.pieceset[16] & check_grid;
		mPE = (fr[5] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[6] & check_grid;
		mPW = (fr[5] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[6] & check_grid;
		mENP = (enp & 0xFEFEFEFEFEFEFEFE & check_grid) >> 9 & fr[5] ^ (enp & 0x7F7F7F7F7F7F7F7F & check_grid) >> 7 & fr[5];
		mENP |= (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5] ^ (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5];
		mP1_prom = mP1 & promotion;
		mPE_prom = mPE & promotion;
		mPW_prom = mPW & promotion;
		mP1 &= ~promotion;
		mPE &= ~promotion;
		mPW &= ~promotion;
	}
	else 
	{
		mP1 = fr[5] >> 8 & ~arg.pieceset[16] & check_grid;
		mPE = (fr[5] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[6] & check_grid;
		mPW = (fr[5] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[6] & check_grid;
		mENP = (enp & 0x7F7F7F7F7F7F7F7F & check_grid) << 9 & fr[5] ^ (enp & 0xFEFEFEFEFEFEFEFE & check_grid) << 7 & fr[5];
		mENP |= (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5] ^ (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5];
		mP1_prom = mP1 & promotion;
		mPE_prom = mPE & promotion;
		mPW_prom = mPW & promotion;
		mP1 &= ~promotion;
		mPE &= ~promotion;
		mPW &= ~promotion;
	}

	while (__builtin_popcountll(mPW))
	{
		//without promotion<1$>
		in = __builtin_ffsll(mPW)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 5; //piecetype
		ZZZ->mdata[captcount] ^= (in - PW) << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
		//determine captured piece<$1>
		f = (ho[1] >> in) & 1LL;
		mask = 0x0000000000000008;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[2] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[3] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[4] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[5] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

		mPW &= ~(1LL << in); 
		captcount++;
	}
	while (__builtin_popcountll(mPE))
	{
		//without promotion<1$>
		in = __builtin_ffsll(mPE)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 5; //piecetype
		ZZZ->mdata[captcount] ^= (in - PE) << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
		//determine captured piece<$1>
		f = (ho[1] >> in) & 1LL;
		mask = 0x0000000000000008;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[2] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[3] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[4] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[5] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

		mPE &= ~(1LL << in); 
		captcount++;
	}
	while (__builtin_popcountll(mENP))
	{
		in = __builtin_ffsll(mENP)-1;
		//enp check for 4-5 row pin
		bb = ~(1LL << in | (enp_P)) & arg.pieceset[16] & occupancyMaskRook[king];
		in_K = (bb * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		mR_q = magicMovesRook[king][in_K] & (ho[1] | ho[2]) & ENProw;

		if (!(mR_q))
		{
			//promotion excluded when en'passan<1$>
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 7; //piecetype
			ZZZ->mdata[captcount] ^= in << 6; //source
			ZZZ->mdata[captcount] ^= enp_sq << 12; //destination

			captcount++;
		}
		mENP &= ~(1LL << in); 
	}
	while (__builtin_popcountll(mP1_prom))
	{
		in = __builtin_ffsll(mP1_prom)-1;
		for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
		{
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 5; //piecetype
			ZZZ->mdata[quietcount] ^= (in - P1) << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination
			ZZZ->mdata[quietcount] ^= it_prom ^ 0x0000000000040000; //promotion

			quietcount++;
		}
		//check is missing, write as const except stm<1$>
		mP1_prom &= ~(1LL << in); //do we really need that?<1$>
	}
	while (__builtin_popcountll(mPW_prom))
	{
		in = __builtin_ffsll(mPW_prom)-1;
		for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
		{
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 5; //piecetype
			ZZZ->mdata[captcount] ^= (in - PW) << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			ZZZ->mdata[captcount] ^= it_prom ^ 0x0000000000040000; //promotion
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			captcount++;
		}
		//check is missing, write as const except stm<1$>
		mPW_prom &= ~(1LL << in); 
	}
	while (__builtin_popcountll(mPE_prom))
	{
		in = __builtin_ffsll(mPE_prom)-1;
		for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
		{
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 5; //piecetype
			ZZZ->mdata[captcount] ^= (in - PE) << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			ZZZ->mdata[captcount] ^= it_prom ^ 0x0000000000040000; //promotion
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			captcount++;
		}
		//check is missing, write as const except stm<1$>
		mPE_prom &= ~(1LL << in); 
	}
	//NIGHT
	fr[4] &= ~ all_pp;
	while (__builtin_popcountll(fr[4]))
	{
		in_N = __builtin_ffsll(fr[4])-1;
		mN_c = movesNight[in_N] & ho[6] & check_grid;

		while (__builtin_popcountll(mN_c))
		{
			in = __builtin_ffsll(mN_c)-1;
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 4; //piecetype
			ZZZ->mdata[captcount] ^= in_N << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

			mN_c &= ~(1LL << in); 
			captcount++;
		}
		fr[4] &= ~(1LL << in_N); 
	}
	//BISHOP
	fr[3] &= ~ all_pp;
	while (__builtin_popcountll(fr[3]))
	{
		in_B = __builtin_ffsll(fr[3])-1;
		in = ((arg.pieceset[16] & occupancyMaskBishop[in_B]) * magicNumberBishop[in_B]) >> magicNumberShiftsBishop[in_B];
		mB_q = magicMovesBishop[in_B][in] & ~ fr[6];
		mB_c = mB_q & ho[6] & check_grid;

		while (__builtin_popcountll(mB_c))
		{
			in = __builtin_ffsll(mB_c)-1;
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 3; //piecetype
			ZZZ->mdata[captcount] ^= in_B << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

			mB_c &= ~(1LL << in); 
			captcount++;
		}
		fr[3] &= ~(1LL << in_B); 
	}
	//ROOK
	fr[2] &= ~ all_pp;
	while (__builtin_popcountll(fr[2]))
	{
		in_R = __builtin_ffsll(fr[2])-1;
		in = ((arg.pieceset[16] & occupancyMaskRook[in_R]) * magicNumberRook[in_R]) >> magicNumberShiftsRook[in_R];
		mR_q = magicMovesRook[in_R][in] & ~ fr[6];
		mR_c = mR_q & ho[6] & check_grid;

		while (__builtin_popcountll(mR_c))
		{
			in = __builtin_ffsll(mR_c)-1;
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 2; //piecetype
			ZZZ->mdata[captcount] ^= in_R << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

			mR_c &= ~(1LL << in); 
			captcount++;
		}
		fr[2] &= ~1LL << in_R; 
	}
	//QUEEN
	fr[1] &= ~ all_pp;
	while (__builtin_popcountll(fr[1]))
	{
		in_Q = __builtin_ffsll(fr[1])-1;
		in_R = ((arg.pieceset[16] & occupancyMaskRook[in_Q]) * magicNumberRook[in_Q]) >> magicNumberShiftsRook[in_Q];
		in_B = ((arg.pieceset[16] & occupancyMaskBishop[in_Q]) * magicNumberBishop[in_Q]) >> magicNumberShiftsBishop[in_Q];
		mQ_q = (magicMovesRook[in_Q][in_R] ^ magicMovesBishop[in_Q][in_B]) & ~ fr[6];
		mQ_c = mQ_q & ho[6] & check_grid;
		mQ_q &= ~ ho[6] & check_grid;

		while (__builtin_popcountll(mQ_c))
		{
			in = __builtin_ffsll(mQ_c)-1;
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 1; //piecetype
			ZZZ->mdata[captcount] ^= in_Q << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

			mQ_c &= ~(1LL << in); 
			captcount++;
		}
		fr[1] &= ~1LL << in_Q; 
	}
	//KING
	in_K = __builtin_ffsll(fr[0])-1;
	mK_q = movesKing[in_K] & ~fr[6] & ~ho[7];
	mK_c = mK_q & ho[6];
	mK_q &= ~ ho[6];

	while (__builtin_popcountll(mK_c))
	{
		in = __builtin_ffsll(mK_c)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 0; //piecetype
		ZZZ->mdata[captcount] ^= in_K << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
		//determine captured piece<$1>
		f = (ho[1] >> in) & 1LL;
		mask = 0x0000000000000008;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[2] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[3] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[4] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[5] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

		mK_c &= ~(1LL << in); 
		captcount++;
	}

	ZZZ->quietcount = quietcount;
	ZZZ->captcount = captcount;

	return captcount - 218;
}

U64 generate_captures(Nmovelist *ZZZ, board arg)
{
	//if (stop) return 0;
	U64 ppb, ppr, bb, at_b, at_r, blank = 0LL, all_pp, mpb, mpr, capture, stm, enp;
	U64 check_pieces, check_grid = ~0LL;
	U64 in_N, in_B, in_R, in_Q, in_K, in;
	U64 mN_q, mN_c, mB_q, mB_c, mR_q, mR_c, mQ_q, mQ_c, mK_q, mK_c;
	U64 mP1, mP2, mPE, mPW, mENP, mP1_prom, mPE_prom, mPW_prom, it_prom;
	U64 cas_bit_K, cas_bit_Q, cas_at_K, cas_at_Q, cas_occ_K, cas_occ_Q, promotion, ENProw, enp_P;
	U64 f, mask;
	U64 *fr, *ho; 
	int in_at_b, in_at_r, king, in_at, in_k, at, pp, mpp, P1, P2, PE, PW;
	unsigned char quietcount = 0,  tmp, piecetype, captcount = 218, enp_sq;

	stm = (arg.info >> 11) & 0x0000000000000008;
	if (stm )
	{
		fr = &arg.pieceset[0];
		ho = &arg.pieceset[8];
		promotion = 0xFF00000000000000;
		P1 = 8;
		PE = 7;
		PW = 9;
		enp_sq = (arg.info >> 1 & 0x0000000000000007) + 40;
		enp = (1LL << enp_sq)*(arg.info & 1LL);
		enp_P = enp >> 8;
		ENProw = 0x000000FF00000000;
	}
	else
	{
		fr = &arg.pieceset[8];
		ho = &arg.pieceset[0];
		promotion = 0x00000000000000FF;
		P1 = -8;
		PE = -9;
		PW = -7;
		enp_sq = (arg.info >> 1 & 0x0000000000000007) + 16;
		enp = (1LL << enp_sq)*(arg.info & 1LL);
		enp_P = enp << 8;
		ENProw = 0x00000000FF000000;
	}
	king = __builtin_ffsll(fr[0])-1;

	//building atack map pieces, later wont be necessary becuase it will be updated incrementally by make, unmake move<$1>
	fr[6] = fr[5] ^ fr[4] ^ fr[2] ^ fr[3] ^ fr[0] ^ fr[1];
	ho[6] = ho[5] ^ ho[4] ^ ho[2] ^ ho[3] ^ ho[0] ^ ho[1];
	arg.pieceset[16] = fr[6] ^ ho[6];
	ho[7] = gen_ho_atackN(arg);

	//save (enp, cast, hm, stm) for undo<1$>
	ZZZ->undo = arg.info; 
	ZZZ->old_zobrist = arg.zobrist;

	//check if king is in CHECK<$1>
	if (fr[0] & ho[7])
	{
		if (stm)	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[5];  		
		else	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[5];  		

		in_at_b = ( (arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); //friendly pieces<$1>
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); //friendly pieces<$1>

		in_N = movesNight[king] & ho[4];

		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); //friendly pieces<$1>
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
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
	}
	all_pp = 0LL;
	ppb = 0LL;
	ppr = 0LL;
	//looking for pieces pinned diagonaly<1$>
	in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
	at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); //friendly pieces<$1>

	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		ppb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at];
		all_pp ^= ppb;
		pp = __builtin_ffsll(ppb)-1;
		bb = arg.pieceset[16] & ~ (1LL << pp);
		in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		in_k = ((bb & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		mpb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at] &  ~ppb ^ (1LL << at); 
		mpb &= check_grid;

		if (__builtin_popcountll( (fr[1] ^ fr[3]) & ppb )) //maybe popcount isn't neccessary<1$>
		{
			while ( __builtin_popcountll(mpb))
			{
				mpp = __builtin_ffsll(mpb)-1;
				if (fr[3] & ppb)	piecetype = 3;
				else piecetype = 1;
				//checking for check?<$1>
				tmp = (1LL << mpp) & ho[6] ? captcount : quietcount;					
				//new move pp on mpp<$1>
				ZZZ->mdata[tmp] &= 0LL;
				ZZZ->mdata[tmp] ^= piecetype; //piecetype
				ZZZ->mdata[tmp] ^= pp << 6; //source
				ZZZ->mdata[tmp] ^= mpp << 12; //destination
				//determine captured piece<$1>
				f = (ho[1] >> mpp) & 1LL;
				mask = 0x0000000000000008;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[2] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[3] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[4] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[5] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);

				mpb &= ~(1LL << (mpp)); 
				(tmp == captcount) ? captcount++ : quietcount++;
			}
		}
		//checking for pawn<1$>
		if ( fr[5] & ppb)
		{
			pp = __builtin_ffsll(ppb)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
			if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & ~promotion & check_grid);
			else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & ~promotion & check_grid );
			//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>

			if (mpp)
			{
				//without promotion<1$>
				ZZZ->mdata[captcount] &= 0LL;
				ZZZ->mdata[captcount] ^= 5; //piecetype
				ZZZ->mdata[captcount] ^= pp << 6; //source
				ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
				//determine captured piece<$1>
				f = (ho[1] >> (mpp-1)) & 1LL;
				mask = 0x0000000000000008;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
				f = (ho[2] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
				f = (ho[3] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
				f = (ho[4] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
				f = (ho[5] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

				mpb &= ~(1LL << (mpp-1)); //do we really need that?<1$>
				captcount++;
			}
			else 
			{
				//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>
				if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & promotion & check_grid);
				else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & promotion & check_grid );
				if (mpp)
				{
					//promotion 
					for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
					{
						ZZZ->mdata[captcount] &= 0LL;
						ZZZ->mdata[captcount] ^= 5; //piecetype
						ZZZ->mdata[captcount] ^= pp << 6; //source
						ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
						ZZZ->mdata[captcount] ^= it_prom ^ 0x0000000000040000; //promotion
						//determine captured piece<$1>
						f = (ho[1] >> (mpp-1)) & 1LL;
						mask = 0x0000000000000008;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[2] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[3] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[4] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[5] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

						captcount++;
					}
					//check is missing, write as const except stm<1$>
					mpb &= ~(1LL << (mpp-1)); //do we really need that?<1$>
				}
				else
				{
					if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho[6]^enp) & check_grid);
					else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho[6] ^ enp) & check_grid);
					if (mpp)
					{
						//promotion excluded when en'passan<1$>
						ZZZ->mdata[captcount] &= 0LL;
						ZZZ->mdata[captcount] ^= 7; //piecetype
						ZZZ->mdata[captcount] ^= pp << 6; //source
						ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
						//determine captured piece<$1>
						f = (ho[1] >> (mpp-1)) & 1LL;
						mask = 0x0000000000000008;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[2] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[3] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[4] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[5] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

						mpb &= ~(1LL << (mpp-1)); //do we really need that?<1$>
						captcount++;
					}
				}
			}
		}
		at_b &= ~(1LL << at); 
	}

	//looking for pieces pinned by file or rank<1$>
	in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
	at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); //friendly pieces<$1>
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
		all_pp ^= ppr;

		pp = __builtin_ffsll(ppr)-1;
		bb = arg.pieceset[16] & ~ (1LL << pp);
		in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		in_k = ((bb & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		mpr = magicMovesRook[king][in_k] & magicMovesRook[at][in_at] &  ~ppr ^ (1LL << at); 
		mpr &= check_grid;

		if (__builtin_popcountll( (fr[1] ^ fr[2]) & ppr )) 
		{
			while ( __builtin_popcountll(mpr))
			{
				mpp = __builtin_ffsll(mpr)-1;
				if (fr[2] & ppr)	piecetype = 2;
				else piecetype = 1;
				//checking for check?<$1>
				//capture = (1LL << mpp) & ho[6] ? 1LL << 6 : 0LL;					
				tmp = (1LL << mpp) & ho[6] ? captcount : quietcount;					
				//new move pp on mpp<$1>
				ZZZ->mdata[tmp] &= 0LL;
				ZZZ->mdata[tmp] ^= piecetype; //piecetype
				ZZZ->mdata[tmp] ^= pp << 6; //source
				ZZZ->mdata[tmp] ^= mpp << 12; //destination
				//determine captured piece<$1>
				f = (ho[1] >> mpp) & 1LL;
				mask = 0x0000000000000008;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[2] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[3] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[4] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[5] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);

				mpr &= ~(1LL << (mpp)); 
				(tmp == captcount) ? captcount++ : quietcount++;;
			}
		}
	}
	//checking for pawn<1$>
	if ( fr[5] & ppr)
	{
		pp = __builtin_ffsll(ppr)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
		if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.pieceset[16]) << 8) & ~arg.pieceset[16] & check_grid);//include just two rank step<1$>
		else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.pieceset[16]) >> 8) & ~arg.pieceset[16] & check_grid);//include just two rank step<1$>
		//pawn advance or pawn capture<1$>
		//without promotion<1$>
		if (mpp)
		{
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 5; //piecetype
			ZZZ->mdata[quietcount] ^= pp << 6; //source
			ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination
			ZZZ->mdata[quietcount] ^= (((mpp - 1)%8) << 22) ^ (1LL << 21); //enp_file

			mpr &= ~(1LL << (mpp-1)); //do we really need that?<1$>
			quietcount++;
		}
		mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.pieceset[16]) & check_grid
			: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.pieceset[16]) & check_grid;//include just one rank step<1$>
		if (mpp)
		{
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 5; //piecetype
			ZZZ->mdata[quietcount] ^= pp << 6; //source
			ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination

			mpr &= ~(1LL << (mpp-1)); //do we really need that?<1$>
			quietcount++;
		}
		at_r &= ~(1LL << at); 
	}
	//legal moves<1$>
	//PAWN
	fr[5] &= ~ all_pp;
	if (stm)
	{
		mP1 = fr[5] << 8 & ~arg.pieceset[16] & check_grid;
		mPE = (fr[5] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[6] & check_grid;
		mPW = (fr[5] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[6] & check_grid;
		mENP = (enp & 0xFEFEFEFEFEFEFEFE & check_grid) >> 9 & fr[5] ^ (enp & 0x7F7F7F7F7F7F7F7F & check_grid) >> 7 & fr[5];
		mENP |= (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5] ^ (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5];
		mP1_prom = mP1 & promotion;
		mPE_prom = mPE & promotion;
		mPW_prom = mPW & promotion;
		mP1 &= ~promotion;
		mPE &= ~promotion;
		mPW &= ~promotion;
	}
	else 
	{
		mP1 = fr[5] >> 8 & ~arg.pieceset[16] & check_grid;
		mPE = (fr[5] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[6] & check_grid;
		mPW = (fr[5] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[6] & check_grid;
		mENP = (enp & 0x7F7F7F7F7F7F7F7F & check_grid) << 9 & fr[5] ^ (enp & 0xFEFEFEFEFEFEFEFE & check_grid) << 7 & fr[5];
		mENP |= (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5] ^ (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5];
		mP1_prom = mP1 & promotion;
		mPE_prom = mPE & promotion;
		mPW_prom = mPW & promotion;
		mP1 &= ~promotion;
		mPE &= ~promotion;
		mPW &= ~promotion;
	}

	while (__builtin_popcountll(mPW))
	{
		//without promotion<1$>
		in = __builtin_ffsll(mPW)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 5; //piecetype
		ZZZ->mdata[captcount] ^= (in - PW) << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
		//determine captured piece<$1>
		f = (ho[1] >> in) & 1LL;
		mask = 0x0000000000000008;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[2] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[3] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[4] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[5] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

		mPW &= ~(1LL << in); 
		captcount++;
	}
	while (__builtin_popcountll(mPE))
	{
		//without promotion<1$>
		in = __builtin_ffsll(mPE)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 5; //piecetype
		ZZZ->mdata[captcount] ^= (in - PE) << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
		//determine captured piece<$1>
		f = (ho[1] >> in) & 1LL;
		mask = 0x0000000000000008;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[2] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[3] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[4] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[5] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

		mPE &= ~(1LL << in); 
		captcount++;
	}
	while (__builtin_popcountll(mENP))
	{
		in = __builtin_ffsll(mENP)-1;
		//enp check for 4-5 row pin
		bb = ~(1LL << in | (enp_P)) & arg.pieceset[16] & occupancyMaskRook[king];
		in_K = (bb * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		mR_q = magicMovesRook[king][in_K] & (ho[1] | ho[2]) & ENProw;

		if (!(mR_q))
		{
			//promotion excluded when en'passan<1$>
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 7; //piecetype
			ZZZ->mdata[captcount] ^= in << 6; //source
			ZZZ->mdata[captcount] ^= enp_sq << 12; //destination

			captcount++;
		}
		mENP &= ~(1LL << in); 
	}
	while (__builtin_popcountll(mP1_prom))
	{
		in = __builtin_ffsll(mP1_prom)-1;
		for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
		{
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 5; //piecetype
			ZZZ->mdata[quietcount] ^= (in - P1) << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination
			ZZZ->mdata[quietcount] ^= it_prom ^ 0x0000000000040000; //promotion

			quietcount++;
		}
		//check is missing, write as const except stm<1$>
		mP1_prom &= ~(1LL << in); //do we really need that?<1$>
	}
	while (__builtin_popcountll(mPW_prom))
	{
		in = __builtin_ffsll(mPW_prom)-1;
		for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
		{
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 5; //piecetype
			ZZZ->mdata[captcount] ^= (in - PW) << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			ZZZ->mdata[captcount] ^= it_prom ^ 0x0000000000040000; //promotion
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			captcount++;
		}
		//check is missing, write as const except stm<1$>
		mPW_prom &= ~(1LL << in); 
	}
	while (__builtin_popcountll(mPE_prom))
	{
		in = __builtin_ffsll(mPE_prom)-1;
		for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
		{
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 5; //piecetype
			ZZZ->mdata[captcount] ^= (in - PE) << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			ZZZ->mdata[captcount] ^= it_prom ^ 0x0000000000040000; //promotion
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			captcount++;
		}
		//check is missing, write as const except stm<1$>
		mPE_prom &= ~(1LL << in); 
	}
	//NIGHT
	fr[4] &= ~ all_pp;
	while (__builtin_popcountll(fr[4]))
	{
		in_N = __builtin_ffsll(fr[4])-1;
		mN_c = movesNight[in_N] & ho[6] & check_grid;

		while (__builtin_popcountll(mN_c))
		{
			in = __builtin_ffsll(mN_c)-1;
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 4; //piecetype
			ZZZ->mdata[captcount] ^= in_N << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

			mN_c &= ~(1LL << in); 
			captcount++;
		}
		fr[4] &= ~(1LL << in_N); 
	}
	//BISHOP
	fr[3] &= ~ all_pp;
	while (__builtin_popcountll(fr[3]))
	{
		in_B = __builtin_ffsll(fr[3])-1;
		in = ((arg.pieceset[16] & occupancyMaskBishop[in_B]) * magicNumberBishop[in_B]) >> magicNumberShiftsBishop[in_B];
		mB_q = magicMovesBishop[in_B][in] & ~ fr[6];
		mB_c = mB_q & ho[6] & check_grid;

		while (__builtin_popcountll(mB_c))
		{
			in = __builtin_ffsll(mB_c)-1;
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 3; //piecetype
			ZZZ->mdata[captcount] ^= in_B << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

			mB_c &= ~(1LL << in); 
			captcount++;
		}
		fr[3] &= ~(1LL << in_B); 
	}
	//ROOK
	fr[2] &= ~ all_pp;
	while (__builtin_popcountll(fr[2]))
	{
		in_R = __builtin_ffsll(fr[2])-1;
		in = ((arg.pieceset[16] & occupancyMaskRook[in_R]) * magicNumberRook[in_R]) >> magicNumberShiftsRook[in_R];
		mR_q = magicMovesRook[in_R][in] & ~ fr[6];
		mR_c = mR_q & ho[6] & check_grid;

		while (__builtin_popcountll(mR_c))
		{
			in = __builtin_ffsll(mR_c)-1;
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 2; //piecetype
			ZZZ->mdata[captcount] ^= in_R << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

			mR_c &= ~(1LL << in); 
			captcount++;
		}
		fr[2] &= ~1LL << in_R; 
	}
	//QUEEN
	fr[1] &= ~ all_pp;
	while (__builtin_popcountll(fr[1]))
	{
		in_Q = __builtin_ffsll(fr[1])-1;
		in_R = ((arg.pieceset[16] & occupancyMaskRook[in_Q]) * magicNumberRook[in_Q]) >> magicNumberShiftsRook[in_Q];
		in_B = ((arg.pieceset[16] & occupancyMaskBishop[in_Q]) * magicNumberBishop[in_Q]) >> magicNumberShiftsBishop[in_Q];
		mQ_q = (magicMovesRook[in_Q][in_R] ^ magicMovesBishop[in_Q][in_B]) & ~ fr[6];
		mQ_c = mQ_q & ho[6] & check_grid;
		mQ_q &= ~ ho[6] & check_grid;

		while (__builtin_popcountll(mQ_c))
		{
			in = __builtin_ffsll(mQ_c)-1;
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 1; //piecetype
			ZZZ->mdata[captcount] ^= in_Q << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

			mQ_c &= ~(1LL << in); 
			captcount++;
		}
		fr[1] &= ~1LL << in_Q; 
	}
	//KING
	in_K = __builtin_ffsll(fr[0])-1;
	mK_q = movesKing[in_K] & ~fr[6] & ~ho[7];
	mK_c = mK_q & ho[6];
	mK_q &= ~ ho[6];

	while (__builtin_popcountll(mK_c))
	{
		in = __builtin_ffsll(mK_c)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 0; //piecetype
		ZZZ->mdata[captcount] ^= in_K << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
		//determine captured piece<$1>
		f = (ho[1] >> in) & 1LL;
		mask = 0x0000000000000008;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[2] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[3] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[4] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[5] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

		mK_c &= ~(1LL << in); 
		captcount++;

	}

	ZZZ->quietcount = quietcount;
	ZZZ->captcount = captcount;

	return captcount - 218;
}

// a definition using static inline
static inline int max(int a, int b) {
	return a > b ? a : b;
}

static inline void fill_move( move *move, U64 p_type, U64 source, U64 destination)
{
	*move &= 0ULL;
	*move ^= p_type; //piecetype
	*move ^= source << 6; //source
	*move ^= destination << 12; //destination
} 

static inline void capt_type( U64 *ho, U64 in, move *move, U64 f, U64 mask)
{
	//determine captured piece<$1>
	f = (ho[1] >> in) & 1LL;
	mask = 0x0000000000000008;
	*move |= (*move & ~mask) | ( -f & mask);
	f = (ho[2] >> in) & 1LL;
	mask += 8;
	*move |= (*move & ~mask) | ( -f & mask);
	f = (ho[3] >> in) & 1LL;
	mask += 8;
	*move |= (*move & ~mask) | ( -f & mask);
	f = (ho[4] >> in) & 1LL;
	mask += 8;
	*move |= (*move & ~mask) | ( -f & mask);
	f = (ho[5] >> in) & 1LL;
	mask += 8;
	*move |= (*move & ~mask) | ( -f & mask);

} 

char generate_movesN_test(Nmovelist *ZZZ, board arg)
{
	//if (stop) return 0;
	U64 ppb, ppr, bb, at_b, at_r, blank = 0LL, all_pp, mpb, mpr, capture, stm, enp;
	U64 check_pieces, check_grid = ~0LL;
	U64 in_N, in_B, in_R, in_Q, in_K, in;
	U64 mN_q, mN_c, mB_q, mB_c, mR_q, mR_c, mQ_q, mQ_c, mK_q, mK_c;
	U64 mP1, mP2, mPE, mPW, mENP, mP1_prom, mPE_prom, mPW_prom, it_prom;
	U64 cas_bit_K, cas_bit_Q, cas_at_K, cas_at_Q, cas_occ_K, cas_occ_Q, promotion, ENProw, enp_P;
	U64 f, mask;
	U64 *fr, *ho; 
	int in_at_b, in_at_r, king, in_at, in_k, at, pp, mpp, P1, P2, PE, PW;
	unsigned char quietcount = 0,  tmp, piecetype, enp_sq;

	stm = (arg.info >> 11) & 0x0000000000000008;
	if (stm )
	{
		fr = &arg.pieceset[0];
		ho = &arg.pieceset[8];
		cas_bit_K = 0x0000000000000010;
		cas_bit_Q = 0x0000000000000020;
		cas_at_K =  0x000000000000000E;
		cas_at_Q =  0x0000000000000038;
		cas_occ_K = 0x0000000000000006;
		cas_occ_Q = 0x0000000000000070;
		promotion = 0xFF00000000000000;
		P1 = 8;
		P2 = 16;
		PE = 7;
		PW = 9;
		enp_sq = (arg.info >> 1 & 0x0000000000000007) + 40;
		enp = (1LL << enp_sq)*(arg.info & 1LL);
		enp_P = enp >> 8;
		ENProw = 0x000000FF00000000;
	}
	else
	{
		fr = &arg.pieceset[8];
		ho = &arg.pieceset[0];
		cas_bit_K = 0x0000000000000040;
		cas_bit_Q = 0x0000000000000080;
		cas_at_K =  0x0E00000000000000;
		cas_at_Q =  0x3800000000000000;
		cas_occ_K = 0x0600000000000000;
		cas_occ_Q = 0x7000000000000000;
		promotion = 0x00000000000000FF;
		P1 = -8;
		P2 = -16;
		PE = -9;
		PW = -7;
		enp_sq = (arg.info >> 1 & 0x0000000000000007) + 16;
		enp = (1LL << enp_sq)*(arg.info & 1LL);
		enp_P = enp << 8;
		ENProw = 0x00000000FF000000;
	}

	king = __builtin_ffsll(fr[0])-1;
	//building atack map pieces, later wont be necessary becuase it will be updated incrementally by make, unmake move<$1>
	fr[6] = fr[5] ^ fr[4] ^ fr[2] ^ fr[3] ^ fr[0] ^ fr[1];
	ho[6] = ho[5] ^ ho[4] ^ ho[2] ^ ho[3] ^ ho[0] ^ ho[1];
	arg.pieceset[16] = fr[6] ^ ho[6];
	ho[7] = gen_ho_atackN(arg);
	//save (enp, cast, hm, stm) for undo<1$>
	ZZZ->undo = arg.info; 
	ZZZ->old_zobrist = arg.zobrist;
	//check if king is in CHECK<$1>
	if (fr[0] & ho[7])
	{
		if (stm)	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[5];  		
		else	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[5];  		

		in_at_b = ( (arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); //friendly pieces<$1>
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); //friendly pieces<$1>

		in_N = movesNight[king] & ho[4];

		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); //friendly pieces<$1>
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
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
	}
	all_pp = 0LL;
	ppb = 0LL;
	ppr = 0LL;

	//looking for pieces pinned diagonaly<1$>
	in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
	at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); //friendly pieces<$1>
	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		ppb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at];
		all_pp ^= ppb;
		bb = arg.pieceset[16] & ~ (1LL << pp);
		in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		in_k = ((bb & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		mpb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at] &  ~ppb ^ (1LL << at); 
		mpb &= check_grid;

		if (__builtin_popcountll( (fr[1] ^ fr[3]) & ppb )) //maybe popcount isn't neccessary<1$>
		{
			while ( __builtin_popcountll(mpb))
			{
				mpp = __builtin_ffsll(mpb)-1;
				if (fr[3] & ppb)	piecetype = 3;
				else piecetype = 1;
				//checking for check?<$1>
				tmp = quietcount;					
				//new move pp on mpp<$1>
				ZZZ->mdata[tmp] &= 0LL;
				ZZZ->mdata[tmp] ^= piecetype; //piecetype
				ZZZ->mdata[tmp] ^= pp << 6; //source
				ZZZ->mdata[tmp] ^= mpp << 12; //destination
				//determine captured piece<$1>
				f = (ho[1] >> mpp) & 1LL;
				mask = 0x0000000000000008;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[2] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[3] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[4] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[5] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);

				mpb &= ~(1LL << (mpp)); 
				quietcount++;
			}
		}
		//checking for pawn<1$>
		if ( fr[5] & ppb)
		{
			pp = __builtin_ffsll(ppb)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
			if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & ~promotion & check_grid);
			else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & ~promotion & check_grid );
			//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>

			if (mpp)
			{
				//without promotion<1$>
				ZZZ->mdata[quietcount] &= 0LL;
				ZZZ->mdata[quietcount] ^= 5; //piecetype
				ZZZ->mdata[quietcount] ^= pp << 6; //source
				ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination
				//determine captured piece<$1>
				f = (ho[1] >> (mpp-1)) & 1LL;
				mask = 0x0000000000000008;
				ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
				f = (ho[2] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
				f = (ho[3] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
				f = (ho[4] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
				f = (ho[5] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);

				mpb &= ~(1LL << (mpp-1)); //do we really need that?<1$>
				quietcount++;
			}
			else 
			{
				//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>
				if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & promotion & check_grid);
				else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & promotion & check_grid );
				if (mpp)
				{
					//promotion 
					for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
					{
						ZZZ->mdata[quietcount] &= 0LL;
						ZZZ->mdata[quietcount] ^= 5; //piecetype
						ZZZ->mdata[quietcount] ^= pp << 6; //source
						ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination
						ZZZ->mdata[quietcount] ^= it_prom ^ 0x0000000000040000; //promotion
						//determine captured piece<$1>
						f = (ho[1] >> (mpp-1)) & 1LL;
						mask = 0x0000000000000008;
						ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
						f = (ho[2] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
						f = (ho[3] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
						f = (ho[4] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
						f = (ho[5] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);

						quietcount++;
					}
					//check is missing, write as const except stm<1$>
					mpb &= ~(1LL << (mpp-1)); //do we really need that?<1$>
				}
				else
				{
					if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho[6]^enp) & check_grid);
					else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho[6] ^ enp) & check_grid);
					if (mpp)
					{
						//promotion excluded when en'passan<1$>
						ZZZ->mdata[quietcount] &= 0LL;
						ZZZ->mdata[quietcount] ^= 7; //piecetype
						ZZZ->mdata[quietcount] ^= pp << 6; //source
						ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination
						//determine captured piece<$1>
						f = (ho[1] >> (mpp-1)) & 1LL;
						mask = 0x0000000000000008;
						ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
						f = (ho[2] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
						f = (ho[3] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
						f = (ho[4] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
						f = (ho[5] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);

						mpb &= ~(1LL << (mpp-1)); //do we really need that?<1$>
						quietcount++;
					}
				}
			}
		}
		at_b &= ~(1LL << at); 
	}

	//looking for pieces pinned by file or rank<1$>
	in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
	at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); //friendly pieces<$1>
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
		all_pp ^= ppr;
		pp = __builtin_ffsll(ppr)-1;
		bb = arg.pieceset[16] & ~ (1LL << pp);
		in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		in_k = ((bb & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		mpr = magicMovesRook[king][in_k] & magicMovesRook[at][in_at] &  ~ppr ^ (1LL << at); 
		mpr &= check_grid;

		if (__builtin_popcountll( (fr[1] ^ fr[2]) & ppr )) 
		{
			while ( __builtin_popcountll(mpr))
			{
				mpp = __builtin_ffsll(mpr)-1;
				if (fr[2] & ppr)	piecetype = 2;
				else piecetype = 1;
				//checking for check?<$1>
				//capture = (1LL << mpp) & ho[6] ? 1LL << 6 : 0LL;					
				tmp = quietcount;					
				//new move pp on mpp<$1>
				ZZZ->mdata[tmp] &= 0LL;
				ZZZ->mdata[tmp] ^= piecetype; //piecetype
				ZZZ->mdata[tmp] ^= pp << 6; //source
				ZZZ->mdata[tmp] ^= mpp << 12; //destination
				//determine captured piece<$1>
				f = (ho[1] >> mpp) & 1LL;
				mask = 0x0000000000000008;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[2] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[3] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[4] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[5] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);

				mpr &= ~(1LL << (mpp)); 
				quietcount++;
			}
		}
		//checking for pawn<1$>
		if ( fr[5] & ppr)
		{
			pp = __builtin_ffsll(ppr)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
			if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.pieceset[16]) << 8) & ~arg.pieceset[16] & check_grid);//include just two rank step<1$>
			else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.pieceset[16]) >> 8) & ~arg.pieceset[16] & check_grid);//include just two rank step<1$>
			//pawn advance or pawn capture<1$>
			//without promotion<1$>
			if (mpp)
			{
				ZZZ->mdata[quietcount] &= 0LL;
				ZZZ->mdata[quietcount] ^= 5; //piecetype
				ZZZ->mdata[quietcount] ^= pp << 6; //source
				ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination
				ZZZ->mdata[quietcount] ^= (((mpp - 1)%8) << 22) ^ (1LL << 21); //enp_file

				mpr &= ~(1LL << (mpp-1)); //do we really need that?<1$>
				quietcount++;
			}
			mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.pieceset[16]) & check_grid
				: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.pieceset[16]) & check_grid;//include just one rank step<1$>
			if (mpp)
			{
				ZZZ->mdata[quietcount] &= 0LL;
				ZZZ->mdata[quietcount] ^= 5; //piecetype
				ZZZ->mdata[quietcount] ^= pp << 6; //source
				ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination

				mpr &= ~(1LL << (mpp-1)); //do we really need that?<1$>
				quietcount++;
			}
		}
		at_r &= ~(1LL << at); 
	}

	//legal moves<1$>
	//PAWN
	fr[5] &= ~ all_pp;
	if (stm)
	{
		mP2 = ((fr[5] & 0x000000000000FF00) << 8 & ~arg.pieceset[16]) << 8 & ~arg.pieceset[16] & check_grid;
		mP1 = fr[5] << 8 & ~arg.pieceset[16] & check_grid;
		mPE = (fr[5] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[6] & check_grid;
		mPW = (fr[5] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[6] & check_grid;
		mENP = (enp & 0xFEFEFEFEFEFEFEFE & check_grid) >> 9 & fr[5] ^ (enp & 0x7F7F7F7F7F7F7F7F & check_grid) >> 7 & fr[5];
		mENP |= (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5] ^ (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5];
		mP1_prom = mP1 & promotion;
		mPE_prom = mPE & promotion;
		mPW_prom = mPW & promotion;
		mP1 &= ~promotion;
		mPE &= ~promotion;
		mPW &= ~promotion;
	}
	else 
	{
		mP2 = ((fr[5] & 0x00FF000000000000) >> 8 & ~arg.pieceset[16]) >> 8 & ~arg.pieceset[16] & check_grid;
		mP1 = fr[5] >> 8 & ~arg.pieceset[16] & check_grid;
		mPE = (fr[5] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[6] & check_grid;
		mPW = (fr[5] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[6] & check_grid;
		mENP = (enp & 0x7F7F7F7F7F7F7F7F & check_grid) << 9 & fr[5] ^ (enp & 0xFEFEFEFEFEFEFEFE & check_grid) << 7 & fr[5];
		mENP |= (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5] ^ (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5];
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
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 5; //piecetype
		ZZZ->mdata[quietcount] ^= (in - P2) << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination
		ZZZ->mdata[quietcount] ^= ((in % 8) << 22) ^ (1LL << 21); //enp_file
		mP2 &= ~(1LL << in); //do we really need that?<1$>
		quietcount++;

	}
	while (__builtin_popcountll(mP1))
	{
		in = __builtin_ffsll(mP1)-1;
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 5; //piecetype
		ZZZ->mdata[quietcount] ^= (in - P1) << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination
		mP1 &= ~(1LL << in); //do we really need that?<1$>
		quietcount++;
	}
	while (__builtin_popcountll(mPW))
	{
		//without promotion<1$>
		in = __builtin_ffsll(mPW)-1;
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 5; //piecetype
		ZZZ->mdata[quietcount] ^= (in - PW) << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination
		//determine captured piece<$1>
		f = (ho[1] >> in) & 1LL;
		mask = 0x0000000000000008;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
		f = (ho[2] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
		f = (ho[3] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
		f = (ho[4] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
		f = (ho[5] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);

		mPW &= ~(1LL << in); 
		quietcount++;
	}
	while (__builtin_popcountll(mPE))
	{
		//without promotion<1$>
		in = __builtin_ffsll(mPE)-1;
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 5; //piecetype
		ZZZ->mdata[quietcount] ^= (in - PE) << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination
		//determine captured piece<$1>
		f = (ho[1] >> in) & 1LL;
		mask = 0x0000000000000008;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
		f = (ho[2] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
		f = (ho[3] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
		f = (ho[4] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
		f = (ho[5] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);

		mPE &= ~(1LL << in); 
		quietcount++;
	}
	while (__builtin_popcountll(mENP))
	{
		in = __builtin_ffsll(mENP)-1;
		//enp check for 4-5 row pin
		bb = ~(1LL << in | (enp_P)) & arg.pieceset[16] & occupancyMaskRook[king];
		in_K = (bb * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		mR_q = magicMovesRook[king][in_K] & (ho[1] | ho[2]) & ENProw;

		if (!(mR_q))
		{
			//promotion excluded when en'passan<1$>
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 7; //piecetype
			ZZZ->mdata[quietcount] ^= in << 6; //source
			ZZZ->mdata[quietcount] ^= enp_sq << 12; //destination

			quietcount++;
		}
		mENP &= ~(1LL << in); 
	}
	while (__builtin_popcountll(mP1_prom))
	{
		in = __builtin_ffsll(mP1_prom)-1;
		for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
		{
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 5; //piecetype
			ZZZ->mdata[quietcount] ^= (in - P1) << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination
			ZZZ->mdata[quietcount] ^= it_prom ^ 0x0000000000040000; //promotion

			quietcount++;
		}
		//check is missing, write as const except stm<1$>
		mP1_prom &= ~(1LL << in); //do we really need that?<1$>
	}
	while (__builtin_popcountll(mPW_prom))
	{
		in = __builtin_ffsll(mPW_prom)-1;
		for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
		{
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 5; //piecetype
			ZZZ->mdata[quietcount] ^= (in - PW) << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination
			ZZZ->mdata[quietcount] ^= it_prom ^ 0x0000000000040000; //promotion

			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			quietcount++;
		}
		//check is missing, write as const except stm<1$>
		mPW_prom &= ~(1LL << in); 
	}
	while (__builtin_popcountll(mPE_prom))
	{
		in = __builtin_ffsll(mPE_prom)-1;
		for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
		{
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 5; //piecetype
			ZZZ->mdata[quietcount] ^= (in - PE) << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination
			ZZZ->mdata[quietcount] ^= it_prom ^ 0x0000000000040000; //promotion

			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			quietcount++;
		}
		//check is missing, write as const except stm<1$>
		mPE_prom &= ~(1LL << in); 
	}
	//NIGHT
	fr[4] &= ~ all_pp;
	while (__builtin_popcountll(fr[4]))
	{
		in_N = __builtin_ffsll(fr[4])-1;
		mN_q = movesNight[in_N] & ~arg.pieceset[16] & check_grid;
		mN_c = movesNight[in_N] & ho[6] & check_grid;

		while (__builtin_popcountll(mN_q))
		{
			in = __builtin_ffsll(mN_q)-1;
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 4; //piecetype
			ZZZ->mdata[quietcount] ^= in_N << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination

			mN_q &= ~(1LL << in);
			quietcount++;
		}
		while (__builtin_popcountll(mN_c))
		{
			in = __builtin_ffsll(mN_c)-1;
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 4; //piecetype
			ZZZ->mdata[quietcount] ^= in_N << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);

			mN_c &= ~(1LL << in); 
			quietcount++;
		}
		fr[4] &= ~(1LL << in_N); 
	}
	//BISHOP
	fr[3] &= ~ all_pp;
	while (__builtin_popcountll(fr[3]))
	{
		in_B = __builtin_ffsll(fr[3])-1;
		in = ((arg.pieceset[16] & occupancyMaskBishop[in_B]) * magicNumberBishop[in_B]) >> magicNumberShiftsBishop[in_B];
		mB_q = magicMovesBishop[in_B][in] & ~ fr[6];
		mB_c = mB_q & ho[6] & check_grid;
		mB_q &= ~ ho[6] & check_grid;

		while (__builtin_popcountll(mB_q))
		{
			in = __builtin_ffsll(mB_q)-1;
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 3; //piecetype
			ZZZ->mdata[quietcount] ^= in_B << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination

			mB_q &= ~(1LL << in);
			quietcount++;
		}
		while (__builtin_popcountll(mB_c))
		{
			in = __builtin_ffsll(mB_c)-1;
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 3; //piecetype
			ZZZ->mdata[quietcount] ^= in_B << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);

			mB_c &= ~(1LL << in); 
			quietcount++;
		}
		fr[3] &= ~(1LL << in_B); 
	}
	//ROOK
	fr[2] &= ~ all_pp;
	while (__builtin_popcountll(fr[2]))
	{
		in_R = __builtin_ffsll(fr[2])-1;
		in = ((arg.pieceset[16] & occupancyMaskRook[in_R]) * magicNumberRook[in_R]) >> magicNumberShiftsRook[in_R];
		mR_q = magicMovesRook[in_R][in] & ~ fr[6];
		mR_c = mR_q & ho[6] & check_grid;
		mR_q &= ~ ho[6] & check_grid;

		while (__builtin_popcountll(mR_q))
		{
			in = __builtin_ffsll(mR_q)-1;
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 2; //piecetype
			ZZZ->mdata[quietcount] ^= in_R << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination

			mR_q &= ~(1LL << in);
			quietcount++;

		}
		while (__builtin_popcountll(mR_c))
		{
			in = __builtin_ffsll(mR_c)-1;
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 2; //piecetype
			ZZZ->mdata[quietcount] ^= in_R << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);

			mR_c &= ~(1LL << in); 
			quietcount++;
		}
		fr[2] &= ~1LL << in_R; 
	}
	//QUEEN
	fr[1] &= ~ all_pp;
	while (__builtin_popcountll(fr[1]))
	{
		in_Q = __builtin_ffsll(fr[1])-1;
		in_R = ((arg.pieceset[16] & occupancyMaskRook[in_Q]) * magicNumberRook[in_Q]) >> magicNumberShiftsRook[in_Q];
		in_B = ((arg.pieceset[16] & occupancyMaskBishop[in_Q]) * magicNumberBishop[in_Q]) >> magicNumberShiftsBishop[in_Q];
		mQ_q = (magicMovesRook[in_Q][in_R] ^ magicMovesBishop[in_Q][in_B]) & ~ fr[6];
		mQ_c = mQ_q & ho[6] & check_grid;
		mQ_q &= ~ ho[6] & check_grid;

		while (__builtin_popcountll(mQ_q))
		{
			in = __builtin_ffsll(mQ_q)-1;
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 1; //piecetype
			ZZZ->mdata[quietcount] ^= in_Q << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination

			mQ_q &= ~(1LL << in);
			quietcount++;
		}
		while (__builtin_popcountll(mQ_c))
		{
			in = __builtin_ffsll(mQ_c)-1;
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 1; //piecetype
			ZZZ->mdata[quietcount] ^= in_Q << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);

			mQ_c &= ~(1LL << in); 
			quietcount++;

		}
		fr[1] &= ~1LL << in_Q; 
	}
	//KING
	in_K = __builtin_ffsll(fr[0])-1;
	mK_q = movesKing[in_K] & ~fr[6] & ~ho[7];
	mK_c = mK_q & ho[6];
	mK_q &= ~ ho[6];

	while (__builtin_popcountll(mK_q))
	{
		in = __builtin_ffsll(mK_q)-1;
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 0; //piecetype
		ZZZ->mdata[quietcount] ^= in_K << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination

		mK_q &= ~(1LL << in);
		quietcount++;
	}
	while (__builtin_popcountll(mK_c))
	{
		in = __builtin_ffsll(mK_c)-1;
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 0; //piecetype
		ZZZ->mdata[quietcount] ^= in_K << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination
		//determine captured piece<$1>
		f = (ho[1] >> in) & 1LL;
		mask = 0x0000000000000008;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
		f = (ho[2] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
		f = (ho[3] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
		f = (ho[4] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);
		f = (ho[5] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[quietcount] |= (ZZZ->mdata[quietcount] & ~mask) | ( -f & mask);

		mK_c &= ~(1LL << in); 
		quietcount++;

	}
	//CASTLE
	if ( arg.info & cas_bit_K && !(ho[7] & cas_at_K) && !(arg.pieceset[16] & cas_occ_K) )
	{
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 6 ^ (6 << 3);
		quietcount++;
	}
	if ( arg.info & cas_bit_Q && !(ho[7] & cas_at_Q) && !(arg.pieceset[16] & cas_occ_Q) )
	{
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 6 ^ (7 << 3);
		quietcount++;
	}

	ZZZ->quietcount = quietcount;
	ZZZ->captcount = 218;

	return quietcount ;
}

int evaluate( board arg, int draft, int color, board *rb)
{
	//if (stop) return 0;
	U64 ppb, ppr, bb, at_b, at_r, blank = 0LL, all_pp, mpb, mpr, capture, stm, enp;
	U64 check_pieces, check_grid = ~0LL;
	U64 in_N, in_B, in_R, in_Q, in_K, in;
	U64 mN_q, mN_c, mB_q, mB_c, mR_q, mR_c, mQ_q, mQ_c, mK_q, mK_c;
	U64 mP1, mP2, mPE, mPW, mENP, mP1_prom, mPE_prom, mPW_prom, it_prom;
	U64 cas_bit_K, cas_bit_Q, cas_at_K, cas_at_Q, cas_occ_K, cas_occ_Q, promotion, ENProw, enp_P;
	U64 f, mask;
	U64 *fr, *ho; 
	int in_at_b, in_at_r, king, in_at, in_k, at, pp, mpp, P1, P2, PE, PW;
	unsigned char quietcount = 0,  tmp, piecetype, captcount = 218, enp_sq;

	stm = (arg.info >> 11) & 0x0000000000000008;
	if (stm )
	{
		fr = &arg.pieceset[0];
		ho = &arg.pieceset[8];
		cas_bit_K = 0x0000000000000010;
		cas_bit_Q = 0x0000000000000020;
		cas_at_K =  0x000000000000000E;
		cas_at_Q =  0x0000000000000038;
		cas_occ_K = 0x0000000000000006;
		cas_occ_Q = 0x0000000000000070;
		promotion = 0xFF00000000000000;
		P1 = 8;
		P2 = 16;
		PE = 7;
		PW = 9;
		enp_sq = (arg.info >> 1 & 0x0000000000000007) + 40;
		enp = (1LL << enp_sq)*(arg.info & 1LL);
		enp_P = enp >> 8;
		ENProw = 0x000000FF00000000;
	}
	else
	{
		fr = &arg.pieceset[8];
		ho = &arg.pieceset[0];
		cas_bit_K = 0x0000000000000040;
		cas_bit_Q = 0x0000000000000080;
		cas_at_K =  0x0E00000000000000;
		cas_at_Q =  0x3800000000000000;
		cas_occ_K = 0x0600000000000000;
		cas_occ_Q = 0x7000000000000000;
		promotion = 0x00000000000000FF;
		P1 = -8;
		P2 = -16;
		PE = -9;
		PW = -7;
		enp_sq = (arg.info >> 1 & 0x0000000000000007) + 16;
		enp = (1LL << enp_sq)*(arg.info & 1LL);
		enp_P = enp << 8;
		ENProw = 0x00000000FF000000;
	}
	king = __builtin_ffsll(fr[0])-1;
	//building atack map pieces, later wont be necessary becuase it will be updated incrementally by make, unmake move<$1>
	fr[6] = fr[5] ^ fr[4] ^ fr[2] ^ fr[3] ^ fr[0] ^ fr[1];
	ho[6] = ho[5] ^ ho[4] ^ ho[2] ^ ho[3] ^ ho[0] ^ ho[1];
	arg.pieceset[16] = fr[6] ^ ho[6];
	ho[7] = gen_ho_atackN(arg);

	if (fr[0] & ho[7])
	{
		in_K = movesKing[king] & ~fr[6] & ~ho[7];
		//if (__builtin_popcountll(in_K) == 0) 	goto found_move;

		if (stm)	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[5];  		
		else	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[5];  		

		in_at_b = ( (arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); //friendly pieces<$1>
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); //friendly pieces<$1>

		in_N = movesNight[king] & ho[4];

		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); //friendly pieces<$1>
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
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
	}

	all_pp = 0LL;
	ppb = 0LL;
	ppr = 0LL;
	//looking for pieces pinned diagonaly<1$>
	in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
	at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); //friendly pieces<$1>
	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		ppb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at];

		all_pp ^= ppb;

		pp = __builtin_ffsll(ppb)-1;
		bb = arg.pieceset[16] & ~ (1LL << pp);
		in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		in_k = ((bb & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		mpb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at] &  ~ppb ^ (1LL << at); 
		mpb &= check_grid;

		if (__builtin_popcountll( (fr[1] ^ fr[3]) & ppb )) //maybe popcount isn't neccessary<1$>
		{
			if ( __builtin_popcountll(mpb))
				goto found_move;
		}
		//checking for pawn<1$>
		if ( fr[5] & ppb)
		{
			pp = __builtin_ffsll(ppb)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
			if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & ~promotion & check_grid);
			else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & ~promotion & check_grid );
			//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>
			//without promotion<1$>
			if (mpp)
				goto found_move;
			else 
			{
				//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>
				if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & promotion & check_grid);
				else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & promotion & check_grid );
				//promotion
				if (mpp)
					goto found_move;
				else
				{
					if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho[6]^enp) & check_grid);
					else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho[6] ^ enp) & check_grid);
					//promotion excluded when en'passan<1$>
					if (mpp)
						goto found_move;
				}
			}
		}
		at_b &= ~(1LL << at); 
	}
	//looking for pieces pinned by file or rank<1$>
	in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
	at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); //friendly pieces<$1>
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
		all_pp ^= ppr;

		pp = __builtin_ffsll(ppr)-1;
		bb = arg.pieceset[16] & ~ (1LL << pp);
		in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		in_k = ((bb & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		mpr = magicMovesRook[king][in_k] & magicMovesRook[at][in_at] &  ~ppr ^ (1LL << at); 
		mpr &= check_grid;

		if (__builtin_popcountll( (fr[1] ^ fr[2]) & ppr )) 
		{
			if ( __builtin_popcountll(mpr))
				goto found_move;
		}
		//checking for pawn<1$>
		if ( fr[5] & ppr)
		{
			pp = __builtin_ffsll(ppr)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
			if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.pieceset[16]) << 8) & ~arg.pieceset[16] & check_grid);//include just two rank step<1$>
			else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.pieceset[16]) >> 8) & ~arg.pieceset[16] & check_grid);//include just two rank step<1$>
			//pawn advance or pawn capture<1$>
			//without promotion<1$>
			if (mpp)
				goto found_move;
			//else 
			mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.pieceset[16]) & check_grid
				: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.pieceset[16]) & check_grid;//include just one rank step<1$>
			if (mpp)
				goto found_move;
		}
		at_r &= ~(1LL << at); 
	}
	//PAWN
	fr[5] &= ~ all_pp;
	if (stm)
	{
		mP2 = ((fr[5] & 0x000000000000FF00) << 8 & ~arg.pieceset[16]) << 8 & ~arg.pieceset[16] & check_grid;
		mP1 = fr[5] << 8 & ~arg.pieceset[16] & check_grid;
		mPE = (fr[5] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[6] & check_grid;
		mPW = (fr[5] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[6] & check_grid;
		mENP = (enp & 0xFEFEFEFEFEFEFEFE & check_grid) >> 9 & fr[5] ^ (enp & 0x7F7F7F7F7F7F7F7F & check_grid) >> 7 & fr[5];
		mENP |= (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5] ^ (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5];
		mP1_prom = mP1 & promotion;
		mPE_prom = mPE & promotion;
		mPW_prom = mPW & promotion;
		mP1 &= ~promotion;
		mPE &= ~promotion;
		mPW &= ~promotion;
	}
	else 
	{
		mP2 = ((fr[5] & 0x00FF000000000000) >> 8 & ~arg.pieceset[16]) >> 8 & ~arg.pieceset[16] & check_grid;
		mP1 = fr[5] >> 8 & ~arg.pieceset[16] & check_grid;
		mPE = (fr[5] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[6] & check_grid;
		mPW = (fr[5] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[6] & check_grid;
		mENP = (enp & 0x7F7F7F7F7F7F7F7F & check_grid) << 9 & fr[5] ^ (enp & 0xFEFEFEFEFEFEFEFE & check_grid) << 7 & fr[5];
		mENP |= (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5] ^ (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5];
		mP1_prom = mP1 & promotion;
		mPE_prom = mPE & promotion;
		mPW_prom = mPW & promotion;
		mP1 &= ~promotion;
		mPE &= ~promotion;
		mPW &= ~promotion;
	}

	if  (__builtin_popcountll(mP1 | mP2 | mPE | mPW | mENP) )
		goto found_move;
	//NIGHT
	fr[4] &= ~ all_pp;
	while (__builtin_popcountll(fr[4]))
	{
		in_N = __builtin_ffsll(fr[4])-1;
		mN_q = movesNight[in_N] & ~fr[6] & check_grid;

		if (__builtin_popcountll(mN_q))
			goto found_move;
		fr[4] &= ~(1LL << in_N); 
	}
	//BISHOP
	fr[3] &= ~ all_pp;
	while (__builtin_popcountll(fr[3]))
	{
		in_B = __builtin_ffsll(fr[3])-1;
		in = ((arg.pieceset[16] & occupancyMaskBishop[in_B]) * magicNumberBishop[in_B]) >> magicNumberShiftsBishop[in_B];
		mB_q = magicMovesBishop[in_B][in] & ~ fr[6] & check_grid;

		if (__builtin_popcountll(mB_q))
			goto found_move;
		fr[3] &= ~(1LL << in_B); 
	}
	//ROOK
	fr[2] &= ~ all_pp;
	while (__builtin_popcountll(fr[2]))
	{
		in_R = __builtin_ffsll(fr[2])-1;
		in = ((arg.pieceset[16] & occupancyMaskRook[in_R]) * magicNumberRook[in_R]) >> magicNumberShiftsRook[in_R];
		mR_q = magicMovesRook[in_R][in] & ~ fr[6] & check_grid;

		if (__builtin_popcountll(mR_q))
			goto found_move;
		fr[2] &= ~1LL << in_R; 
	}
	//QUEEN
	fr[1] &= ~ all_pp;
	while (__builtin_popcountll(fr[1]))
	{
		in_Q = __builtin_ffsll(fr[1])-1;
		in_R = ((arg.pieceset[16] & occupancyMaskRook[in_Q]) * magicNumberRook[in_Q]) >> magicNumberShiftsRook[in_Q];
		in_B = ((arg.pieceset[16] & occupancyMaskBishop[in_Q]) * magicNumberBishop[in_Q]) >> magicNumberShiftsBishop[in_Q];
		mQ_q = (magicMovesRook[in_Q][in_R] ^ magicMovesBishop[in_Q][in_B]) & ~ fr[6] & check_grid;

		if (__builtin_popcountll(mQ_q))
			goto found_move;
		fr[1] &= ~1LL << in_Q; 
	}
	//KING
	in_K = __builtin_ffsll(fr[0])-1;
	mK_q = movesKing[in_K] & ~fr[6] & ~ho[7];

	if (__builtin_popcountll(mK_q))
		goto found_move;

stand_pat :	;
		int score;
		if ((fr[0]&ho[7])>0)
			score = color*(-WIN - draft);
		else 
			score = 0;
		return score;	//MATE SCORE
		return 0;

found_move:
		return neval( rb);
}


U64 gen_ho_atackN( board arg)
{
	U64 in_N, in_B, in_R, in_Q, in_K, in, at;
	U64 *fr, *ho; 

	at = 0LL;

	if (arg.info >> 14 & 1LL)   //stm
	{
		fr = &arg.pieceset[0];
		ho = &arg.pieceset[8];
		at |= (ho[5] & 0xFEFEFEFEFEFEFEFE) >> 9;
		at |= (ho[5] & 0x7F7F7F7F7F7F7F7F) >> 7;
	}
	else
	{
		fr = &arg.pieceset[8];
		ho = &arg.pieceset[0];
		at |= (ho[5] & 0xFEFEFEFEFEFEFEFE) << 7;
		at |= (ho[5] & 0x7F7F7F7F7F7F7F7F) << 9;
	}
	//search squares without king cause of possible check<1$>
	arg.pieceset[16] ^= fr[0];  
	//NIGHT
	while (__builtin_popcountll(ho[4]))
	{
		in_N = __builtin_ffsll(ho[4])-1;
		at |= movesNight[in_N];
		ho[4] &= ~1LL << in_N; 
	}
	//BISHOP
	while (__builtin_popcountll(ho[3]))
	{
		in_B = __builtin_ffsll(ho[3])-1;
		in = ((arg.pieceset[16] & occupancyMaskBishop[in_B]) * magicNumberBishop[in_B]) >> magicNumberShiftsBishop[in_B];
		at |= magicMovesBishop[in_B][in];
		ho[3] &= ~1LL << in_B; 
	}
	//ROOK
	while (__builtin_popcountll(ho[2]))
	{
		in_R = __builtin_ffsll(ho[2])-1;
		in = ((arg.pieceset[16] & occupancyMaskRook[in_R]) * magicNumberRook[in_R]) >> magicNumberShiftsRook[in_R];
		at |= magicMovesRook[in_R][in] ;
		ho[2] &= ~1LL << in_R; 
	}
	//QUEEN
	while (__builtin_popcountll(ho[1]))
	{
		in_Q = __builtin_ffsll(ho[1])-1;
		in_R = ((arg.pieceset[16] & occupancyMaskRook[in_Q]) * magicNumberRook[in_Q]) >> magicNumberShiftsRook[in_Q];
		in_B = ((arg.pieceset[16] & occupancyMaskBishop[in_Q]) * magicNumberBishop[in_Q]) >> magicNumberShiftsBishop[in_Q];
		at |= (magicMovesRook[in_Q][in_R] ^ magicMovesBishop[in_Q][in_B]);
		ho[1] &= ~1LL << in_Q; 
	}
	//KING
	in_K = __builtin_ffsll(ho[0])-1;
	at |= movesKing[in_K];
	//only empty squares<1$>
	//ho[7] &= ~arg.fr[6];		
	return at;
}

char generate_movesN(Nmovelist *ZZZ, board arg)
{
	//if (stop) return 0;
	U64 ppb, ppr, bb, at_b, at_r, blank = 0LL, all_pp, mpb, mpr, capture, stm, enp;
	U64 check_pieces, check_grid = ~0LL;
	U64 in_N, in_B, in_R, in_Q, in_K, in;
	U64 mN_q, mN_c, mB_q, mB_c, mR_q, mR_c, mQ_q, mQ_c, mK_q, mK_c;
	U64 mP1, mP2, mPE, mPW, mENP, mP1_prom, mPE_prom, mPW_prom, it_prom;
	U64 cas_bit_K, cas_bit_Q, cas_at_K, cas_at_Q, cas_occ_K, cas_occ_Q, promotion, ENProw, enp_P;
	U64 f, mask;
	U64 *fr, *ho; 
	int in_at_b, in_at_r, king, in_at, in_k, at, pp, mpp, P1, P2, PE, PW;
	unsigned char quietcount = 0,  tmp, piecetype, captcount = 218, enp_sq;

	stm = (arg.info >> 11) & 0x0000000000000008;
	if (stm )
	{
		fr = &arg.pieceset[0];
		ho = &arg.pieceset[8];
		cas_bit_K = 0x0000000000000010;
		cas_bit_Q = 0x0000000000000020;
		cas_at_K =  0x000000000000000E;
		cas_at_Q =  0x0000000000000038;
		cas_occ_K = 0x0000000000000006;
		cas_occ_Q = 0x0000000000000070;
		promotion = 0xFF00000000000000;
		P1 = 8;
		P2 = 16;
		PE = 7;
		PW = 9;
		enp_sq = (arg.info >> 1 & 0x0000000000000007) + 40;
		enp = (1LL << enp_sq)*(arg.info & 1LL);
		enp_P = enp >> 8;
		ENProw = 0x000000FF00000000;
	}
	else
	{
		fr = &arg.pieceset[8];
		ho = &arg.pieceset[0];
		cas_bit_K = 0x0000000000000040;
		cas_bit_Q = 0x0000000000000080;
		cas_at_K =  0x0E00000000000000;
		cas_at_Q =  0x3800000000000000;
		cas_occ_K = 0x0600000000000000;
		cas_occ_Q = 0x7000000000000000;
		promotion = 0x00000000000000FF;
		P1 = -8;
		P2 = -16;
		PE = -9;
		PW = -7;
		enp_sq = (arg.info >> 1 & 0x0000000000000007) + 16;
		enp = (1LL << enp_sq)*(arg.info & 1LL);
		enp_P = enp << 8;
		ENProw = 0x00000000FF000000;
	}
	king = __builtin_ffsll(fr[0])-1;

	//building atack map pieces, later wont be necessary becuase it will be updated incrementally by make, unmake move<$1>
	fr[6] = fr[5] ^ fr[4] ^ fr[2] ^ fr[3] ^ fr[0] ^ fr[1];
	ho[6] = ho[5] ^ ho[4] ^ ho[2] ^ ho[3] ^ ho[0] ^ ho[1];
	arg.pieceset[16] = fr[6] ^ ho[6];
	ho[7] = gen_ho_atackN(arg);

	//save (enp, cast, hm, stm) for undo<1$>
	ZZZ->undo = arg.info; 
	ZZZ->old_zobrist = arg.zobrist;
	//check if king is in CHECK<$1>
	if (fr[0] & ho[7])
	{
		if (stm)	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[5];  		
		else	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[5];  		

		in_at_b = ( (arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); //friendly pieces<$1>
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); //friendly pieces<$1>

		in_N = movesNight[king] & ho[4];

		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); //friendly pieces<$1>
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
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
	}
	all_pp = 0LL;
	ppb = 0LL;
	ppr = 0LL;
	/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/	//ho[7] = 0LL; // TEMPORARY<1$>
	//bb = blank & occupancyMaskBishop[king];

	//looking for pieces pinned diagonaly<1$>
	in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
	at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); //friendly pieces<$1>
	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		ppb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at];
		all_pp ^= ppb;

		pp = __builtin_ffsll(ppb)-1;
		bb = arg.pieceset[16] & ~ (1LL << pp);
		in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		in_k = ((bb & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		mpb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at] &  ~ppb ^ (1LL << at); 
		mpb &= check_grid;

		if (__builtin_popcountll( (fr[1] ^ fr[3]) & ppb )) //maybe popcount isn't neccessary<1$>
		{
			while ( __builtin_popcountll(mpb))
			{
				mpp = __builtin_ffsll(mpb)-1;
				if (fr[3] & ppb)	piecetype = 3;
				else piecetype = 1;

				//checking for check?<$1>
				tmp = (1LL << mpp) & ho[6] ? captcount : quietcount;					

				//new move pp on mpp<$1>
				ZZZ->mdata[tmp] &= 0LL;
				ZZZ->mdata[tmp] ^= piecetype; //piecetype
				ZZZ->mdata[tmp] ^= pp << 6; //source
				ZZZ->mdata[tmp] ^= mpp << 12; //destination
				//determine captured piece<$1>
				f = (ho[1] >> mpp) & 1LL;
				mask = 0x0000000000000008;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[2] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[3] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[4] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[5] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);

				mpb &= ~(1LL << (mpp)); 
				(tmp == captcount) ? captcount++ : quietcount++;
			}
		}
		//checking for pawn<1$>
		if ( fr[5] & ppb)
		{
			pp = __builtin_ffsll(ppb)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
			if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & ~promotion & check_grid);
			else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & ~promotion & check_grid );
			//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>

			if (mpp)
			{
				//without promotion<1$>
				ZZZ->mdata[captcount] &= 0LL;
				ZZZ->mdata[captcount] ^= 5; //piecetype
				ZZZ->mdata[captcount] ^= pp << 6; //source
				ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
				//determine captured piece<$1>
				f = (ho[1] >> (mpp-1)) & 1LL;
				mask = 0x0000000000000008;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
				f = (ho[2] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
				f = (ho[3] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
				f = (ho[4] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
				f = (ho[5] >> (mpp-1)) & 1LL;
				mask += 8;
				ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

				mpb &= ~(1LL << (mpp-1)); //do we really need that?<1$>
				captcount++;
			}
			else 
			{
				//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>
				if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & promotion & check_grid);
				else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & promotion & check_grid );
				if (mpp)
				{
					//promotion 
					for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
					{
						ZZZ->mdata[captcount] &= 0LL;
						ZZZ->mdata[captcount] ^= 5; //piecetype
						ZZZ->mdata[captcount] ^= pp << 6; //source
						ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
						ZZZ->mdata[captcount] ^= it_prom ^ 0x0000000000040000; //promotion

						//determine captured piece<$1>
						f = (ho[1] >> (mpp-1)) & 1LL;
						mask = 0x0000000000000008;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[2] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[3] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[4] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[5] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

						captcount++;
					}
					//check is missing, write as const except stm<1$>
					mpb &= ~(1LL << (mpp-1)); //do we really need that?<1$>
				}
				else
				{
					if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho[6]^enp) & check_grid);
					else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho[6] ^ enp) & check_grid);
					if (mpp)
					{
						//promotion excluded when en'passan<1$>
						ZZZ->mdata[captcount] &= 0LL;
						ZZZ->mdata[captcount] ^= 7; //piecetype
						ZZZ->mdata[captcount] ^= pp << 6; //source
						ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
						//determine captured piece<$1>
						f = (ho[1] >> (mpp-1)) & 1LL;
						mask = 0x0000000000000008;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[2] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[3] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[4] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
						f = (ho[5] >> (mpp-1)) & 1LL;
						mask += 8;
						ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

						mpb &= ~(1LL << (mpp-1)); //do we really need that?<1$>
						captcount++;
					}
				}
			}
		}
		at_b &= ~(1LL << at); 
	}
	//looking for pieces pinned by file or rank<1$>
	in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
	at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); //friendly pieces<$1>
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
		all_pp ^= ppr;

		pp = __builtin_ffsll(ppr)-1;
		bb = arg.pieceset[16] & ~ (1LL << pp);
		in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		in_k = ((bb & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		mpr = magicMovesRook[king][in_k] & magicMovesRook[at][in_at] &  ~ppr ^ (1LL << at); 
		mpr &= check_grid;

		if (__builtin_popcountll( (fr[1] ^ fr[2]) & ppr )) 
		{
			while ( __builtin_popcountll(mpr))
			{
				mpp = __builtin_ffsll(mpr)-1;
				if (fr[2] & ppr)	piecetype = 2;
				else piecetype = 1;
				//checking for check?<$1>
				tmp = (1LL << mpp) & ho[6] ? captcount : quietcount;					

				//new move pp on mpp<$1>
				ZZZ->mdata[tmp] &= 0LL;
				ZZZ->mdata[tmp] ^= piecetype; //piecetype
				ZZZ->mdata[tmp] ^= pp << 6; //source
				ZZZ->mdata[tmp] ^= mpp << 12; //destination
				//determine captured piece<$1>
				f = (ho[1] >> mpp) & 1LL;
				mask = 0x0000000000000008;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[2] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[3] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[4] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);
				f = (ho[5] >> mpp) & 1LL;
				mask += 8;
				ZZZ->mdata[tmp] |= (ZZZ->mdata[tmp] & ~mask) | ( -f & mask);

				mpr &= ~(1LL << (mpp)); 
				(tmp == captcount) ? captcount++ : quietcount++;;
			}
		}

		//checking for pawn<1$>
		if ( fr[5] & ppr)
		{
			pp = __builtin_ffsll(ppr)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
			if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.pieceset[16]) << 8) & ~arg.pieceset[16] & check_grid);//include just two rank step<1$>
			else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.pieceset[16]) >> 8) & ~arg.pieceset[16] & check_grid);//include just two rank step<1$>
			//pawn advance or pawn capture<1$>
			//without promotion<1$>
			if (mpp)
			{
				ZZZ->mdata[quietcount] &= 0LL;
				ZZZ->mdata[quietcount] ^= 5; //piecetype
				ZZZ->mdata[quietcount] ^= pp << 6; //source
				ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination
				ZZZ->mdata[quietcount] ^= (((mpp - 1)%8) << 22) ^ (1LL << 21); //enp_file

				mpr &= ~(1LL << (mpp-1)); //do we really need that?<1$>
				quietcount++;
			}
			mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.pieceset[16]) & check_grid
				: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.pieceset[16]) & check_grid;//include just one rank step<1$>
			if (mpp)
			{
				ZZZ->mdata[quietcount] &= 0LL;
				ZZZ->mdata[quietcount] ^= 5; //piecetype
				ZZZ->mdata[quietcount] ^= pp << 6; //source
				ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination

				mpr &= ~(1LL << (mpp-1)); //do we really need that?<1$>
				quietcount++;
			}

		}
		at_r &= ~(1LL << at); 
	}
	//legal moves<1$>
	//PAWN
	fr[5] &= ~ all_pp;
	if (stm)
	{
		mP2 = ((fr[5] & 0x000000000000FF00) << 8 & ~arg.pieceset[16]) << 8 & ~arg.pieceset[16] & check_grid;
		mP1 = fr[5] << 8 & ~arg.pieceset[16] & check_grid;
		mPE = (fr[5] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[6] & check_grid;
		mPW = (fr[5] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[6] & check_grid;
		mENP = (enp & 0xFEFEFEFEFEFEFEFE & check_grid) >> 9 & fr[5] ^ (enp & 0x7F7F7F7F7F7F7F7F & check_grid) >> 7 & fr[5];
		mENP |= (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5] ^ (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5];
		mP1_prom = mP1 & promotion;
		mPE_prom = mPE & promotion;
		mPW_prom = mPW & promotion;
		mP1 &= ~promotion;
		mPE &= ~promotion;
		mPW &= ~promotion;
	}
	else 
	{
		mP2 = ((fr[5] & 0x00FF000000000000) >> 8 & ~arg.pieceset[16]) >> 8 & ~arg.pieceset[16] & check_grid;
		mP1 = fr[5] >> 8 & ~arg.pieceset[16] & check_grid;
		mPE = (fr[5] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[6] & check_grid;
		mPW = (fr[5] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[6] & check_grid;
		mENP = (enp & 0x7F7F7F7F7F7F7F7F & check_grid) << 9 & fr[5] ^ (enp & 0xFEFEFEFEFEFEFEFE & check_grid) << 7 & fr[5];
		mENP |= (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5] ^ (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5];
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
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 5; //piecetype
		ZZZ->mdata[quietcount] ^= (in - P2) << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination
		ZZZ->mdata[quietcount] ^= ((in % 8) << 22) ^ (1LL << 21); //enp_file
		mP2 &= ~(1LL << in); //do we really need that?<1$>
		quietcount++;

	}
	while (__builtin_popcountll(mP1))
	{
		in = __builtin_ffsll(mP1)-1;
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 5; //piecetype
		ZZZ->mdata[quietcount] ^= (in - P1) << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination
		mP1 &= ~(1LL << in); //do we really need that?<1$>
		quietcount++;
	}
	while (__builtin_popcountll(mPW))
	{
		//without promotion<1$>
		in = __builtin_ffsll(mPW)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 5; //piecetype
		ZZZ->mdata[captcount] ^= (in - PW) << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
		//determine captured piece<$1>
		f = (ho[1] >> in) & 1LL;
		mask = 0x0000000000000008;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[2] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[3] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[4] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[5] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

		mPW &= ~(1LL << in); 
		captcount++;
	}
	while (__builtin_popcountll(mPE))
	{
		//without promotion<1$>
		in = __builtin_ffsll(mPE)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 5; //piecetype
		ZZZ->mdata[captcount] ^= (in - PE) << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
		//determine captured piece<$1>
		f = (ho[1] >> in) & 1LL;
		mask = 0x0000000000000008;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[2] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[3] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[4] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[5] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

		mPE &= ~(1LL << in); 
		captcount++;
	}
	while (__builtin_popcountll(mENP))
	{
		in = __builtin_ffsll(mENP)-1;
		//enp check for 4-5 row pin
		bb = ~(1LL << in | (enp_P)) & arg.pieceset[16] & occupancyMaskRook[king];
		in_K = (bb * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		mR_q = magicMovesRook[king][in_K] & (ho[1] | ho[2]) & ENProw;

		if (!(mR_q))
		{
			//promotion excluded when en'passan<1$>
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 7; //piecetype
			ZZZ->mdata[captcount] ^= in << 6; //source
			ZZZ->mdata[captcount] ^= enp_sq << 12; //destination

			captcount++;
		}
		mENP &= ~(1LL << in); 
	}
	while (__builtin_popcountll(mP1_prom))
	{
		in = __builtin_ffsll(mP1_prom)-1;
		for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
		{
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 5; //piecetype
			ZZZ->mdata[quietcount] ^= (in - P1) << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination
			ZZZ->mdata[quietcount] ^= it_prom ^ 0x0000000000040000; //promotion

			quietcount++;
		}
		//check is missing, write as const except stm<1$>
		mP1_prom &= ~(1LL << in); //do we really need that?<1$>
	}
	while (__builtin_popcountll(mPW_prom))
	{
		in = __builtin_ffsll(mPW_prom)-1;
		for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
		{
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 5; //piecetype
			ZZZ->mdata[captcount] ^= (in - PW) << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			ZZZ->mdata[captcount] ^= it_prom ^ 0x0000000000040000; //promotion
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			captcount++;
		}
		//check is missing, write as const except stm<1$>
		mPW_prom &= ~(1LL << in); 
	}
	while (__builtin_popcountll(mPE_prom))
	{
		in = __builtin_ffsll(mPE_prom)-1;
		for (it_prom = 0x0000000000000000; it_prom ^ 0x0000000000200000; it_prom += 0x80000)
		{
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 5; //piecetype
			ZZZ->mdata[captcount] ^= (in - PE) << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			ZZZ->mdata[captcount] ^= it_prom ^ 0x0000000000040000; //promotion
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			captcount++;
		}
		//check is missing, write as const except stm<1$>
		mPE_prom &= ~(1LL << in); 
	}
	//NIGHT
	fr[4] &= ~ all_pp;
	while (__builtin_popcountll(fr[4]))
	{
		in_N = __builtin_ffsll(fr[4])-1;
		mN_q = movesNight[in_N] & ~arg.pieceset[16] & check_grid;
		mN_c = movesNight[in_N] & ho[6] & check_grid;

		while (__builtin_popcountll(mN_q))
		{
			in = __builtin_ffsll(mN_q)-1;
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 4; //piecetype
			ZZZ->mdata[quietcount] ^= in_N << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination

			mN_q &= ~(1LL << in);
			quietcount++;
		}
		while (__builtin_popcountll(mN_c))
		{
			in = __builtin_ffsll(mN_c)-1;
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 4; //piecetype
			ZZZ->mdata[captcount] ^= in_N << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

			mN_c &= ~(1LL << in); 
			captcount++;
		}
		fr[4] &= ~(1LL << in_N); 
	}
	//BISHOP
	fr[3] &= ~ all_pp;
	while (__builtin_popcountll(fr[3]))
	{
		in_B = __builtin_ffsll(fr[3])-1;
		in = ((arg.pieceset[16] & occupancyMaskBishop[in_B]) * magicNumberBishop[in_B]) >> magicNumberShiftsBishop[in_B];
		mB_q = magicMovesBishop[in_B][in] & ~ fr[6];
		mB_c = mB_q & ho[6] & check_grid;
		mB_q &= ~ ho[6] & check_grid;

		while (__builtin_popcountll(mB_q))
		{
			in = __builtin_ffsll(mB_q)-1;
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 3; //piecetype
			ZZZ->mdata[quietcount] ^= in_B << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination

			mB_q &= ~(1LL << in);
			quietcount++;
		}
		while (__builtin_popcountll(mB_c))
		{
			in = __builtin_ffsll(mB_c)-1;
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 3; //piecetype
			ZZZ->mdata[captcount] ^= in_B << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

			mB_c &= ~(1LL << in); 
			captcount++;
		}
		fr[3] &= ~(1LL << in_B); 
	}
	//ROOK
	fr[2] &= ~ all_pp;
	while (__builtin_popcountll(fr[2]))
	{
		in_R = __builtin_ffsll(fr[2])-1;
		in = ((arg.pieceset[16] & occupancyMaskRook[in_R]) * magicNumberRook[in_R]) >> magicNumberShiftsRook[in_R];
		mR_q = magicMovesRook[in_R][in] & ~ fr[6];
		mR_c = mR_q & ho[6] & check_grid;
		mR_q &= ~ ho[6] & check_grid;

		while (__builtin_popcountll(mR_q))
		{
			in = __builtin_ffsll(mR_q)-1;
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 2; //piecetype
			ZZZ->mdata[quietcount] ^= in_R << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination

			mR_q &= ~(1LL << in);
			quietcount++;

		}
		while (__builtin_popcountll(mR_c))
		{
			in = __builtin_ffsll(mR_c)-1;
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 2; //piecetype
			ZZZ->mdata[captcount] ^= in_R << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

			mR_c &= ~(1LL << in); 
			captcount++;
		}
		fr[2] &= ~1LL << in_R; 
	}
	//QUEEN
	fr[1] &= ~ all_pp;
	while (__builtin_popcountll(fr[1]))
	{
		in_Q = __builtin_ffsll(fr[1])-1;
		in_R = ((arg.pieceset[16] & occupancyMaskRook[in_Q]) * magicNumberRook[in_Q]) >> magicNumberShiftsRook[in_Q];
		in_B = ((arg.pieceset[16] & occupancyMaskBishop[in_Q]) * magicNumberBishop[in_Q]) >> magicNumberShiftsBishop[in_Q];
		mQ_q = (magicMovesRook[in_Q][in_R] ^ magicMovesBishop[in_Q][in_B]) & ~ fr[6];
		mQ_c = mQ_q & ho[6] & check_grid;
		mQ_q &= ~ ho[6] & check_grid;

		while (__builtin_popcountll(mQ_q))
		{
			in = __builtin_ffsll(mQ_q)-1;
			ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 1; //piecetype
			ZZZ->mdata[quietcount] ^= in_Q << 6; //source
			ZZZ->mdata[quietcount] ^= in << 12; //destination

			mQ_q &= ~(1LL << in);
			quietcount++;
		}
		while (__builtin_popcountll(mQ_c))
		{
			in = __builtin_ffsll(mQ_c)-1;
			ZZZ->mdata[captcount] &= 0LL;
			ZZZ->mdata[captcount] ^= 1; //piecetype
			ZZZ->mdata[captcount] ^= in_Q << 6; //source
			ZZZ->mdata[captcount] ^= in << 12; //destination
			//determine captured piece<$1>
			f = (ho[1] >> in) & 1LL;
			mask = 0x0000000000000008;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[2] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[3] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[4] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
			f = (ho[5] >> in) & 1LL;
			mask += 8;
			ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

			mQ_c &= ~(1LL << in); 
			captcount++;

		}
		fr[1] &= ~1LL << in_Q; 
	}
	//KING
	in_K = __builtin_ffsll(fr[0])-1;
	mK_q = movesKing[in_K] & ~fr[6] & ~ho[7];
	mK_c = mK_q & ho[6];
	mK_q &= ~ ho[6];
	while (__builtin_popcountll(mK_q))
	{
		in = __builtin_ffsll(mK_q)-1;
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 0; //piecetype
		ZZZ->mdata[quietcount] ^= in_K << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination

		mK_q &= ~(1LL << in);
		quietcount++;
	}
	while (__builtin_popcountll(mK_c))
	{
		in = __builtin_ffsll(mK_c)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 0; //piecetype
		ZZZ->mdata[captcount] ^= in_K << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
		//determine captured piece<$1>
		f = (ho[1] >> in) & 1LL;
		mask = 0x0000000000000008;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[2] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[3] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[4] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);
		f = (ho[5] >> in) & 1LL;
		mask += 8;
		ZZZ->mdata[captcount] |= (ZZZ->mdata[captcount] & ~mask) | ( -f & mask);

		mK_c &= ~(1LL << in); 
		captcount++;

	}
	//fr[0] &= ~1LL << in_K; 
	//CASTLE
	if ( arg.info & cas_bit_K && !(ho[7] & cas_at_K) && !(arg.pieceset[16] & cas_occ_K) )
	{
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 6 ^ (6 << 3);
		ZZZ->mdata[quietcount] ^= king << 6; //source
		ZZZ->mdata[quietcount] ^= (king-2) << 12; //destination

		quietcount++;
	}
	if ( arg.info & cas_bit_Q && !(ho[7] & cas_at_Q) && !(arg.pieceset[16] & cas_occ_Q) )
	{
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 6 ^ (7 << 3);
		ZZZ->mdata[quietcount] ^= king << 6; //source
		ZZZ->mdata[quietcount] ^= (king+2) << 12; //destination
		quietcount++;
	}

	ZZZ->quietcount = quietcount;
	ZZZ->captcount = captcount;

	return quietcount + captcount - 218;
}

int Ndo_move(board *b, move m)
{
	//if (stop) return 0;
	U64 stm, castle, cast_diff, hm, fm, m_cas, f_cas, CK, CQ, KS, KR, QR, hKR, hQR;
	unsigned char p_type, capt_type, prom_type, sq_from, sq_dest, enp, pindex;

	stm = (b->info >> 14) & 0x0000000000000001;
	KR = 0x0100000000000000 >> (stm*56);
	QR = 0x8000000000000000 >> (stm*56);
	//ULL used to escape left shirt warning<1$>
	hKR = 0x0000000000000001ULL << (stm*56);
	hQR = 0x0000000000000080ULL << (stm*56);
	castle = 0LL;
	p_type = m & 0x00000007;
	capt_type = m >> 3 & 0x00000007;

	if (p_type < 6)
	{

		sq_from = m >> 6 & 0x0000003F;
		sq_dest = m >> 12 & 0x0000003F;
		prom_type = m >> 19 & 0x00000003;
		b->pieceset[ p_type + ((!stm) << 3) ] &= ~(1ULL << sq_from);
		b->pieceset[ p_type + ((!stm) << 3) ] |= 1ULL << sq_dest;

		b->zobrist ^= zobrist[ stm * 6*64 + p_type * 64 + sq_from];
		b->zobrist ^= zobrist[ stm * 6*64 + p_type * 64 + sq_dest];

		//loosing castle rights if king plays or castling, posible improvement with bitwise operator<1$>
		m_cas = 0x0000000000000C0 >> (stm<<1);
		f_cas = (!p_type) | (p_type == 6);
		castle |= (castle & ~m_cas) | (-f_cas & m_cas);
		//if rook plays, loosing appropriate castling rights<1$>
		m_cas = 0x000000000000040 >> (stm<<1);
		f_cas = (p_type == 2) && ( 1LL << sq_from & KR); 
		castle |= (castle & ~m_cas) | (-f_cas & m_cas);
		m_cas = 0x000000000000080 >> (stm<<1);
		f_cas = (p_type == 2) && ( 1LL << sq_from & QR);
		castle |= (castle & ~m_cas) | (-f_cas & m_cas);
		//loosing castle rights when rook is captured<1$>
		//not checked if mask is bigger than condition<1$>
		m_cas = 0x000000000000010 << (stm<<1);
		f_cas = (1LL << sq_dest & hKR) && ( capt_type == 2 );
		castle |= (castle & ~m_cas) | (-f_cas & m_cas);

		m_cas = 0x000000000000020 << (stm<<1);
		f_cas = (1LL << sq_dest & hQR) && ( capt_type == 2 );
		castle |= (castle & ~m_cas) | (-f_cas & m_cas);

		//removing captured piece
		b->pieceset[capt_type + ((stm) << 3)] &= ~(1LL << sq_dest);

		//zobrist is null when capt_type is 0<1$>
		b->zobrist ^= capt_type ? zobrist[ !stm * 6*64 + capt_type * 64 + 64 + sq_dest] : 0ULL;

		//promotion
		b->pieceset[prom_type + ((!stm) << 3) + 1] |= (m >> 18 & 1LL) << sq_dest;
		b->pieceset[5 + ((!stm) << 3)] &= ~((m >> 18 & 1LL) << sq_dest);

		//!!!!!!!!!!!! zobrist treba biti 0 kad nema promotion tj. m >> 18 je 0 !!!!!
		b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + prom_type * 64 + 64 + sq_dest] : 0ULL;
		b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + 5 * 64 + sq_dest] : 0ULL;

		//update all_p
		b->pieceset[16] &= ~(1LL << sq_from);
		b->pieceset[16] |= 1LL << sq_dest;

	}
	else if (p_type == 7)
	{
		sq_from = m >> 6 & 0x0000003F;
		sq_dest = m >> 12 & 0x0000003F;

		b->pieceset[ 5 + ((!stm) << 3) ] &= ~(1LL << sq_from);
		b->pieceset[ 5 + ((!stm) << 3) ] |= 1LL << sq_dest;
		b->pieceset[ 5 + ((stm) << 3) ] &= ~(1LL << (sq_dest + 8 - 16*stm));

		b->pieceset[16] &= ~(1LL << sq_from);
		b->pieceset[16] |= 1LL << sq_dest;
		b->pieceset[16] &= ~(1LL << (sq_dest + 8 - 16*stm));

		//zobrist
		b->zobrist ^= zobrist[ stm * 6*64 + 5 * 64 + sq_dest];
		b->zobrist ^= zobrist[ stm * 6*64 + 5 * 64 + sq_from];
		b->zobrist ^= zobrist[ !stm * 6*64 + 5 * 64 + sq_dest + 8 - stm*16 ];
	}
	else if (capt_type == 6)
	{
		CK = 0x0200000000000000 >> stm*56;
		CQ = 0x2000000000000000 >> stm*56;
		KS = 0x0800000000000000 >> stm*56;

		castle = 0x0000000000000C0 >> (stm*2);
		b->pieceset[0 + ((!stm) << 3)] &= ~KS;
		b->pieceset[0 + ((!stm) << 3)] |= CK;
		b->pieceset[2 + ((!stm) << 3)] &= ~KR;
		b->pieceset[2 + ((!stm) << 3)] |= CK << 1;

		b->pieceset[16] &= ~KS;
		b->pieceset[16] |= CK;
		b->pieceset[16] &= ~KR;
		b->pieceset[16] |= CK << 1;

		//zobrist
		b->zobrist ^= zobrist[ stm * 6*64 + 57 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 59 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 56 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 58 - stm*56];
	}
	else if (capt_type == 7)
	{
		CK = 0x0200000000000000 >> stm*56;
		CQ = 0x2000000000000000 >> stm*56;
		KS = 0x0800000000000000 >> stm*56;

		castle = 0x0000000000000C0 >> (stm*2);
		b->pieceset[0 + ((!stm) << 3)] &= ~KS;
		b->pieceset[0 + ((!stm) << 3)] |= CQ;
		b->pieceset[2 + ((!stm) << 3)] &= ~QR;
		b->pieceset[2 + ((!stm) << 3)] |= CQ >> 1;

		b->pieceset[16] &= ~KS;
		b->pieceset[16] |= CQ;
		b->pieceset[16] &= ~QR;
		b->pieceset[16] |= CQ >> 1;

		//zobrist
		b->zobrist ^= zobrist[ stm * 6*64 + 61 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 59 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 63 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 60 - stm*56];
	}
	//increase hm if not capture or pawn move<1$>
	hm = capt_type || (p_type == 5) ? -(b->info & 0x0000000000003F00) : (1LL << 8);

	//zobrist
	cast_diff = b->info & castle;
	b->zobrist ^= zobrist[ 778] * (cast_diff >> 4 & 1LL);  
	b->zobrist ^= zobrist[ 779] * (cast_diff >> 5 & 1LL);  
	b->zobrist ^= zobrist[ 780] * (cast_diff >> 6 & 1LL);  
	b->zobrist ^= zobrist[ 781] * (cast_diff >> 7 & 1LL); 

	enp = b->info >> 1 & 0x0000000000000007;
	b->zobrist ^= b->info & 1L ? zobrist[ 768 + enp] : 0ULL;

	enp = m >> 22 & 0x0000000000000007;
	b->zobrist ^= m >> 21 & 0x0000000000000001 ? zobrist[ 768 + enp] : 0ULL;

	b->zobrist ^= zobrist[ 777];

	b->info &= ~castle; 

	b->info ^= 0x0000000000004000;//stm
	b->info ^= (b->info & 0x000000000000000F) ^ ( m >> 21 & 0x000000000000000F);//enp
	b->info += hm;//hm
	b->info += (~ stm & 1LL) << 16;//fm
}

int Nundo_move(board *b, Nmovelist *ml, move m)
{
	//if (stop) return 0;
	U64 *p, *capt_p, *prom_p, stm, empty = 0LL, hm, CK, CQ, KS, KR, QR, cast_diff;
	int p_type, sq_from, sq_dest, prom_type, enp, pindex, cindex;
	U64 capt_type, cbit, mask;
	stm = ml->undo >> 14 & 0x0000000000000001 ;
	p_type = m & 0x00000007;
	capt_type = m >> 3 & 0x0000000000000007;

	if (p_type < 6)
	{
		sq_from = m >> 6 & 0x0000003F;
		sq_dest = m >> 12 & 0x0000003F;
		prom_type = m >> 19 & 0x00000003;

		//undoing move
		b->pieceset[ p_type + ((!stm) << 3) ] &= ~(1ULL << sq_dest);
		b->pieceset[ p_type + ((!stm) << 3) ] |= 1ULL << sq_from;

		b->zobrist ^= zobrist[ stm * 6*64 + p_type * 64 + sq_from];
		b->zobrist ^= zobrist[ stm * 6*64 + p_type * 64 + sq_dest];

		//returning captured piece
		//maybe possible to avoid 2 instructions with extra capt flag<1$>
		cbit = capt_type ? 1LL : 0LL;
		b->pieceset[capt_type + ((stm) << 3)] |= cbit << sq_dest;

		//zobrist captured piece
		//zobrist is null when capt_type is 0<1$>
		b->zobrist ^= capt_type ? zobrist[ !stm * 6*64 + capt_type * 64 + 64  + sq_dest] : 0ULL;
		//promotion
		b->pieceset[prom_type + ((!stm) << 3) + 1] ^= (m >> 18 & 1LL) << sq_dest;
		//zobrist promotion piece
		b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + prom_type * 64 + 64 + sq_dest] : 0ULL;
		b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + 5 * 64 + sq_dest] : 0ULL;

		//update all_p
		b->pieceset[16] &= ~(1LL << sq_dest);
		b->pieceset[16] |= 1LL << sq_from;

	}
	else if (p_type == 7)
	{
		sq_from = m >> 6 & 0x0000003F;
		sq_dest = m >> 12 & 0x0000003F;

		b->pieceset[ 5 + ((!stm) << 3) ] &= ~(1LL << sq_dest);
		b->pieceset[ 5 + ((!stm) << 3) ] |= (1LL << sq_from);
		b->pieceset[ 5 + ((stm) << 3) ] |= 1LL << (sq_dest + 8 - 16*stm);

		b->pieceset[16] &= ~(1LL << sq_dest);
		b->pieceset[16] |= 1LL << sq_from;
		b->pieceset[16] |= (1LL << (sq_dest + 8 - 16*stm));

		//zobrist
		b->zobrist ^= zobrist[ stm * 6*64 + 5 * 64 + sq_dest];
		b->zobrist ^= zobrist[ stm * 6*64 + 5 * 64 + sq_from];
		b->zobrist ^= zobrist[ !stm * 6*64 + 5 * 64 + sq_dest + 8 - stm*16 ];

	}
	else if (capt_type == 6)
	{
		KR = 0x0100000000000000 >> stm*56;
		QR = 0x8000000000000000 >> stm*56;
		CK = 0x0200000000000000 >> stm*56;
		CQ = 0x2000000000000000 >> stm*56;
		KS = 0x0800000000000000 >> stm*56;

		b->pieceset[0 + ((!stm) << 3)] |= KS;
		b->pieceset[0 + ((!stm) << 3)] &= ~CK;
		b->pieceset[2 + ((!stm) << 3)] |= KR;
		b->pieceset[2 + ((!stm) << 3)] &= ~(CK << 1);

		b->pieceset[16] |= KS;
		b->pieceset[16] &= ~CK;
		b->pieceset[16] |= KR;
		b->pieceset[16] &= ~(CK << 1);

		//zobrist
		b->zobrist ^= zobrist[ stm * 6*64 + 57 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 59 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 56 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 58 - stm*56];
	}
	else if (capt_type == 7)
	{
		//maybe possible to use && or ?<1$>
		KR = 0x0100000000000000 >> stm*56;
		QR = 0x8000000000000000 >> stm*56;
		CK = 0x0200000000000000 >> stm*56;
		CQ = 0x2000000000000000 >> stm*56;
		KS = 0x0800000000000000 >> stm*56;

		b->pieceset[0 + ((!stm) << 3)] |= KS;
		b->pieceset[0 + ((!stm) << 3)] &= ~CQ;
		b->pieceset[2 + ((!stm) << 3)] |= QR;
		b->pieceset[2 + ((!stm) << 3)] &= ~(CQ >> 1);

		b->pieceset[16] |= KS;
		b->pieceset[16] &= ~CQ;
		b->pieceset[16] |= QR;
		b->pieceset[16] &= ~(CQ >> 1);

		//zobrist
		b->zobrist ^= zobrist[ stm * 6*64 + 61 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 59 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 63 - stm*56];
		b->zobrist ^= zobrist[ stm * 6*64 + 2*64 + 60 - stm*56];
	}

	b->info = ml->undo;
	b->zobrist = ml->old_zobrist;
}

void print_move_xboard_1(move *m)
{
	int piece_type, capt_type, from, dest, count = 0;
	short d_pa;
	char promotion, fr[3], dst[3];

	piece_type = *m & 0x0000000000000007;
	capt_type = (*m >> 3) & 0x0000000000000007;
	from = (*m >> 6) & 0x000000000000003F;
	dest = (*m >> 12) & 0x000000000000003F;
	square( dest, dst);
	square( from, fr);

	if (piece_type == 6 )
		printf("%s ", piece(capt_type));
	else if (piece_type == 7 )
		printf("%s%s\n", fr, dest);
	else	
		printf("%s%s\n", fr, dest);

}

void printmoveN(move *m)
{
	int piece_type, capt_type, from, dest, count = 0;
	short d_pa;
	char promotion, fr[3], dst[3];

	piece_type = *m & 0x0000000000000007;
	capt_type = (*m >> 3) & 0x0000000000000007;
	from = (*m >> 6) & 0x000000000000003F;
	dest = (*m >> 12) & 0x000000000000003F;
	square( dest, dst);
	square( from, fr);

	if (piece_type == 7 ) 	printf("%s%s ", fr, dst);
	else	printf("%s%s ", fr, dst);

}

void printmovedetailsN(move *ff)
{
	unsigned char p_type, capt_type, sq_from, sq_dest;
	printf("\n-----move_details-------\n");
	printmoveN(ff);
	printBits(4, ff);
	p_type = *ff & 0x00000007;
	capt_type = *ff >> 3 & 0x0000000000000007;
	sq_from = *ff >> 6 & 0x0000003F;
	sq_dest = *ff >> 12 & 0x0000003F;
	printf("p %d capt %d\n", p_type, capt_type);
	printf("fr %d dst %d \n", sq_from, sq_dest);
	printf("----------\n");
}

void print_state(board arg)
{
	char i;
	for ( i = 0; i < 17; i++)
	{
		printf("pieceset[   %d    ]\n", i);
		printBits(8, &arg.pieceset[i]);

	}
	printBits(8, &arg.info);

}
