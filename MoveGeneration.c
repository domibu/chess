#include <stdio.h>
#include <stdlib.h>
#include "BitBoardMagic.h"
#include "MoveGeneration.h"
#include "Search.h"

static inline void fill_move( Nmove *move, U64 p_type, U64 source, U64 destination);
static inline void capt_type( U64 *ho, U64 in, Nmove *move, U64 f, U64 mask);

U64 generate_captures_2(Nmovelist *ZZZ, Nboard arg)
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

	stm = (arg.info >> 11) & 0x0000000000000008;// 1 sa 14. mjesta na 3.
	if (stm )
	{
		fr = &arg.pieceset[0];
		ho = &arg.pieceset[8];
		/*cas_bit_K = 0x0000000000000010;
		cas_bit_Q = 0x0000000000000020;
		cas_at_K =  0x000000000000000E;
		cas_at_Q =  0x0000000000000038;
		cas_occ_K = 0x0000000000000006;
		cas_occ_Q = 0x0000000000000070;*/
		promotion = 0xFF00000000000000;
		P1 = 8;
		//P2 = 16;
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
		/*cas_bit_K = 0x0000000000000040;
		cas_bit_Q = 0x0000000000000080;
		cas_at_K =  0x0E00000000000000;
		cas_at_Q =  0x3800000000000000;
		cas_occ_K = 0x0600000000000000;
		cas_occ_Q = 0x7000000000000000;*/
		promotion = 0x00000000000000FF;
		P1 = -8;
		//P2 = -16;
		PE = -9;
		PW = -7;
		enp_sq = (arg.info >> 1 & 0x0000000000000007) + 16;
		enp = (1LL << enp_sq)*(arg.info & 1LL);
		enp_P = enp << 8;
		ENProw = 0x00000000FF000000;
	}

	king = __builtin_ffsll(fr[0])-1;
        
	//!!!!!!!!!!!!!!! gradnja atack map, pieces, kasnije neće biti potrebno jer će se update-ati sa make, unmake move!!!!!!!!!!!!!!!!!!!!!!!
	fr[6] = fr[5] ^ fr[4] ^ fr[2] ^ fr[3] ^ fr[0] ^ fr[1];
	ho[6] = ho[5] ^ ho[4] ^ ho[2] ^ ho[3] ^ ho[0] ^ ho[1];
	arg.pieceset[16] = fr[6] ^ ho[6];
	ho[7] = gen_ho_atackN(arg);
	
	ZZZ->undo = arg.info; //spremaj old: enp, cast, hm i stm - za undo
	ZZZ->old_zobrist = arg.zobrist;

	/////////////////////////////////////// 	provjeri jel šah	////////////////////////
	if (fr[0] & ho[7])
	{
		//printf("is_check\n");
		if (stm)	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[5];  		
		else	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[5];  		

		in_at_b = ( (arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); // ~ frijendly pieces ??
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); // ~ frijendly pieces ??
		
		in_N = movesNight[king] & ho[4];
		
		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); // ~ frijendly pieces ??
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
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

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/	//ho[7] = 0LL; // PRIVREMENO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	//////////////////////////////////	 tražim vezane figure po dijagonali	/////////////////////////////////
	in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
	at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); // ~ frijendly pieces ??

	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index kralja moze i izvan while	
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

		if (__builtin_popcountll( (fr[1] ^ fr[3]) & ppb )) // vjerovatno ne treba popcount
		{
			//printf("mpb\n");
			//printBits(8, &mpb);
			while ( __builtin_popcountll(mpb))
			{
				mpp = __builtin_ffsll(mpb)-1;
				if (fr[3] & ppb)	piecetype = 3;
				else piecetype = 1;
			
				//provjera check?
				tmp = (1LL << mpp) & ho[6] ? captcount : quietcount;					
				
				//novi potez pp na mpp
				ZZZ->mdata[tmp] &= 0LL;
				ZZZ->mdata[tmp] ^= piecetype; //piecetype
				ZZZ->mdata[tmp] ^= pp << 6; //source
				ZZZ->mdata[tmp] ^= mpp << 12; //destination
				//odredi captured piece
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

		////////////////////////////////	provjera za pješaka	/////////////////////////

		if ( fr[5] & ppb)
		{
			pp = __builtin_ffsll(ppb)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
			if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & ~promotion & check_grid);
			else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & ~promotion & check_grid );
			// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
		
			if (mpp)
			{
				// bez PROMOTION
				ZZZ->mdata[captcount] &= 0LL;
				ZZZ->mdata[captcount] ^= 5; //piecetype
				ZZZ->mdata[captcount] ^= pp << 6; //source
				ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
				//odredi captured piece
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

				mpb &= ~(1LL << (mpp-1)); //pitanje jel potrebno
				captcount++;
			}
			else 
			{
			// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
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
						
						//odredi captured piece
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
					//check fali, zapisi kao const osim stm
					mpb &= ~(1LL << (mpp-1)); //pitanje jel potrebno
				}
				else
				{
					if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho[6]^enp) & check_grid);
					else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho[6] ^ enp) & check_grid);
					if (mpp)
					{
						//promotion nije kad je en'passan
						ZZZ->mdata[captcount] &= 0LL;
						ZZZ->mdata[captcount] ^= 7; //piecetype
						ZZZ->mdata[captcount] ^= pp << 6; //source
						ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
						//odredi captured piece
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

						mpb &= ~(1LL << (mpp-1)); //pitanje jel potrebno
						captcount++;
					}
				}
			}
		}
		at_b &= ~(1LL << at); 
	}

	/////////////////////////	 tražim vezane figure po liniji i redu	//////////////////

	in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
	at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); // ~ frijendly pieces ??
	//printBits(8, &at_r);
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
		in_k = ((arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
		//ppb = magicMovesBishop[king][in_k] ;
		all_pp ^= ppr;

		pp = __builtin_ffsll(ppr)-1;
		bb = arg.pieceset[16] & ~ (1LL << pp);
		in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		in_k = ((bb & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		mpr = magicMovesRook[king][in_k] & magicMovesRook[at][in_at] &  ~ppr ^ (1LL << at); 
		mpr &= check_grid;
		//printf("mpr\n");
		//printBits(8, &mpr);
		
		if (__builtin_popcountll( (fr[1] ^ fr[2]) & ppr )) 
		{
			while ( __builtin_popcountll(mpr))
			{
				mpp = __builtin_ffsll(mpr)-1;
				if (fr[2] & ppr)	piecetype = 2;
				else piecetype = 1;
				//provjera check?
				//capture = (1LL << mpp) & ho[6] ? 1LL << 6 : 0LL;					
				tmp = (1LL << mpp) & ho[6] ? captcount : quietcount;					
				
				//novi potez pp na mpp
				ZZZ->mdata[tmp] &= 0LL;
				ZZZ->mdata[tmp] ^= piecetype; //piecetype
				ZZZ->mdata[tmp] ^= pp << 6; //source
				ZZZ->mdata[tmp] ^= mpp << 12; //destination
				//odredi captured piece
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
		//ppb &= ~ (1LL << pp);
	}
/*		//provjera za pješaka
		if ( fr[5] & ppr)
		{
			pp = __builtin_ffsll(ppr)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
			if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.pieceset[16]) << 8) & ~arg.pieceset[16] & check_grid);// za dva
			else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.pieceset[16]) >> 8) & ~arg.pieceset[16] & check_grid);// za dva
			//PITANJE DA LI PAWN ADVANCE JE QUIET ILI CAPTURE
			// bez PROMOTION
			if (mpp)
			{
				ZZZ->mdata[quietcount] &= 0LL;
				ZZZ->mdata[quietcount] ^= 5; //piecetype
				ZZZ->mdata[quietcount] ^= pp << 6; //source
				ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination
				ZZZ->mdata[quietcount] ^= (((mpp - 1)%8) << 22) ^ (1LL << 21); //enp_file

				mpr &= ~(1LL << (mpp-1)); //pitanje jel potrebno
				quietcount++;
			}
			//else 
			mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.pieceset[16]) & check_grid
					: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.pieceset[16]) & check_grid;// za  jedan 
			if (mpp)
			{
				ZZZ->mdata[quietcount] &= 0LL;
				ZZZ->mdata[quietcount] ^= 5; //piecetype
				ZZZ->mdata[quietcount] ^= pp << 6; //source
				ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination

				mpr &= ~(1LL << (mpp-1)); //pitanje jel potrebno
				quietcount++;
			}
		at_r &= ~(1LL << at); 
	}*/
	
	//legalnipotezi
	
	//PAWN
	fr[5] &= ~ all_pp;
	if (stm)
	{
	//mP2 = ((fr[5] & 0x000000000000FF00) << 8 & ~arg.pieceset[16]) << 8 & ~arg.pieceset[16] & check_grid;
	mP1 = fr[5] << 8 & ~arg.pieceset[16] & check_grid;
	mPE = (fr[5] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[6] & check_grid;
	mPW = (fr[5] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[6] & check_grid;
	mENP = (enp & 0xFEFEFEFEFEFEFEFE & check_grid) >> 9 & fr[5] ^ (enp & 0x7F7F7F7F7F7F7F7F & check_grid) >> 7 & fr[5];
	mENP |= (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5] ^ (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5];
	//mENP = 0LL;
	mP1_prom = mP1 & promotion;
	mPE_prom = mPE & promotion;
	mPW_prom = mPW & promotion;
	mP1 &= ~promotion;
	mPE &= ~promotion;
	mPW &= ~promotion;
	}
	else 
	{
	//mP2 = ((fr[5] & 0x00FF000000000000) >> 8 & ~arg.pieceset[16]) >> 8 & ~arg.pieceset[16] & check_grid;
	mP1 = fr[5] >> 8 & ~arg.pieceset[16] & check_grid;
	mPE = (fr[5] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[6] & check_grid;
	mPW = (fr[5] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[6] & check_grid;
	mENP = (enp & 0x7F7F7F7F7F7F7F7F & check_grid) << 9 & fr[5] ^ (enp & 0xFEFEFEFEFEFEFEFE & check_grid) << 7 & fr[5];
	mENP |= (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5] ^ (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5];
	//mENP = 0LL;
	mP1_prom = mP1 & promotion;
	mPE_prom = mPE & promotion;
	mPW_prom = mPW & promotion;
	mP1 &= ~promotion;
	mPE &= ~promotion;
	mPW &= ~promotion;
	//printf("P1\n");
	//printBits(8, &mP1_prom);
	}

	while (__builtin_popcountll(mPW))
	{
		// bez PROMOTION
		in = __builtin_ffsll(mPW)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 5; //piecetype
		ZZZ->mdata[captcount] ^= (in - PW) << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
      		//odredi captured piece
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
		// bez PROMOTION
		in = __builtin_ffsll(mPE)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 5; //piecetype
		ZZZ->mdata[captcount] ^= (in - PE) << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
      		//odredi captured piece
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
        		//promotion nije kad je en'passan
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
		//check fali, zapisi kao const osim stm
        	mP1_prom &= ~(1LL << in); //pitanje jel potrebno
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
           			
      		//odredi captured piece
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
		//check fali, zapisi kao const osim stm
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
           			
      		//odredi captured piece
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
		//check fali, zapisi kao const osim stm
		mPE_prom &= ~(1LL << in); 
	}
	
	
	
	
	//NIGHT
	fr[4] &= ~ all_pp;
	while (__builtin_popcountll(fr[4]))
	{
		in_N = __builtin_ffsll(fr[4])-1;
		//mN_q = movesNight[in_N] & ~arg.pieceset[16] & check_grid;
		mN_c = movesNight[in_N] & ho[6] & check_grid;
		
		while (__builtin_popcountll(mN_c))
		{
        		in = __builtin_ffsll(mN_c)-1;
        		ZZZ->mdata[captcount] &= 0LL;
        		ZZZ->mdata[captcount] ^= 4; //piecetype
        		ZZZ->mdata[captcount] ^= in_N << 6; //source
        		ZZZ->mdata[captcount] ^= in << 12; //destination
              		//odredi captured piece
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
		//mB_q &= ~ ho[6] & check_grid;
	
		while (__builtin_popcountll(mB_c))
		{
        		in = __builtin_ffsll(mB_c)-1;
        		ZZZ->mdata[captcount] &= 0LL;
        		ZZZ->mdata[captcount] ^= 3; //piecetype
        		ZZZ->mdata[captcount] ^= in_B << 6; //source
        		ZZZ->mdata[captcount] ^= in << 12; //destination
              		//odredi captured piece
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
		//mR_q &= ~ ho[6] & check_grid;
	
		while (__builtin_popcountll(mR_c))
		{
        		in = __builtin_ffsll(mR_c)-1;
        		ZZZ->mdata[captcount] &= 0LL;
        		ZZZ->mdata[captcount] ^= 2; //piecetype
        		ZZZ->mdata[captcount] ^= in_R << 6; //source
        		ZZZ->mdata[captcount] ^= in << 12; //destination
              		//odredi captured piece
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
              		//odredi captured piece
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
	//while (__builtin_popcountll(fr[0]))
	/*printf("ho7\n");
	printBits(8, &ho[7]);
	printf("ho5\n");
	printBits(8, &ho[5]);*/
	{
		in_K = __builtin_ffsll(fr[0])-1;
		mK_q = movesKing[in_K] & ~fr[6] & ~ho[7];
	//printf("mk\n");
	//printBits(8, &mK_q);
		mK_c = mK_q & ho[6];
		mK_q &= ~ ho[6];
	/*printf("mkQ\n");
	printBits(8, &mK_q);
	printf("mkC\n");
	printBits(8, &mK_c);*/
		
		while (__builtin_popcountll(mK_c))
		{
        		in = __builtin_ffsll(mK_c)-1;
        		ZZZ->mdata[captcount] &= 0LL;
        		ZZZ->mdata[captcount] ^= 0; //piecetype
        		ZZZ->mdata[captcount] ^= in_K << 6; //source
        		ZZZ->mdata[captcount] ^= in << 12; //destination
              		//odredi captured piece
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
	}

        ZZZ->quietcount = quietcount;
        ZZZ->captcount = captcount;

	return captcount - 218;
}

U64 generate_captures(Nmovelist *ZZZ, Nboard arg)
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

	stm = (arg.info >> 11) & 0x0000000000000008;// 1 sa 14. mjesta na 3.
	if (stm )
	{
		fr = &arg.pieceset[0];
		ho = &arg.pieceset[8];
		/*cas_bit_K = 0x0000000000000010;
		cas_bit_Q = 0x0000000000000020;
		cas_at_K =  0x000000000000000E;
		cas_at_Q =  0x0000000000000038;
		cas_occ_K = 0x0000000000000006;
		cas_occ_Q = 0x0000000000000070;*/
		promotion = 0xFF00000000000000;
		P1 = 8;
		//P2 = 16;
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
		/*cas_bit_K = 0x0000000000000040;
		cas_bit_Q = 0x0000000000000080;
		cas_at_K =  0x0E00000000000000;
		cas_at_Q =  0x3800000000000000;
		cas_occ_K = 0x0600000000000000;
		cas_occ_Q = 0x7000000000000000;*/
		promotion = 0x00000000000000FF;
		P1 = -8;
		//P2 = -16;
		PE = -9;
		PW = -7;
		enp_sq = (arg.info >> 1 & 0x0000000000000007) + 16;
		enp = (1LL << enp_sq)*(arg.info & 1LL);
		enp_P = enp << 8;
		ENProw = 0x00000000FF000000;
	}

	king = __builtin_ffsll(fr[0])-1;
        
	//!!!!!!!!!!!!!!! gradnja atack map, pieces, kasnije neće biti potrebno jer će se update-ati sa make, unmake move!!!!!!!!!!!!!!!!!!!!!!!
	fr[6] = fr[5] ^ fr[4] ^ fr[2] ^ fr[3] ^ fr[0] ^ fr[1];
	ho[6] = ho[5] ^ ho[4] ^ ho[2] ^ ho[3] ^ ho[0] ^ ho[1];
	arg.pieceset[16] = fr[6] ^ ho[6];
	ho[7] = gen_ho_atackN(arg);
	
	ZZZ->undo = arg.info; //spremaj old: enp, cast, hm i stm - za undo
	ZZZ->old_zobrist = arg.zobrist;

	/////////////////////////////////////// 	provjeri jel šah	////////////////////////
	if (fr[0] & ho[7])
	{
		//printf("is_check\n");
		if (stm)	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[5];  		
		else	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[5];  		

		in_at_b = ( (arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); // ~ frijendly pieces ??
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); // ~ frijendly pieces ??
		
		in_N = movesNight[king] & ho[4];
		
		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); // ~ frijendly pieces ??
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
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

/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/	//ho[7] = 0LL; // PRIVREMENO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!

	//////////////////////////////////	 tražim vezane figure po dijagonali	/////////////////////////////////
	in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
	at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); // ~ frijendly pieces ??

	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index kralja moze i izvan while	
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

		if (__builtin_popcountll( (fr[1] ^ fr[3]) & ppb )) // vjerovatno ne treba popcount
		{
			//printf("mpb\n");
			//printBits(8, &mpb);
			while ( __builtin_popcountll(mpb))
			{
				mpp = __builtin_ffsll(mpb)-1;
				if (fr[3] & ppb)	piecetype = 3;
				else piecetype = 1;
			
				//provjera check?
				tmp = (1LL << mpp) & ho[6] ? captcount : quietcount;					
				
				//novi potez pp na mpp
				ZZZ->mdata[tmp] &= 0LL;
				ZZZ->mdata[tmp] ^= piecetype; //piecetype
				ZZZ->mdata[tmp] ^= pp << 6; //source
				ZZZ->mdata[tmp] ^= mpp << 12; //destination
				//odredi captured piece
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

		////////////////////////////////	provjera za pješaka	/////////////////////////

		if ( fr[5] & ppb)
		{
			pp = __builtin_ffsll(ppb)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
			if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & ~promotion & check_grid);
			else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & ~promotion & check_grid );
			// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
		
			if (mpp)
			{
				// bez PROMOTION
				ZZZ->mdata[captcount] &= 0LL;
				ZZZ->mdata[captcount] ^= 5; //piecetype
				ZZZ->mdata[captcount] ^= pp << 6; //source
				ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
				//odredi captured piece
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

				mpb &= ~(1LL << (mpp-1)); //pitanje jel potrebno
				captcount++;
			}
			else 
			{
			// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
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
						
						//odredi captured piece
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
					//check fali, zapisi kao const osim stm
					mpb &= ~(1LL << (mpp-1)); //pitanje jel potrebno
				}
				else
				{
					if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho[6]^enp) & check_grid);
					else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho[6] ^ enp) & check_grid);
					if (mpp)
					{
						//promotion nije kad je en'passan
						ZZZ->mdata[captcount] &= 0LL;
						ZZZ->mdata[captcount] ^= 7; //piecetype
						ZZZ->mdata[captcount] ^= pp << 6; //source
						ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
						//odredi captured piece
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

						mpb &= ~(1LL << (mpp-1)); //pitanje jel potrebno
						captcount++;
					}
				}
			}
		}
		at_b &= ~(1LL << at); 
	}

	/////////////////////////	 tražim vezane figure po liniji i redu	//////////////////

	in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
	at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); // ~ frijendly pieces ??
	//printBits(8, &at_r);
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
		in_k = ((arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
		//ppb = magicMovesBishop[king][in_k] ;
		all_pp ^= ppr;

		pp = __builtin_ffsll(ppr)-1;
		bb = arg.pieceset[16] & ~ (1LL << pp);
		in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		in_k = ((bb & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		mpr = magicMovesRook[king][in_k] & magicMovesRook[at][in_at] &  ~ppr ^ (1LL << at); 
		mpr &= check_grid;
		//printf("mpr\n");
		//printBits(8, &mpr);
		
		if (__builtin_popcountll( (fr[1] ^ fr[2]) & ppr )) 
		{
			while ( __builtin_popcountll(mpr))
			{
				mpp = __builtin_ffsll(mpr)-1;
				if (fr[2] & ppr)	piecetype = 2;
				else piecetype = 1;
				//provjera check?
				//capture = (1LL << mpp) & ho[6] ? 1LL << 6 : 0LL;					
				tmp = (1LL << mpp) & ho[6] ? captcount : quietcount;					
				
				//novi potez pp na mpp
				ZZZ->mdata[tmp] &= 0LL;
				ZZZ->mdata[tmp] ^= piecetype; //piecetype
				ZZZ->mdata[tmp] ^= pp << 6; //source
				ZZZ->mdata[tmp] ^= mpp << 12; //destination
				//odredi captured piece
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
		//ppb &= ~ (1LL << pp);
	}
/*		//provjera za pješaka
		if ( fr[5] & ppr)
		{
			pp = __builtin_ffsll(ppr)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
			if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.pieceset[16]) << 8) & ~arg.pieceset[16] & check_grid);// za dva
			else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.pieceset[16]) >> 8) & ~arg.pieceset[16] & check_grid);// za dva
			//PITANJE DA LI PAWN ADVANCE JE QUIET ILI CAPTURE
			// bez PROMOTION
			if (mpp)
			{
				ZZZ->mdata[quietcount] &= 0LL;
				ZZZ->mdata[quietcount] ^= 5; //piecetype
				ZZZ->mdata[quietcount] ^= pp << 6; //source
				ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination
				ZZZ->mdata[quietcount] ^= (((mpp - 1)%8) << 22) ^ (1LL << 21); //enp_file

				mpr &= ~(1LL << (mpp-1)); //pitanje jel potrebno
				quietcount++;
			}
			//else 
			mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.pieceset[16]) & check_grid
					: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.pieceset[16]) & check_grid;// za  jedan 
			if (mpp)
			{
				ZZZ->mdata[quietcount] &= 0LL;
				ZZZ->mdata[quietcount] ^= 5; //piecetype
				ZZZ->mdata[quietcount] ^= pp << 6; //source
				ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination

				mpr &= ~(1LL << (mpp-1)); //pitanje jel potrebno
				quietcount++;
			}
		at_r &= ~(1LL << at); 
	}*/
	
	//legalnipotezi
	
	//PAWN
	fr[5] &= ~ all_pp;
	if (stm)
	{
	//mP2 = ((fr[5] & 0x000000000000FF00) << 8 & ~arg.pieceset[16]) << 8 & ~arg.pieceset[16] & check_grid;
	mP1 = fr[5] << 8 & ~arg.pieceset[16] & check_grid;
	mPE = (fr[5] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[6] & check_grid;
	mPW = (fr[5] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[6] & check_grid;
	mENP = (enp & 0xFEFEFEFEFEFEFEFE & check_grid) >> 9 & fr[5] ^ (enp & 0x7F7F7F7F7F7F7F7F & check_grid) >> 7 & fr[5];
	mENP |= (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5] ^ (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5];
	//mENP = 0LL;
	mP1_prom = mP1 & promotion;
	mPE_prom = mPE & promotion;
	mPW_prom = mPW & promotion;
	mP1 &= ~promotion;
	mPE &= ~promotion;
	mPW &= ~promotion;
	}
	else 
	{
	//mP2 = ((fr[5] & 0x00FF000000000000) >> 8 & ~arg.pieceset[16]) >> 8 & ~arg.pieceset[16] & check_grid;
	mP1 = fr[5] >> 8 & ~arg.pieceset[16] & check_grid;
	mPE = (fr[5] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[6] & check_grid;
	mPW = (fr[5] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[6] & check_grid;
	mENP = (enp & 0x7F7F7F7F7F7F7F7F & check_grid) << 9 & fr[5] ^ (enp & 0xFEFEFEFEFEFEFEFE & check_grid) << 7 & fr[5];
	mENP |= (enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5] ^ (enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5];
	//mENP = 0LL;
	mP1_prom = mP1 & promotion;
	mPE_prom = mPE & promotion;
	mPW_prom = mPW & promotion;
	mP1 &= ~promotion;
	mPE &= ~promotion;
	mPW &= ~promotion;
	//printf("P1\n");
	//printBits(8, &mP1_prom);
	}

	while (__builtin_popcountll(mPW))
	{
		// bez PROMOTION
		in = __builtin_ffsll(mPW)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 5; //piecetype
		ZZZ->mdata[captcount] ^= (in - PW) << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
      		//odredi captured piece
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
		// bez PROMOTION
		in = __builtin_ffsll(mPE)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 5; //piecetype
		ZZZ->mdata[captcount] ^= (in - PE) << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
      		//odredi captured piece
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
        		//promotion nije kad je en'passan
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
		//check fali, zapisi kao const osim stm
        	mP1_prom &= ~(1LL << in); //pitanje jel potrebno
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
           			
      		//odredi captured piece
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
		//check fali, zapisi kao const osim stm
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
           			
      		//odredi captured piece
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
		//check fali, zapisi kao const osim stm
		mPE_prom &= ~(1LL << in); 
	}
	
	
	
	
	//NIGHT
	fr[4] &= ~ all_pp;
	while (__builtin_popcountll(fr[4]))
	{
		in_N = __builtin_ffsll(fr[4])-1;
		//mN_q = movesNight[in_N] & ~arg.pieceset[16] & check_grid;
		mN_c = movesNight[in_N] & ho[6] & check_grid;
		
		while (__builtin_popcountll(mN_c))
		{
        		in = __builtin_ffsll(mN_c)-1;
        		ZZZ->mdata[captcount] &= 0LL;
        		ZZZ->mdata[captcount] ^= 4; //piecetype
        		ZZZ->mdata[captcount] ^= in_N << 6; //source
        		ZZZ->mdata[captcount] ^= in << 12; //destination
              		//odredi captured piece
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
		//mB_q &= ~ ho[6] & check_grid;
	
		while (__builtin_popcountll(mB_c))
		{
        		in = __builtin_ffsll(mB_c)-1;
        		ZZZ->mdata[captcount] &= 0LL;
        		ZZZ->mdata[captcount] ^= 3; //piecetype
        		ZZZ->mdata[captcount] ^= in_B << 6; //source
        		ZZZ->mdata[captcount] ^= in << 12; //destination
              		//odredi captured piece
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
		//mR_q &= ~ ho[6] & check_grid;
	
		while (__builtin_popcountll(mR_c))
		{
        		in = __builtin_ffsll(mR_c)-1;
        		ZZZ->mdata[captcount] &= 0LL;
        		ZZZ->mdata[captcount] ^= 2; //piecetype
        		ZZZ->mdata[captcount] ^= in_R << 6; //source
        		ZZZ->mdata[captcount] ^= in << 12; //destination
              		//odredi captured piece
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
              		//odredi captured piece
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
	//while (__builtin_popcountll(fr[0]))
	/*printf("ho7\n");
	printBits(8, &ho[7]);
	printf("ho5\n");
	printBits(8, &ho[5]);*/
	{
		in_K = __builtin_ffsll(fr[0])-1;
		mK_q = movesKing[in_K] & ~fr[6] & ~ho[7];
	//printf("mk\n");
	//printBits(8, &mK_q);
		mK_c = mK_q & ho[6];
		mK_q &= ~ ho[6];
	/*printf("mkQ\n");
	printBits(8, &mK_q);
	printf("mkC\n");
	printBits(8, &mK_c);*/
		
		while (__builtin_popcountll(mK_c))
		{
        		in = __builtin_ffsll(mK_c)-1;
        		ZZZ->mdata[captcount] &= 0LL;
        		ZZZ->mdata[captcount] ^= 0; //piecetype
        		ZZZ->mdata[captcount] ^= in_K << 6; //source
        		ZZZ->mdata[captcount] ^= in << 12; //destination
              		//odredi captured piece
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
	}

        ZZZ->quietcount = quietcount;
        ZZZ->captcount = captcount;

	return captcount - 218;
}

// a definition using static inline
static inline int max(int a, int b) {
  return a > b ? a : b;
}

static inline void fill_move( Nmove *move, U64 p_type, U64 source, U64 destination)
{
	*move &= 0ULL;
	*move ^= p_type; //piecetype
	*move ^= source << 6; //source
	*move ^= destination << 12; //destination
} 

static inline void capt_type( U64 *ho, U64 in, Nmove *move, U64 f, U64 mask)
{
	//odredi captured piece
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

char generate_movesN_test(Nmovelist *ZZZ, Nboard arg)
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
	//move *m_list = NULL, *m;
	//if arg.info  
        	/*printf("Squiet count: %d	\n", quietcount);
        	printf("Scapt count: %d	\n", captcount);
        	printf("Smoves count: %d	\n", quietcount + 255 - captcount);*/

	stm = (arg.info >> 11) & 0x0000000000000008;// 1 sa 14. mjesta na 3.
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
	/*printf("enp\n");
	printBits(8, &enp);*/

	king = __builtin_ffsll(fr[0])-1;
        
	//!!!!!!!!!!!!!!! gradnja atack map, pieces, kasnije neće biti potrebno jer će se update-ati sa make, unmake move!!!!!!!!!!!!!!!!!!!!!!!
	fr[6] = fr[5] ^ fr[4] ^ fr[2] ^ fr[3] ^ fr[0] ^ fr[1];
	ho[6] = ho[5] ^ ho[4] ^ ho[2] ^ ho[3] ^ ho[0] ^ ho[1];
	arg.pieceset[16] = fr[6] ^ ho[6];
	ho[7] = gen_ho_atackN(arg);
	
	//print_state(arg);
				/*printf("frK\n");
				printBits(8, &fr[0]);
				printf("hoK\n");
				printBits(8, &ho[0]);
				printf("fr[6]\n");
				printBits(8, &fr[6]);*/
	
	ZZZ->undo = arg.info; //spremaj old: enp, cast, hm i stm - za undo
	ZZZ->old_zobrist = arg.zobrist;
	// provjeri jel šah
	if (fr[0] & ho[7])
	{
		//printf("is_check\n");
		if (stm)	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[5];  		
		else	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[5];  		

		in_at_b = ( (arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); // ~ frijendly pieces ??
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); // ~ frijendly pieces ??
		
		in_N = movesNight[king] & ho[4];
		
		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); // ~ frijendly pieces ??
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
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

		/*in_K = __builtin_ffsll(fr[0])-1;
		mK_q = movesKing[in_K] & ~fr[6] & ~ho[7];
		mK_c = mK_q & ho[6];
		mK_q &= ~ ho[6];
		while (__builtin_popcountll(mK_q))
		{
			in = __builtin_ffsll(mK_q)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			movearray[movecount].next = m;
			movearray[movecount].from = (1LL << in_K);
			movearray[movecount].dest = (1LL << in);
			ZZZ->mdata[movecount] ^= 1LL  ^ stm;
			//check fali, zapisi kao const osim stm
			mK_q &= ~movearray[movecount].dest; 
		}
		while (__builtin_popcountll(mK_c))
		{
			in = __builtin_ffsll(mK_c)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			movearray[movecount].next = m;
			movearray[movecount].from = (1LL << in_K);
			movearray[movecount].dest = (1LL << in);
			ZZZ->mdata[movecount] ^= 1LL ^ (1LL << 6) ^ stm;
			//check fali, zapisi kao const osim stm
			mK_c &= ~movearray[movecount].dest; 
		}*/
	}
	


	all_pp = 0LL;
	ppb = 0LL;
	ppr = 0LL;
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/	//ho[7] = 0LL; // PRIVREMENO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//bb = blank & occupancyMaskBishop[king];
	//printf("gen_m\n");

	// tražim vezane figure po dijagonali
	in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
	at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); // ~ frijendly pieces ??
	//printBits(8, &at_b);
	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index kralja moze i izvan while	
		in_k = ((arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		ppb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at];
		//ppb = magicMovesBishop[king][in_k] ;
		all_pp ^= ppb;
		//while (__builtin_popcountll(ppb))
		{
			pp = __builtin_ffsll(ppb)-1;
			bb = arg.pieceset[16] & ~ (1LL << pp);
			in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
			in_k = ((bb & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
			mpb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at] &  ~ppb ^ (1LL << at); 
			mpb &= check_grid;

			if (__builtin_popcountll( (fr[1] ^ fr[3]) & ppb )) // vjerovatno ne treba popcount
			{
				//printf("mpb\n");
				//printBits(8, &mpb);
				while ( __builtin_popcountll(mpb))
				{
					mpp = __builtin_ffsll(mpb)-1;
					if (fr[3] & ppb)	piecetype = 3;
					else piecetype = 1;
				
					//provjera check?
					//tmp = (1LL << mpp) & ho[6] ? captcount : quietcount;					
					tmp = quietcount;					

					//novi potez pp na mpp
					ZZZ->mdata[tmp] &= 0LL;
					ZZZ->mdata[tmp] ^= piecetype; //piecetype
					ZZZ->mdata[tmp] ^= pp << 6; //source
					ZZZ->mdata[tmp] ^= mpp << 12; //destination
                        		//odredi captured piece
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
			//provjera za pješaka
			if ( fr[5] & ppb)
			{
				pp = __builtin_ffsll(ppb)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
				if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & ~promotion & check_grid);
				else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & ~promotion & check_grid );
				// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
			
				if (mpp)
				{
					// bez PROMOTION
					ZZZ->mdata[quietcount] &= 0LL;
					ZZZ->mdata[quietcount] ^= 5; //piecetype
					ZZZ->mdata[quietcount] ^= pp << 6; //source
					ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination
                        		//odredi captured piece
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

					mpb &= ~(1LL << (mpp-1)); //pitanje jel potrebno
					quietcount++;
				}
				else 
				{
				// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
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
		                			
                                        		//odredi captured piece
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
						//check fali, zapisi kao const osim stm
        					mpb &= ~(1LL << (mpp-1)); //pitanje jel potrebno
					}
					else
					{
						if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho[6]^enp) & check_grid);
						else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho[6] ^ enp) & check_grid);
						if (mpp)
						{
							//promotion nije kad je en'passan
					                ZZZ->mdata[quietcount] &= 0LL;
                					ZZZ->mdata[quietcount] ^= 7; //piecetype
		                			ZZZ->mdata[quietcount] ^= pp << 6; //source
		                			ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination
                                        		//odredi captured piece
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
        
                					mpb &= ~(1LL << (mpp-1)); //pitanje jel potrebno
		                			quietcount++;
						}
					}
				}
			}
		}
		at_b &= ~(1LL << at); 
	}

	// tražim vezane figure po liniji i redu
	in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
	at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); // ~ frijendly pieces ??
	//printBits(8, &at_r);
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
		in_k = ((arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
		//ppb = magicMovesBishop[king][in_k] ;
		all_pp ^= ppr;
		//while (__builtin_popcountll(ppb))
		{
			pp = __builtin_ffsll(ppr)-1;
			bb = arg.pieceset[16] & ~ (1LL << pp);
			in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
			in_k = ((bb & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
			mpr = magicMovesRook[king][in_k] & magicMovesRook[at][in_at] &  ~ppr ^ (1LL << at); 
			mpr &= check_grid;
			//printf("mpr\n");
			//printBits(8, &mpr);
			
			if (__builtin_popcountll( (fr[1] ^ fr[2]) & ppr )) 
			{
				while ( __builtin_popcountll(mpr))
				{
					mpp = __builtin_ffsll(mpr)-1;
					if (fr[2] & ppr)	piecetype = 2;
					else piecetype = 1;
					//provjera check?
					//capture = (1LL << mpp) & ho[6] ? 1LL << 6 : 0LL;					
					tmp = quietcount;					
					
					//novi potez pp na mpp
					ZZZ->mdata[tmp] &= 0LL;
					ZZZ->mdata[tmp] ^= piecetype; //piecetype
					ZZZ->mdata[tmp] ^= pp << 6; //source
					ZZZ->mdata[tmp] ^= mpp << 12; //destination
                        		//odredi captured piece
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
			//ppb &= ~ (1LL << pp);
		}
			//provjera za pješaka
			if ( fr[5] & ppr)
			{
				pp = __builtin_ffsll(ppr)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
				if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.pieceset[16]) << 8) & ~arg.pieceset[16] & check_grid);// za dva
				else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.pieceset[16]) >> 8) & ~arg.pieceset[16] & check_grid);// za dva
                                //PITANJE DA LI PAWN ADVANCE JE QUIET ILI CAPTURE
				// bez PROMOTION
				if (mpp)
				{
					ZZZ->mdata[quietcount] &= 0LL;
					ZZZ->mdata[quietcount] ^= 5; //piecetype
					ZZZ->mdata[quietcount] ^= pp << 6; //source
					ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination
					ZZZ->mdata[quietcount] ^= (((mpp - 1)%8) << 22) ^ (1LL << 21); //enp_file

					mpr &= ~(1LL << (mpp-1)); //pitanje jel potrebno
					quietcount++;
				}
				//else 
				mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.pieceset[16]) & check_grid
						: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.pieceset[16]) & check_grid;// za  jedan 
				if (mpp)
				{
					ZZZ->mdata[quietcount] &= 0LL;
					ZZZ->mdata[quietcount] ^= 5; //piecetype
					ZZZ->mdata[quietcount] ^= pp << 6; //source
					ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination

					mpr &= ~(1LL << (mpp-1)); //pitanje jel potrebno
					quietcount++;
				}
				
			}
		at_r &= ~(1LL << at); 
	}
	//ppb =  magicMovesBishop[at][in_at];
	//printf("all_pp\n");
	//printBits(8, &all_pp);
	
	//legalnipotezi
	
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
	//mENP = 0LL;
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
	//mENP = 0LL;
	mP1_prom = mP1 & promotion;
	mPE_prom = mPE & promotion;
	mPW_prom = mPW & promotion;
	mP1 &= ~promotion;
	mPE &= ~promotion;
	mPW &= ~promotion;
	//printf("P1\n");
	//printBits(8, &mP1_prom);
	}

	while (__builtin_popcountll(mP2))
	{
		in = __builtin_ffsll(mP2)-1;
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 5; //piecetype
		ZZZ->mdata[quietcount] ^= (in - P2) << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination
		ZZZ->mdata[quietcount] ^= ((in % 8) << 22) ^ (1LL << 21); //enp_file
		mP2 &= ~(1LL << in); //pitanje jel potrebno
		quietcount++;

	}
	while (__builtin_popcountll(mP1))
	{
		in = __builtin_ffsll(mP1)-1;
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 5; //piecetype
		ZZZ->mdata[quietcount] ^= (in - P1) << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination
		mP1 &= ~(1LL << in); //pitanje jel potrebno
		quietcount++;
	}
	while (__builtin_popcountll(mPW))
	{
		// bez PROMOTION
		in = __builtin_ffsll(mPW)-1;
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 5; //piecetype
		ZZZ->mdata[quietcount] ^= (in - PW) << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination
      		//odredi captured piece
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
		// bez PROMOTION
		in = __builtin_ffsll(mPE)-1;
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 5; //piecetype
		ZZZ->mdata[quietcount] ^= (in - PE) << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination
      		//odredi captured piece
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
        		//promotion nije kad je en'passan
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
		//check fali, zapisi kao const osim stm
        	mP1_prom &= ~(1LL << in); //pitanje jel potrebno
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
           			
      		//odredi captured piece
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
		//check fali, zapisi kao const osim stm
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
           			
      		//odredi captured piece
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
		//check fali, zapisi kao const osim stm
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
              		//odredi captured piece
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
              		//odredi captured piece
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
              		//odredi captured piece
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
              		//odredi captured piece
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
	//while (__builtin_popcountll(fr[0]))
	/*printf("ho7\n");
	printBits(8, &ho[7]);
	printf("ho5\n");
	printBits(8, &ho[5]);*/
	{
		in_K = __builtin_ffsll(fr[0])-1;
		mK_q = movesKing[in_K] & ~fr[6] & ~ho[7];
	//printf("mk\n");
	//printBits(8, &mK_q);
		mK_c = mK_q & ho[6];
		mK_q &= ~ ho[6];
	/*printf("mkQ\n");
	printBits(8, &mK_q);
	printf("mkC\n");
	printBits(8, &mK_c);*/
		
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
              		//odredi captured piece
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
		//fr[0] &= ~1LL << in_K; 
	}
	//CASTLE
	if ( arg.info & cas_bit_K && !(ho[7] & cas_at_K) && !(arg.pieceset[16] & cas_occ_K) )
	{
			//printf("casK	\n");
        		ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 6 ^ (6 << 3);
			quietcount++;
	}
	if ( arg.info & cas_bit_Q && !(ho[7] & cas_at_Q) && !(arg.pieceset[16] & cas_occ_Q) )
	{
	//printf("casQ\n");
        		ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 6 ^ (7 << 3);
			quietcount++;
	}
	//potrebno je odrolati pinned pieces 
	
	
	/*printf("ho.atackmap\n");
	printBits(8, &ho[7]);
	printf("checkgrid\n");
	printBits(8, &check_grid);*/
	
        	/*printf("quiet count: %d	\n", quietcount);
        	printf("capt count: %d	\n", 255-captcount);
        	printf("moves count: %d	\n", quietcount + 255 -captcount);*/

        ZZZ->quietcount = quietcount;
        ZZZ->captcount = 218;

	return quietcount ;
}

int evaluate( Nboard arg, int draft, int color, Nboard *rb)
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

	stm = (arg.info >> 11) & 0x0000000000000008;// 1 sa 14. mjesta na 3.
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
        
	//!!!!!!!!!!!!!!! gradnja atack map, pieces, kasnije neće biti potrebno jer će se update-ati sa make, unmake move!!!!!!!!!!!!!!!!!!!!!!!
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
		at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); // ~ frijendly pieces ??
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); // ~ frijendly pieces ??
		
		in_N = movesNight[king] & ho[4];
		
		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); // ~ frijendly pieces ??
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
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

	// tražim vezane figure po dijagonali
	in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
	at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); // ~ frijendly pieces ??
	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index kralja moze i izvan while	
		in_k = ((arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		ppb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at];

		all_pp ^= ppb;

			pp = __builtin_ffsll(ppb)-1;
			bb = arg.pieceset[16] & ~ (1LL << pp);
			in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
			in_k = ((bb & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
			mpb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at] &  ~ppb ^ (1LL << at); 
			mpb &= check_grid;

			if (__builtin_popcountll( (fr[1] ^ fr[3]) & ppb )) // vjerovatno ne treba popcount
			{
				if ( __builtin_popcountll(mpb))
					goto found_move;
			}
			//provjera za pješaka
			if ( fr[5] & ppb)
			{
				pp = __builtin_ffsll(ppb)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
				if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & ~promotion & check_grid);
				else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & ~promotion & check_grid );
				// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
			
						// bez PROMOTION
				if (mpp)
					goto found_move;
				else 
				{
				// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
					if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & promotion & check_grid);
					else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & promotion & check_grid );
						//promotion
					if (mpp)
	                                        goto found_move;
					else
					{
						if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho[6]^enp) & check_grid);
						else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho[6] ^ enp) & check_grid);
							//promotion nije kad je en'passan
						if (mpp)
							goto found_move;
					}
				}
			}
		at_b &= ~(1LL << at); 
	}

	// tražim vezane figure po liniji i redu
	in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
	at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); // ~ frijendly pieces ??
	//printBits(8, &at_r);
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
		in_k = ((arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
		//ppb = magicMovesBishop[king][in_k] ;
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
		
			//provjera za pješaka
			if ( fr[5] & ppr)
			{
				pp = __builtin_ffsll(ppr)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
				if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.pieceset[16]) << 8) & ~arg.pieceset[16] & check_grid);// za dva
				else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.pieceset[16]) >> 8) & ~arg.pieceset[16] & check_grid);// za dva
                                //PITANJE DA LI PAWN ADVANCE JE QUIET ILI CAPTURE
				// bez PROMOTION
				if (mpp)
					goto found_move;
				//else 
				mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.pieceset[16]) & check_grid
						: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.pieceset[16]) & check_grid;// za  jedan 
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
	//mENP = 0LL;
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
	//mENP = 0LL;
	mP1_prom = mP1 & promotion;
	mPE_prom = mPE & promotion;
	mPW_prom = mPW & promotion;
	mP1 &= ~promotion;
	mPE &= ~promotion;
	mPW &= ~promotion;
	//printf("P1\n");
	//printBits(8, &mP1_prom);
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
			//score = ((!(color)) == ((stm))) ? + WIN  : -(WIN) ;
			if ((fr[0]&ho[7])>0)
			score = color*(-WIN - draft);
			else score = 0;
			//printf("MATE --- col %d stm %llu score*%d*\n", color, stm, score);
			//printNboard(arg);
			
			return score;	//MATE SCORE
	
	return 0;

found_move:
	return neval( rb);
}


U64 gen_ho_atackN( Nboard arg)
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
//		printf("white move\n");
	}
	else
	{
		fr = &arg.pieceset[8];
		ho = &arg.pieceset[0];
	at |= (ho[5] & 0xFEFEFEFEFEFEFEFE) << 7;
	at |= (ho[5] & 0x7F7F7F7F7F7F7F7F) << 9;
//		printf("black move\n");
	}
	//gledati napadnuta polja bez kralja zbog šahova
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
	
	//ho[7] &= ~arg.fr[6];		//!!!!!!!!!!!!!!!!!!!! samo prazna polja
	return at;
}

char generate_movesN(Nmovelist *ZZZ, Nboard arg)
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
	//move *m_list = NULL, *m;
	//if arg.info  
        	/*printf("Squiet count: %d	\n", quietcount);
        	printf("Scapt count: %d	\n", captcount);
        	printf("Smoves count: %d	\n", quietcount + 255 - captcount);*/

	stm = (arg.info >> 11) & 0x0000000000000008;// 1 sa 14. mjesta na 3.
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
	/*printf("enp\n");
	printBits(8, &enp);*/

	king = __builtin_ffsll(fr[0])-1;
        
	//!!!!!!!!!!!!!!! gradnja atack map, pieces, kasnije neće biti potrebno jer će se update-ati sa make, unmake move!!!!!!!!!!!!!!!!!!!!!!!
	fr[6] = fr[5] ^ fr[4] ^ fr[2] ^ fr[3] ^ fr[0] ^ fr[1];
	ho[6] = ho[5] ^ ho[4] ^ ho[2] ^ ho[3] ^ ho[0] ^ ho[1];
	arg.pieceset[16] = fr[6] ^ ho[6];
	ho[7] = gen_ho_atackN(arg);
	
	//print_state(arg);
				/*printf("frK\n");
				printBits(8, &fr[0]);
				printf("hoK\n");
				printBits(8, &ho[0]);
				printf("fr[6]\n");
				printBits(8, &fr[6]);*/
	
	ZZZ->undo = arg.info; //spremaj old: enp, cast, hm i stm - za undo
	ZZZ->old_zobrist = arg.zobrist;
	// provjeri jel šah
	if (fr[0] & ho[7])
	{
		//printf("is_check\n");
		if (stm)	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) << 7 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) << 9 & ho[5];  		
		else	in_K = (fr[0] & 0xFEFEFEFEFEFEFEFE) >> 9 & ho[5] | (fr[0] & 0x7F7F7F7F7F7F7F7F) >> 7 & ho[5];  		

		in_at_b = ( (arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); // ~ frijendly pieces ??
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); // ~ frijendly pieces ??
		
		in_N = movesNight[king] & ho[4];
		
		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); // ~ frijendly pieces ??
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
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

		/*in_K = __builtin_ffsll(fr[0])-1;
		mK_q = movesKing[in_K] & ~fr[6] & ~ho[7];
		mK_c = mK_q & ho[6];
		mK_q &= ~ ho[6];
		while (__builtin_popcountll(mK_q))
		{
			in = __builtin_ffsll(mK_q)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			movearray[movecount].next = m;
			movearray[movecount].from = (1LL << in_K);
			movearray[movecount].dest = (1LL << in);
			ZZZ->mdata[movecount] ^= 1LL  ^ stm;
			//check fali, zapisi kao const osim stm
			mK_q &= ~movearray[movecount].dest; 
		}
		while (__builtin_popcountll(mK_c))
		{
			in = __builtin_ffsll(mK_c)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			movearray[movecount].next = m;
			movearray[movecount].from = (1LL << in_K);
			movearray[movecount].dest = (1LL << in);
			ZZZ->mdata[movecount] ^= 1LL ^ (1LL << 6) ^ stm;
			//check fali, zapisi kao const osim stm
			mK_c &= ~movearray[movecount].dest; 
		}*/
	}
	


	all_pp = 0LL;
	ppb = 0LL;
	ppr = 0LL;
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/	//ho[7] = 0LL; // PRIVREMENO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//bb = blank & occupancyMaskBishop[king];
	//printf("gen_m\n");

	// tražim vezane figure po dijagonali
	in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
	at_b = magicMovesBishop[king][in_at_b] & (ho[3] ^ ho[1]); // ~ frijendly pieces ??
	//printBits(8, &at_b);
	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index kralja moze i izvan while	
		in_k = ((arg.pieceset[16] & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		ppb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at];
		//ppb = magicMovesBishop[king][in_k] ;
		all_pp ^= ppb;
		//while (__builtin_popcountll(ppb))
		{
			pp = __builtin_ffsll(ppb)-1;
			bb = arg.pieceset[16] & ~ (1LL << pp);
			in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
			in_k = ((bb & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
			mpb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at] &  ~ppb ^ (1LL << at); 
			mpb &= check_grid;

			if (__builtin_popcountll( (fr[1] ^ fr[3]) & ppb )) // vjerovatno ne treba popcount
			{
				//printf("mpb\n");
				//printBits(8, &mpb);
				while ( __builtin_popcountll(mpb))
				{
					mpp = __builtin_ffsll(mpb)-1;
					if (fr[3] & ppb)	piecetype = 3;
					else piecetype = 1;
				
					//provjera check?
					tmp = (1LL << mpp) & ho[6] ? captcount : quietcount;					
					
					//novi potez pp na mpp
					ZZZ->mdata[tmp] &= 0LL;
					ZZZ->mdata[tmp] ^= piecetype; //piecetype
					ZZZ->mdata[tmp] ^= pp << 6; //source
					ZZZ->mdata[tmp] ^= mpp << 12; //destination
                        		//odredi captured piece
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
			//provjera za pješaka
			if ( fr[5] & ppb)
			{
				pp = __builtin_ffsll(ppb)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
				if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & ~promotion & check_grid);
				else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & ~promotion & check_grid );
				// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
			
				if (mpp)
				{
					// bez PROMOTION
					ZZZ->mdata[captcount] &= 0LL;
					ZZZ->mdata[captcount] ^= 5; //piecetype
					ZZZ->mdata[captcount] ^= pp << 6; //source
					ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
                        		//odredi captured piece
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

					mpb &= ~(1LL << (mpp-1)); //pitanje jel potrebno
					captcount++;
				}
				else 
				{
				// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
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
		                			
                                        		//odredi captured piece
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
						//check fali, zapisi kao const osim stm
        					mpb &= ~(1LL << (mpp-1)); //pitanje jel potrebno
					}
					else
					{
						if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho[6]^enp) & check_grid);
						else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho[6] ^ enp) & check_grid);
						if (mpp)
						{
							//promotion nije kad je en'passan
					                ZZZ->mdata[captcount] &= 0LL;
                					ZZZ->mdata[captcount] ^= 7; //piecetype
		                			ZZZ->mdata[captcount] ^= pp << 6; //source
		                			ZZZ->mdata[captcount] ^= (mpp - 1) << 12; //destination
                                        		//odredi captured piece
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
        
                					mpb &= ~(1LL << (mpp-1)); //pitanje jel potrebno
		                			captcount++;
						}
					}
				}
			}
		}
		at_b &= ~(1LL << at); 
	}

	// tražim vezane figure po liniji i redu
	in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
	at_r = magicMovesRook[king][in_at_r] & (ho[2] ^ ho[1]); // ~ frijendly pieces ??
	//printBits(8, &at_r);
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
		in_k = ((arg.pieceset[16] & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
		//ppb = magicMovesBishop[king][in_k] ;
		all_pp ^= ppr;
		//while (__builtin_popcountll(ppb))
		{
			pp = __builtin_ffsll(ppr)-1;
			bb = arg.pieceset[16] & ~ (1LL << pp);
			in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
			in_k = ((bb & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
			mpr = magicMovesRook[king][in_k] & magicMovesRook[at][in_at] &  ~ppr ^ (1LL << at); 
			mpr &= check_grid;
			//printf("mpr\n");
			//printBits(8, &mpr);
			
			if (__builtin_popcountll( (fr[1] ^ fr[2]) & ppr )) 
			{
				while ( __builtin_popcountll(mpr))
				{
					mpp = __builtin_ffsll(mpr)-1;
					if (fr[2] & ppr)	piecetype = 2;
					else piecetype = 1;
					//provjera check?
					//capture = (1LL << mpp) & ho[6] ? 1LL << 6 : 0LL;					
					tmp = (1LL << mpp) & ho[6] ? captcount : quietcount;					
					
					//novi potez pp na mpp
					ZZZ->mdata[tmp] &= 0LL;
					ZZZ->mdata[tmp] ^= piecetype; //piecetype
					ZZZ->mdata[tmp] ^= pp << 6; //source
					ZZZ->mdata[tmp] ^= mpp << 12; //destination
                        		//odredi captured piece
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
			//ppb &= ~ (1LL << pp);
		}
			//provjera za pješaka
			if ( fr[5] & ppr)
			{
				pp = __builtin_ffsll(ppr)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
				if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.pieceset[16]) << 8) & ~arg.pieceset[16] & check_grid);// za dva
				else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.pieceset[16]) >> 8) & ~arg.pieceset[16] & check_grid);// za dva
                                //PITANJE DA LI PAWN ADVANCE JE QUIET ILI CAPTURE
				// bez PROMOTION
				if (mpp)
				{
					ZZZ->mdata[quietcount] &= 0LL;
					ZZZ->mdata[quietcount] ^= 5; //piecetype
					ZZZ->mdata[quietcount] ^= pp << 6; //source
					ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination
					ZZZ->mdata[quietcount] ^= (((mpp - 1)%8) << 22) ^ (1LL << 21); //enp_file

					mpr &= ~(1LL << (mpp-1)); //pitanje jel potrebno
					quietcount++;
				}
				//else 
				mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.pieceset[16]) & check_grid
						: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.pieceset[16]) & check_grid;// za  jedan 
				if (mpp)
				{
					ZZZ->mdata[quietcount] &= 0LL;
					ZZZ->mdata[quietcount] ^= 5; //piecetype
					ZZZ->mdata[quietcount] ^= pp << 6; //source
					ZZZ->mdata[quietcount] ^= (mpp - 1) << 12; //destination

					mpr &= ~(1LL << (mpp-1)); //pitanje jel potrebno
					quietcount++;
				}
				
			}
		at_r &= ~(1LL << at); 
	}
	//ppb =  magicMovesBishop[at][in_at];
	//printf("all_pp\n");
	//printBits(8, &all_pp);
	
	//legalnipotezi
	
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
	//mENP = 0LL;
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
	//mENP = 0LL;
	mP1_prom = mP1 & promotion;
	mPE_prom = mPE & promotion;
	mPW_prom = mPW & promotion;
	mP1 &= ~promotion;
	mPE &= ~promotion;
	mPW &= ~promotion;
	//printf("P1\n");
	//printBits(8, &mP1_prom);
	}

	while (__builtin_popcountll(mP2))
	{
		in = __builtin_ffsll(mP2)-1;
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 5; //piecetype
		ZZZ->mdata[quietcount] ^= (in - P2) << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination
		ZZZ->mdata[quietcount] ^= ((in % 8) << 22) ^ (1LL << 21); //enp_file
		mP2 &= ~(1LL << in); //pitanje jel potrebno
		quietcount++;

	}
	while (__builtin_popcountll(mP1))
	{
		in = __builtin_ffsll(mP1)-1;
		ZZZ->mdata[quietcount] &= 0LL;
		ZZZ->mdata[quietcount] ^= 5; //piecetype
		ZZZ->mdata[quietcount] ^= (in - P1) << 6; //source
		ZZZ->mdata[quietcount] ^= in << 12; //destination
		mP1 &= ~(1LL << in); //pitanje jel potrebno
		quietcount++;
	}
	while (__builtin_popcountll(mPW))
	{
		// bez PROMOTION
		in = __builtin_ffsll(mPW)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 5; //piecetype
		ZZZ->mdata[captcount] ^= (in - PW) << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
      		//odredi captured piece
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
		// bez PROMOTION
		in = __builtin_ffsll(mPE)-1;
		ZZZ->mdata[captcount] &= 0LL;
		ZZZ->mdata[captcount] ^= 5; //piecetype
		ZZZ->mdata[captcount] ^= (in - PE) << 6; //source
		ZZZ->mdata[captcount] ^= in << 12; //destination
      		//odredi captured piece
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
        		//promotion nije kad je en'passan
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
		//check fali, zapisi kao const osim stm
        	mP1_prom &= ~(1LL << in); //pitanje jel potrebno
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
           			
      		//odredi captured piece
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
		//check fali, zapisi kao const osim stm
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
           			
      		//odredi captured piece
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
		//check fali, zapisi kao const osim stm
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
              		//odredi captured piece
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
              		//odredi captured piece
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
              		//odredi captured piece
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
              		//odredi captured piece
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
	//while (__builtin_popcountll(fr[0]))
	/*printf("ho7\n");
	printBits(8, &ho[7]);
	printf("ho5\n");
	printBits(8, &ho[5]);*/
	{
		in_K = __builtin_ffsll(fr[0])-1;
		mK_q = movesKing[in_K] & ~fr[6] & ~ho[7];
	//printf("mk\n");
	//printBits(8, &mK_q);
		mK_c = mK_q & ho[6];
		mK_q &= ~ ho[6];
	/*printf("mkQ\n");
	printBits(8, &mK_q);
	printf("mkC\n");
	printBits(8, &mK_c);*/
		
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
              		//odredi captured piece
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
	}
	//CASTLE
	if ( arg.info & cas_bit_K && !(ho[7] & cas_at_K) && !(arg.pieceset[16] & cas_occ_K) )
	{
//		U64 CK = 0x0200000000000000 >> stm*56;

			//printf("casK	\n");
        		ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 6 ^ (6 << 3);
	       		ZZZ->mdata[quietcount] ^= king << 6; //source
        		ZZZ->mdata[quietcount] ^= (king-2) << 12; //destination
 	
			quietcount++;
	}
	if ( arg.info & cas_bit_Q && !(ho[7] & cas_at_Q) && !(arg.pieceset[16] & cas_occ_Q) )
	{
//                U64 CQ = 0x2000000000000000 >> stm*56;
	//printf("casQ\n");
        		ZZZ->mdata[quietcount] &= 0LL;
			ZZZ->mdata[quietcount] ^= 6 ^ (7 << 3);
	       		ZZZ->mdata[quietcount] ^= king << 6; //source
        		ZZZ->mdata[quietcount] ^= (king+2) << 12; //destination
			quietcount++;
	}
	//potrebno je odrolati pinned pieces 
	
	
	/*printf("ho.atackmap\n");
	printBits(8, &ho[7]);
	printf("checkgrid\n");
	printBits(8, &check_grid);*/
	
        	/*printf("quiet count: %d	\n", quietcount);
        	printf("capt count: %d	\n", 255-captcount);
        	printf("moves count: %d	\n", quietcount + 255 -captcount);*/

        ZZZ->quietcount = quietcount;
        ZZZ->captcount = captcount;

	return quietcount + captcount - 218;
}

int Ndo_move(Nboard *b, Nmove m)
{
//if (stop) return 0;
	U64 stm, castle, cast_diff, hm, fm, m_cas, f_cas, CK, CQ, KS, KR, QR, hKR, hQR;
	
	unsigned char p_type, capt_type, prom_type, sq_from, sq_dest, enp, pindex;
	
	stm = (b->info >> 14) & 0x0000000000000001;
	
	KR = 0x0100000000000000 >> (stm*56);
	QR = 0x8000000000000000 >> (stm*56);
	hKR = 0x0000000000000001ULL << (stm*56);//stavi ULL da izbjegnes left shift warning
	hQR = 0x0000000000000080ULL << (stm*56);
	//castle = 0x000000000000001E;
	castle = 0LL;
	//printf("m\n");
	//printBits(4, &m);
		        //printmoveN( &m);
	p_type = m & 0x00000007;
	capt_type = m >> 3 & 0x00000007;
	//printf("stm %llu p %d capt %d\n", stm, p_type, capt_type);
	
	if (p_type < 6)
	{
	        
	        sq_from = m >> 6 & 0x0000003F;
	//printf("m\n");
	//printBits(4, &m);
	        sq_dest = m >> 12 & 0x0000003F;
	        prom_type = m >> 19 & 0x00000003;
	//printf("fr %d dst %d\n", sq_from, sq_dest);
	        //pindex = p_type + ((!stm) << 3);
	//printf("fr %d dst %d\n", sq_from, sq_dest);
	        //if (((sq_from == 6) || (sq_dest == 6))) 
	        /*{
	        printf("Dfr %d dst %d pindex %d\n", sq_from, sq_dest,pindex);
	        printf("m\n");
	        printBits(4, &m);
	        
                }*/
        
	//printf("pieceset ind %d\n", p_type + ((!stm) << 3));
        	b->pieceset[ p_type + ((!stm) << 3) ] &= ~(1ULL << sq_from);
        	b->pieceset[ p_type + ((!stm) << 3) ] |= 1ULL << sq_dest;

	//printf("p_t %d sq_fr %d sq_d %d\n", p_type, sq_from, sq_dest);
        	b->zobrist ^= zobrist[ stm * 6*64 + p_type * 64 + sq_from];
        	b->zobrist ^= zobrist[ stm * 6*64 + p_type * 64 + sq_dest];
	
	//castle
	/*t = (m->dest & hQR);
	printf("hqr-dest\n");
	printBits(8, &hQR);
	 0;t = ( m->info & 1LL << 26 );
	printf("hkr\n");
	printBits(8, &hKR);*/
	//provjera da li igra kralj ili je rokada fr gubi oba castla
        	//moguća dorada uvjeta sa bitwise operator
        	m_cas = 0x0000000000000C0 >> (stm<<1);
        	f_cas = (!p_type) | (p_type == 6);
	        castle |= (castle & ~m_cas) | (-f_cas & m_cas);
	        //provjera da li je top igrao fr gubi castle
	        m_cas = 0x000000000000040 >> (stm<<1);
	        f_cas = (p_type == 2) && ( 1LL << sq_from & KR); 
	        castle |= (castle & ~m_cas) | (-f_cas & m_cas);
	        m_cas = 0x000000000000080 >> (stm<<1);
	        f_cas = (p_type == 2) && ( 1LL << sq_from & QR);
	        castle |= (castle & ~m_cas) | (-f_cas & m_cas);
	//provjera da li je top pojeden ho gubi castle 
//nije provjereno jel je mask na vecoj mjeri od uvjeta
	        m_cas = 0x000000000000010 << (stm<<1);
	        f_cas = (1LL << sq_dest & hKR) && ( capt_type == 2 );
	        castle |= (castle & ~m_cas) | (-f_cas & m_cas);
	//printf("fcasK\n");
	//printBits(8, &f_cas);
	 
	        m_cas = 0x000000000000020 << (stm<<1);
	        f_cas = (1LL << sq_dest & hQR) && ( capt_type == 2 );
	        castle |= (castle & ~m_cas) | (-f_cas & m_cas);
	//printf("fcasQ\n");
	//printBits(8, &f_cas);



	//b->info & 0x000000000000006 (m->info & 1LL << 26 && (p->R & 0x0000000000000001) )

	//doing move
	//printf("prije\n");
	//printBits(8, p);
	//printf("poslije\n");
	//printBits(8, p);
	
	//removing captured piece
	b->pieceset[capt_type + ((stm) << 3)] &= ~(1LL << sq_dest);
	
	//printf("cap %d\n", capt_type);
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!!!!!!!!!!!! zobrist treba biti 0 kad capt_type 0 !!!!!!!!!!!!!!!!!!!!!!!!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	b->zobrist ^= capt_type ? zobrist[ !stm * 6*64 + capt_type * 64 + 64 + sq_dest] : 0ULL;
		

	//promotion
	b->pieceset[prom_type + ((!stm) << 3) + 1] |= (m >> 18 & 1LL) << sq_dest;
	b->pieceset[5 + ((!stm) << 3)] &= ~((m >> 18 & 1LL) << sq_dest);
	
	//printf("prom %d\n", prom_type);
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!!!!!!!!!!!! zobrist treba biti 0 kad nema promotion tj. m >> 18 je 0 !!!!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	
	b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + prom_type * 64 + 64 + sq_dest] : 0ULL;
	b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + 5 * 64 + sq_dest] : 0ULL;
	
	
	//update all_p
	b->pieceset[16] &= ~(1LL << sq_from);
	b->pieceset[16] |= 1LL << sq_dest;
	
	/*if (__builtin_popcountll(b->pieceset[0]) > 1) {
	if (b->pieceset[5] & 1LL)
	        {
	        printf("Dfr %d dst %d pindex %d\n", sq_from, sq_dest,pindex);
	        printf("m\n");
	        printBits(4, &m);
	        stop = 1;
                }
	
	}*/
	}
	else if (p_type == 7)
	{
	//hm = ((m->info & 0x0000000000001000));
	//printf("enpuvjet\n");
	//printBits(8, &hm);

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
	/*if (b->pieceset[5] & 1LL)
	        {
	        printf("Dfr %d dst %d pindex %d\n", sq_from, sq_dest,pindex);
	        printf("m\n");
	        printBits(4, &m);
	        stop = 1;
                }*/
	else if (capt_type == 6)
	{
	//hm = ((m->info & 0x0000000000002000));
	//printf("casK\n");
	//printBits(8, &hm);

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
	//hm = ((m->info & 0x0000000000006000));
	//printf("casQ\n");
	//printBits(8, &hm);

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
	
	
	//half move korak je 1 ako potez nije capture ili pawn advance
	hm = capt_type || (p_type == 5) ? -(b->info & 0x0000000000003F00) : (1LL << 8);

	// b_info = stm ^ castle ^ doublepawnpush ^ halfmove ^ fullmove
	
	/*printf("b-dpp\n");
	t = b->info & 0x000000000000FF00;
	printBits(8, &t);
	printf("m-dpp\n");
	t = m->info & 0x0000000000FF0000;
	printBits(8, &t);*/

	
	//zobrist
	//printBits(8, &cast_diff);
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

int Nundo_move(Nboard *b, Nmovelist *ml, Nmove m)
{
//if (stop) return 0;
	U64 *p, *capt_p, *prom_p, stm, empty = 0LL, hm, CK, CQ, KS, KR, QR, cast_diff;
	piece_set *fr, *ho; 
	int p_type, sq_from, sq_dest, prom_type, enp, pindex, cindex;
	U64 capt_type, cbit, mask;
	stm = ml->undo >> 14 & 0x0000000000000001 ;
	//stm = !(b->info & 1LL) ;
       	p_type = m & 0x00000007;
       	capt_type = m >> 3 & 0x0000000000000007;
	//printf("stm %llu p %d capt %d\n", stm, p_type, capt_type);
	
	//printf("m\n");
	//printBits(4, &m);

	if (p_type < 6)
	{
	        sq_from = m >> 6 & 0x0000003F;
	        sq_dest = m >> 12 & 0x0000003F;
	        prom_type = m >> 19 & 0x00000003;
                
                //pindex = p_type + ((!stm) << 3);        	
                //cindex = capt_type + ((stm) << 3);        	
	        /*if (((sq_from == 6) || (sq_dest == 6))) 
	        {
	        printf("Ufr %d dst %d\n", sq_from, sq_dest);
	        printf("m\n");
	        printBits(4, &m);
	        
                }*/
        	//undoing move
        	b->pieceset[ p_type + ((!stm) << 3) ] &= ~(1ULL << sq_dest);
        	b->pieceset[ p_type + ((!stm) << 3) ] |= 1ULL << sq_from;
	
	//printf("UN p_t %d sq_fr %d sq_d %d\n", p_type, sq_from, sq_dest);
	
        	b->zobrist ^= zobrist[ stm * 6*64 + p_type * 64 + sq_from];
        	b->zobrist ^= zobrist[ stm * 6*64 + p_type * 64 + sq_dest];

        	//returning captured piece
        	//cbit = (capt_type ^ (capt_type - 1)) * 1ULL;
	        /*mask = 0x000000000000001;
	        cbit = 0ULL;
	        cbit |= (cbit & ~mask) | (-capt_type & mask);*/
	        //!!!!!!!!!!!!!!moguće izbjkeci 2 naredne sa dodatnim capture flag bitom!!!!!!!!!!!!!!!!!!!
        	//mask = b->pieceset[stm<<3];
        	cbit = capt_type ? 1LL : 0LL;
        	b->pieceset[capt_type + ((stm) << 3)] |= cbit << sq_dest;
        	//->pieceset[stm<<3] = mask;
        	
        	
	//zobrist captured piece
	//printf("UNcap %d\n", capt_type);
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//!!!!!!!!!!!! zobrist treba biti 0 kad capt_type 0 !!!!!!!!!!!!!!!!!!!!!!!!!
	//!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	b->zobrist ^= capt_type ? zobrist[ !stm * 6*64 + capt_type * 64 + 64  + sq_dest] : 0ULL;

	        //promotion
	b->pieceset[prom_type + ((!stm) << 3) + 1] ^= (m >> 18 & 1LL) << sq_dest;
	//b->pieceset[5 + ((!stm) << 3)] |= (m >> 18 & 1LL) << sq_from;
	        
	//zobrist promotion piece
	//prom_type = __builtin_ffsll( m->info >> 8 & 0x000000000000000F);
	b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + prom_type * 64 + 64 + sq_dest] : 0ULL;
	//printf("UNprom %d\n", prom_type);
	b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + 5 * 64 + sq_dest] : 0ULL;

	//update all_p
	b->pieceset[16] &= ~(1LL << sq_dest);
	b->pieceset[16] |= 1LL << sq_from;
	
	
	//if (__builtin_popcountll(b->pieceset[0]) > 1) {
	/*if (b->pieceset[5] & 1LL)
	{
	        printf("Ufr %d dst %d pindex %d cindex %d\n", sq_from, sq_dest,pindex,cindex);
	        printf("m\n");
	        printBits(4, &m);
                printf("stm %llu p %d capt %llu\n", stm, p_type, capt_type);
                printmoveN(&m);
	        stop = 1;
	        hm = (cbit) << sq_dest;
	        printf("brisac\n");
	        printBits(8, &hm);
	        empty = (cbit);
	        printf("empty\n");
	        printBits(8, &cbit);
 	        
               
	
	        }*/
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
		//printf("UNzob_ind %llu, stm %llu sq_d %d\n", stm * 6*64 + 5 * 64 + sq_dest, stm, sq_dest);
		b->zobrist ^= zobrist[ stm * 6*64 + 5 * 64 + sq_dest];
		b->zobrist ^= zobrist[ stm * 6*64 + 5 * 64 + sq_from];
		b->zobrist ^= zobrist[ !stm * 6*64 + 5 * 64 + sq_dest + 8 - stm*16 ];

	}
	else if (capt_type == 6)
	{
	/*hm = ((m->info & 0x0000000000002000));
	printf("casK\n");
	printBits(8, &hm);*/

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
	/*hm = ((m->info & 0x0000000000006000));
	printf("casQ\n");
	printBits(8, &hm);*/

		KR = 0x0100000000000000 >> stm*56;//mozda && ili ?
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
	//b->info -= ( stm & 1LL) << 16;//fm
        b->zobrist = ml->old_zobrist;

	
	//b->info -= (b->info & 1LL) << 32;
	//b->info ^= 0x0000000000000001;
}

move *generate_movesALLOC(board arg)
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
	//if arg.info  
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

	//!!!!!!!!!!!!!!! gradnja atack map, pieces, kasnije neće biti potrebno jer će se update-ati sa make, unmake move!!!!!!!!!!!!!!!!!!!!!!!
	fr->pieces = fr->P ^ fr->N ^ fr->R ^ fr->B ^ fr->K ^ fr->Q;
	ho->pieces = ho->P ^ ho->N ^ ho->R ^ ho->B ^ ho->K ^ ho->Q;
	arg.all_p = fr->pieces ^ ho->pieces;
	ho->atack = gen_ho_atack(arg);
	
	// provjeri jel šah
	if (fr->K & ho->atack)
	{
		//printf("is_check\n");
		if (stm)	in_K = (fr->K & 0xFEFEFEFEFEFEFEFE) << 7 & ho->P | (fr->K & 0x7F7F7F7F7F7F7F7F) << 9 & ho->P;  		
		else	in_K = (fr->K & 0xFEFEFEFEFEFEFEFE) >> 9 & ho->P | (fr->K & 0x7F7F7F7F7F7F7F7F) >> 7 & ho->P;  		

		in_at_b = ( (arg.all_p & occupancyMaskBishop[king]) * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho->B ^ ho->Q); // ~ frijendly pieces ??
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.all_p & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho->R ^ ho->Q); // ~ frijendly pieces ??
		
		in_N = movesNight[king] & ho->N;
		
		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.all_p & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); // ~ frijendly pieces ??
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.all_p & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
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

		/*in_K = __builtin_ffsll(fr->K)-1;
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
			m_list->info ^= 1LL  ^ stm;
			//check fali, zapisi kao const osim stm
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
			m_list->info ^= 1LL ^ (1LL << 6) ^ stm;
			//check fali, zapisi kao const osim stm
			mK_c &= ~m_list->dest; 
		}*/
	}
	


	all_pp = 0LL;
	ppb = 0LL;
	ppr = 0LL;
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/	//ho->atack = 0LL; // PRIVREMENO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//bb = blank & occupancyMaskBishop[king];
	//printf("gen_m\n");

	// tražim vezane figure po dijagonali
	in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
	at_b = magicMovesBishop[king][in_at_b] & (ho->B ^ ho->Q); // ~ frijendly pieces ??
	//printBits(8, &at_b);
	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.all_p & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index kralja moze i izvan while	
		in_k = ((arg.all_p & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		ppb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at];
		//ppb = magicMovesBishop[king][in_k] ;
		all_pp ^= ppb;
		//while (__builtin_popcountll(ppb))
		{
			pp = __builtin_ffsll(ppb)-1;
			bb = arg.all_p & ~ (1LL << pp);
			in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
			in_k = ((bb & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
			mpb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at] &  ~ppb ^ (1LL << at); 
			mpb &= check_grid;

			if (__builtin_popcountll( (fr->Q ^ fr->B) & ppb )) // vjerovatno ne treba popcount
			{
				//printf("mpb\n");
				//printBits(8, &mpb);
				while ( __builtin_popcountll(mpb))
				{
					mpp = __builtin_ffsll(mpb)-1;
					if (fr->B & ppb)	piece_type = (1LL << 3);
					else piece_type = (1LL << 1);
					//provjera check?
					capture = (1LL << mpp) & ho->pieces ? 1LL << 6 : 0LL;					
					
					//novi potez pp na mpp
					m = m_list;
					m_list = malloc(sizeof(move));
					m_list->next = m;
					m_list->from = ppb;
					m_list->dest = (1LL << mpp);
					m_list->info = piece_type ^ capture ^ stm; // check fali
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					mpb &= ~m_list->dest; 
					
				}
			}
			//provjera za pješaka
			if ( fr->P & ppb)
			{
				pp = __builtin_ffsll(ppb)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
				if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho->pieces & ~promotion & check_grid);
				else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho->pieces & ~promotion & check_grid );
				// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
	/*printf("pawn_pinned\n");
	bb = (ho->pieces ^ enp); 
	printBits(8, &bb);
	bb = enp ;
	printBits(8, &enp);
	bb = (ppb << 9) ^ (ppb << 7) ;
	printBits(8, &bb);
		printBits(8, &ho->pieces);*/
				
				if (mpp)
				{
					// bez PROMOTION

					m = m_list;
					m_list = malloc(sizeof(move));
					m_list->next = m;
					m_list->from = ppb;
					m_list->dest = (1LL << (mpp-1));
					m_list->info = (1LL << 5) ^ (1LL << 6) ^ stm;// check fali i zapisi kao const osim stm
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					mpb &= ~m_list->dest; //pitanje jel potrebno
				}
				else 
				{
				// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
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
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
						}
						//check fali, zapisi kao const osim stm
						mpb &= ~m_list->dest; //pitanje jel potrebno
					}
					else
					{
						if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho->pieces^enp) & check_grid);
						else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho->pieces ^ enp) & check_grid);
						if (mpp)
						{
							//promotion nije kad je en'passan
							capture = 1LL << 6;
						
							m = m_list;
							m_list = malloc(sizeof(move));
							m_list->next = m;
							m_list->from = ppb;
							m_list->dest = (1LL << (mpp-1));
							m_list->info = (1LL << 5) ^ (1LL << 6) ^ (1LL << 12) ^ stm;
							//check fali, zapisi kao const osim stm
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
							mpb &= ~m_list->dest; //pitanje jel potrebno
						}
					}
					
				}

			}
			//provjera za pješaka

			
			//ppb &= ~ (1LL << pp);
		}
		at_b &= ~(1LL << at); 
	}
	//ppb =  magicMovesBishop[at][in_at];
	//		printf("all_ppb\n");
	//printBits(8, &all_ppb);

	//printBits(8, &ppb);


	// tražim vezane figure po liniji i redu
	in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
	at_r = magicMovesRook[king][in_at_r] & (ho->R ^ ho->Q); // ~ frijendly pieces ??
	//printBits(8, &at_r);
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.all_p & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
		in_k = ((arg.all_p & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
		//ppb = magicMovesBishop[king][in_k] ;
		all_pp ^= ppr;
		//while (__builtin_popcountll(ppb))
		{
			pp = __builtin_ffsll(ppr)-1;
			bb = arg.all_p & ~ (1LL << pp);
			in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
			in_k = ((bb & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
			mpr = magicMovesRook[king][in_k] & magicMovesRook[at][in_at] &  ~ppr ^ (1LL << at); 
			mpr &= check_grid;
			//printf("mpr\n");
			//printBits(8, &mpr);
			
			if (__builtin_popcountll( (fr->Q ^ fr->R) & ppr )) 
			{
				while ( __builtin_popcountll(mpr))
				{
					mpp = __builtin_ffsll(mpr)-1;
					if (fr->R & ppr)	piece_type = (1LL << 2);
					else piece_type = (1LL << 1);
					//provjera check?
					capture = (1LL << mpp) & ho->pieces ? 1LL << 6 : 0LL;					
					
					//novi potez pp na mpp
					m = m_list;
					m_list = malloc(sizeof(move));
					m_list->next = m;
					m_list->from = ppr;
					m_list->dest = (1LL << mpp);
					m_list->info = piece_type ^ capture ^ stm; // check fali
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					mpr &= ~m_list->dest; 
					
				}
			}


			
			//ppb &= ~ (1LL << pp);
		}
			//provjera za pješaka
			if ( fr->P & ppr)
			{
				pp = __builtin_ffsll(ppr)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
				if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.all_p) << 8) & ~arg.all_p & check_grid);// za dva
				else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.all_p) >> 8) & ~arg.all_p & check_grid);// za dva
	/*printf("pawn_pinned\n");
	bb = ( mpr & ((0x000000000000FF00 << 8) & ~ho->pieces) << 8 & ~ho->pieces); // dodaj ogranicenje za a i h liniju ovisno o smjeru capture
	printBits(8, &bb);
	bb = enp ;
	printBits(8, &enp);
	bb = (ppb << 9) ^ (ppb << 7) ;
	printBits(8, &bb);
		printBits(8, &ho->pieces);*/
		
//PITANJE DA LI PAWN ADVANCE JE QUIET ILI CAPTURE
				if (mpp)
				{
					// nije uracunat PROMOTION

					m = m_list;
					m_list = malloc(sizeof(move));
					m_list->next = m;
					m_list->from = ppr;
					m_list->dest = (1LL << (mpp-1));
					m_list->info = (1LL << 5) ^ (1LL << 15) ^ (1LL << (16 + (mpp-1)%8)) ^ stm;// check fali i zapisi kao const osim stm, također izbjegni modulus
					//printf("m-info 16 mpp %d mpp%8 %d\n", mpp, (16 + mpp%8));
					//printBits(8, &m_list->info);

		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					
					mpb &= ~m_list->dest; 
				}
				//else 
				{
					mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.all_p) & check_grid
							: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.all_p) & check_grid;// za  jedan 
					if (mpp)
					{
						//promotion nije kad je en'passan
						//capture = 1LL << 6;
					
						m = m_list;
						m_list = malloc(sizeof(move));
						m_list->next = m;
						m_list->from = ppr;
						m_list->dest = (1LL << (mpp-1));
						m_list->info = (1LL << 5) ^ (1LL << 15) ^ stm;
						//check fali, zapisi kao const osim stm
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
						mpb &= ~m_list->dest; 
					}
				}
				
			}
		at_r &= ~(1LL << at); 
	}
	//ppb =  magicMovesBishop[at][in_at];
	//printf("all_pp\n");
	//printBits(8, &all_pp);
	
	//legalnipotezi
	
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
	//mENP = 0LL;
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
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
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
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
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
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
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		}
		//check fali, zapisi kao const osim stm
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
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		}
		//check fali, zapisi kao const osim stm
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
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
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
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
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
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
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
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
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mQ_c &= ~m_list->dest; 
		}
		fr->Q &= ~1LL << in_Q; 
	}
	
	//KING
	//while (__builtin_popcountll(fr->K))
	{
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
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
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mK_c &= ~m_list->dest; 
		}
		//fr->K &= ~1LL << in_K; 
	}
	//CASTLE
	if ( arg.info & cas_bit_K && !(ho->atack & cas_at_K) && !(arg.all_p & cas_occ_K) )
	{
			
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->info = 1LL << 13 ^ stm;
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
	}
	if ( arg.info & cas_bit_Q && !(ho->atack & cas_at_Q) && !(arg.all_p & cas_occ_Q) )
	{
			m = m_list;
			m_list = malloc(sizeof(move));
			m_list->next = m;
			m_list->info = 1LL << 14 ^ stm;
		//prenesi old_enp_sq i old castle
		m_list->info ^= (arg.info & 0x0000000000FFFF1E) << 32;
	}
	//potrebno je odrolati pinned pieces 
	
	
	/*printf("ho.atackmap\n");
	printBits(8, &ho->atack);
	printf("checkgrid\n");
	printBits(8, &check_grid);*/


	return m_list;
}

char generate_moves(board arg, move *movearray)
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
	//move *m_list = NULL, *m;
	//if arg.info  
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

	//!!!!!!!!!!!!!!! gradnja atack map, pieces, kasnije neće biti potrebno jer će se update-ati sa make, unmake move!!!!!!!!!!!!!!!!!!!!!!!
	fr->pieces = fr->P ^ fr->N ^ fr->R ^ fr->B ^ fr->K ^ fr->Q;
	ho->pieces = ho->P ^ ho->N ^ ho->R ^ ho->B ^ ho->K ^ ho->Q;
	arg.all_p = fr->pieces ^ ho->pieces;
	ho->atack = gen_ho_atack(arg);
	
	// provjeri jel šah
	if (fr->K & ho->atack)
	{
		//printf("is_check\n");
		if (stm)	in_K = (fr->K & 0xFEFEFEFEFEFEFEFE) << 7 & ho->P | (fr->K & 0x7F7F7F7F7F7F7F7F) << 9 & ho->P;  		
		else	in_K = (fr->K & 0xFEFEFEFEFEFEFEFE) >> 9 & ho->P | (fr->K & 0x7F7F7F7F7F7F7F7F) >> 7 & ho->P;  		

		in_at_b = ( (arg.all_p & occupancyMaskBishop[king]) * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
		at_b = magicMovesBishop[king][in_at_b] & (ho->B ^ ho->Q); // ~ frijendly pieces ??
		mpb = magicMovesBishop[king][in_at_b];		

		in_at_r = ( (arg.all_p & occupancyMaskRook[king]) * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
		at_r = magicMovesRook[king][in_at_r] & (ho->R ^ ho->Q); // ~ frijendly pieces ??
		
		in_N = movesNight[king] & ho->N;
		
		check_pieces = in_K | in_N | at_b | at_r;

		if (__builtin_popcountll(check_pieces) < 2) 
		{		
			if (__builtin_popcountll(at_b))
			{
				
				at = __builtin_ffsll(at_b)-1;
				in_at = ((arg.all_p & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
				check_grid = magicMovesBishop[at][in_at] & magicMovesBishop[king][in_at_b] | (1LL << at); // ~ frijendly pieces ??
			}
			else if (__builtin_popcountll(at_r))
			{
				at = __builtin_ffsll(at_r)-1;
				in_at = ((arg.all_p & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
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

		/*in_K = __builtin_ffsll(fr->K)-1;
		mK_q = movesKing[in_K] & ~fr->pieces & ~ho->atack;
		mK_c = mK_q & ho->pieces;
		mK_q &= ~ ho->pieces;
		while (__builtin_popcountll(mK_q))
		{
			in = __builtin_ffsll(mK_q)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			movearray[movecount].next = m;
			movearray[movecount].from = (1LL << in_K);
			movearray[movecount].dest = (1LL << in);
			movearray[movecount].info ^= 1LL  ^ stm;
			//check fali, zapisi kao const osim stm
			mK_q &= ~movearray[movecount].dest; 
		}
		while (__builtin_popcountll(mK_c))
		{
			in = __builtin_ffsll(mK_c)-1;
			m = m_list;
			m_list = malloc(sizeof(move));
			movearray[movecount].next = m;
			movearray[movecount].from = (1LL << in_K);
			movearray[movecount].dest = (1LL << in);
			movearray[movecount].info ^= 1LL ^ (1LL << 6) ^ stm;
			//check fali, zapisi kao const osim stm
			mK_c &= ~movearray[movecount].dest; 
		}*/
	}
	


	all_pp = 0LL;
	ppb = 0LL;
	ppr = 0LL;
/*!!!!!!!!!!!!!!!!!!!!!!!!!!!!*/	//ho->atack = 0LL; // PRIVREMENO !!!!!!!!!!!!!!!!!!!!!!!!!!!!!!!
	//bb = blank & occupancyMaskBishop[king];
	//printf("gen_m\n");

	// tražim vezane figure po dijagonali
	in_at_b = ( blank * magicNumberBishop[king] ) >> magicNumberShiftsBishop[king]; 
	at_b = magicMovesBishop[king][in_at_b] & (ho->B ^ ho->Q); // ~ frijendly pieces ??
	//printBits(8, &at_b);
	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.all_p & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index kralja moze i izvan while	
		in_k = ((arg.all_p & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
		ppb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at];
		//ppb = magicMovesBishop[king][in_k] ;
		all_pp ^= ppb;
		//while (__builtin_popcountll(ppb))
		{
			pp = __builtin_ffsll(ppb)-1;
			bb = arg.all_p & ~ (1LL << pp);
			in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
			in_k = ((bb & occupancyMaskBishop[king]) * magicNumberBishop[king]) >> magicNumberShiftsBishop[king];
			mpb = magicMovesBishop[king][in_k] & magicMovesBishop[at][in_at] &  ~ppb ^ (1LL << at); 
			mpb &= check_grid;

			if (__builtin_popcountll( (fr->Q ^ fr->B) & ppb )) // vjerovatno ne treba popcount
			{
				//printf("mpb\n");
				//printBits(8, &mpb);
				while ( __builtin_popcountll(mpb))
				{
					mpp = __builtin_ffsll(mpb)-1;
					if (fr->B & ppb)	piece_type = (1LL << 3);
					else piece_type = (1LL << 1);
					//provjera check?
					capture = (1LL << mpp) & ho->pieces ? 1LL << 6 : 0LL;					
					
					//novi potez pp na mpp
					movearray[movecount].from = ppb;
					movearray[movecount].dest = (1LL << mpp);
					movearray[movecount].info = piece_type ^ capture ^ stm; // check fali
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					mpb &= ~movearray[movecount].dest; 
					movecount++;
				}
			}
			//provjera za pješaka
			if ( fr->P & ppb)
			{
				pp = __builtin_ffsll(ppb)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
				if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho->pieces & ~promotion & check_grid);
				else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho->pieces & ~promotion & check_grid );
				// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
	/*printf("pawn_pinned\n");
	bb = (ho->pieces ^ enp); 
	printBits(8, &bb);
	bb = enp ;
	printBits(8, &enp);
	bb = (ppb << 9) ^ (ppb << 7) ;
	printBits(8, &bb);
		printBits(8, &ho->pieces);*/
				
				if (mpp)
				{
					// bez PROMOTION

					movearray[movecount].from = ppb;
					movearray[movecount].dest = (1LL << (mpp-1));
					movearray[movecount].info = (1LL << 5) ^ (1LL << 6) ^ stm;// check fali i zapisi kao const osim stm
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					mpb &= ~movearray[movecount].dest; //pitanje jel potrebno
					movecount++;
				}
				else 
				{
				// dodaj ogranicenje za a i h liniju ovisno o smjeru capture, moguće da ograničenja i nisu potrebna
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
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
        					movecount++;
						}
						//check fali, zapisi kao const osim stm
						mpb &= ~movearray[movecount-1].dest; //pitanje jel potrebno
					}
					else
					{
						if (stm) mpp =__builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho->pieces^enp) & check_grid);
						else mpp = __builtin_ffsll( mpb & (((ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho->pieces ^ enp) & check_grid);
						if (mpp)
						{
							//promotion nije kad je en'passan
							capture = 1LL << 6;
						
							movearray[movecount].from = ppb;
							movearray[movecount].dest = (1LL << (mpp-1));
							movearray[movecount].info = (1LL << 5) ^ (1LL << 6) ^ (1LL << 12) ^ stm;
							//check fali, zapisi kao const osim stm
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
							mpb &= ~movearray[movecount].dest; //pitanje jel potrebno
                					movecount++;
						}
					}
					
				}

			}
			//provjera za pješaka

			
			//ppb &= ~ (1LL << pp);
		}
		at_b &= ~(1LL << at); 
	}
	//ppb =  magicMovesBishop[at][in_at];
	//		printf("all_ppb\n");
	//printBits(8, &all_ppb);

	//printBits(8, &ppb);


	// tražim vezane figure po liniji i redu
	in_at_r = ( blank * magicNumberRook[king] ) >> magicNumberShiftsRook[king]; 
	at_r = magicMovesRook[king][in_at_r] & (ho->R ^ ho->Q); // ~ frijendly pieces ??
	//printBits(8, &at_r);
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.all_p & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index kralja moze i izvan while	
		in_k = ((arg.all_p & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
		ppr= magicMovesRook[king][in_k] & magicMovesRook[at][in_at];
		//ppb = magicMovesBishop[king][in_k] ;
		all_pp ^= ppr;
		//while (__builtin_popcountll(ppb))
		{
			pp = __builtin_ffsll(ppr)-1;
			bb = arg.all_p & ~ (1LL << pp);
			in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
			in_k = ((bb & occupancyMaskRook[king]) * magicNumberRook[king]) >> magicNumberShiftsRook[king];
			mpr = magicMovesRook[king][in_k] & magicMovesRook[at][in_at] &  ~ppr ^ (1LL << at); 
			mpr &= check_grid;
			//printf("mpr\n");
			//printBits(8, &mpr);
			
			if (__builtin_popcountll( (fr->Q ^ fr->R) & ppr )) 
			{
				while ( __builtin_popcountll(mpr))
				{
					mpp = __builtin_ffsll(mpr)-1;
					if (fr->R & ppr)	piece_type = (1LL << 2);
					else piece_type = (1LL << 1);
					//provjera check?
					capture = (1LL << mpp) & ho->pieces ? 1LL << 6 : 0LL;					
					
					//novi potez pp na mpp
					movearray[movecount].from = ppr;
					movearray[movecount].dest = (1LL << mpp);
					movearray[movecount].info = piece_type ^ capture ^ stm; // check fali
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					mpr &= ~movearray[movecount].dest; 
					movecount++;
					
				}
			}


			
			//ppb &= ~ (1LL << pp);
		}
			//provjera za pješaka
			if ( fr->P & ppr)
			{
				pp = __builtin_ffsll(ppr)-1;  //vjerovatno moguce koristiti pp odozgora jer jedan at ima jedan pinned_p
				if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & ppr) << 8) & ~arg.all_p) << 8) & ~arg.all_p & check_grid);// za dva
				else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & ppr) >> 8) & ~arg.all_p) >> 8) & ~arg.all_p & check_grid);// za dva
	/*printf("pawn_pinned\n");
	bb = ( mpr & ((0x000000000000FF00 << 8) & ~ho->pieces) << 8 & ~ho->pieces); // dodaj ogranicenje za a i h liniju ovisno o smjeru capture
	printBits(8, &bb);
	bb = enp ;
	printBits(8, &enp);
	bb = (ppb << 9) ^ (ppb << 7) ;
	printBits(8, &bb);
		printBits(8, &ho->pieces);*/
		
//PITANJE DA LI PAWN ADVANCE JE QUIET ILI CAPTURE
				if (mpp)
				{
					// nije uracunat PROMOTION

					movearray[movecount].from = ppr;
					movearray[movecount].dest = (1LL << (mpp-1));
					movearray[movecount].info = (1LL << 5) ^ (1LL << 15) ^ (1LL << (16 + (mpp-1)%8)) ^ stm;// check fali i zapisi kao const osim stm, također izbjegni modulus
					//printf("m-info 16 mpp %d mpp%8 %d\n", mpp, (16 + mpp%8));
					//printBits(8, &movearray[movecount].info);

		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
					
					mpb &= ~movearray[movecount].dest; 
					movecount++;
				}
				//else 
				{
					mpp = (stm) ? __builtin_ffsll( mpr & (ppr << 8) & ~arg.all_p) & check_grid
							: __builtin_ffsll( mpr & (ppr >> 8) & ~arg.all_p) & check_grid;// za  jedan 
					if (mpp)
					{
						//promotion nije kad je en'passan
						//capture = 1LL << 6;
					
						movearray[movecount].from = ppr;
						movearray[movecount].dest = (1LL << (mpp-1));
						movearray[movecount].info = (1LL << 5) ^ (1LL << 15) ^ stm;
						//check fali, zapisi kao const osim stm
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
						mpb &= ~movearray[movecount].dest; 
        					movecount++;
					}
				}
				
			}
		at_r &= ~(1LL << at); 
	}
	//ppb =  magicMovesBishop[at][in_at];
	//printf("all_pp\n");
	//printBits(8, &all_pp);
	
	//legalnipotezi
	
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
	//mENP = 0LL;
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
		mP2 &= ~movearray[movecount].dest; 
		movecount++;
	}
	while (__builtin_popcountll(mP1))
	{
		in = __builtin_ffsll(mP1)-1;
		movearray[movecount].from = 1LL << (in - P1 );
		movearray[movecount].dest = 1LL << in;
		movearray[movecount].info = (1LL << 5) ^ (1LL << 15) ^ stm;
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
		mP1 &= ~movearray[movecount].dest; 
		movecount++;
	}
	while (__builtin_popcountll(mPW))
	{
		in = __builtin_ffsll(mPW)-1;
		movearray[movecount].from = 1LL << (in - PW);
		movearray[movecount].dest = 1LL << in;
		movearray[movecount].info = (1LL << 5) ^ (1LL << 6) ^ stm;
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
		mPW &= ~movearray[movecount].dest; 
		movecount++;
	}
	while (__builtin_popcountll(mPE))
	{
		in = __builtin_ffsll(mPE)-1;
		movearray[movecount].from = 1LL << (in - PE);
		movearray[movecount].dest = 1LL << in;
		movearray[movecount].info = (1LL << 5) ^ (1LL << 6) ^ stm;
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		//check fali, zapisi kao const osim stm
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
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		movecount++;
		}
		//check fali, zapisi kao const osim stm
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
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
		movecount++;
		}
		//check fali, zapisi kao const osim stm
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mN_q &= ~movearray[movecount].dest; 
			movecount++;
		}
		while (__builtin_popcountll(mN_c))
		{
			in = __builtin_ffsll(mN_c)-1;
			movearray[movecount].from = (1LL << in_N);
			movearray[movecount].dest = (1LL << in);
			movearray[movecount].info = (1LL << 4) ^ (1LL << 6) ^ stm;
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mB_q &= ~movearray[movecount].dest; 
			movecount++;
		}
		while (__builtin_popcountll(mB_c))
		{
			in = __builtin_ffsll(mB_c)-1;
			movearray[movecount].from = (1LL << in_B);
			movearray[movecount].dest = (1LL << in);
			movearray[movecount].info = (1LL << 3) ^ (1LL << 6) ^ stm;
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mR_q &= ~movearray[movecount].dest; 
			movecount++;
		}
		while (__builtin_popcountll(mR_c))
		{
			in = __builtin_ffsll(mR_c)-1;
			movearray[movecount].from = (1LL << in_R);
			movearray[movecount].dest = (1LL << in);
			movearray[movecount].info = (1LL << 2) ^ (1LL << 6) ^ stm;
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mQ_q &= ~movearray[movecount].dest; 
			movecount++;
		}
		while (__builtin_popcountll(mQ_c))
		{
			in = __builtin_ffsll(mQ_c)-1;
			movearray[movecount].from = (1LL << in_Q);
			movearray[movecount].dest = (1LL << in);
			movearray[movecount].info = (1LL << 1) ^ (1LL << 6) ^ stm;
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mQ_c &= ~movearray[movecount].dest; 
			movecount++;
		}
		fr->Q &= ~1LL << in_Q; 
	}
	
	//KING
	//while (__builtin_popcountll(fr->K))
	{
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mK_q &= ~movearray[movecount].dest; 
			movecount++;
		}
		while (__builtin_popcountll(mK_c))
		{
			in = __builtin_ffsll(mK_c)-1;
			movearray[movecount].from = (1LL << in_K);
			movearray[movecount].dest = (1LL << in);
			movearray[movecount].info = 1LL ^ (1LL << 6) ^ stm;
		//odredi captured piece
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
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			//check fali, zapisi kao const osim stm
			mK_c &= ~movearray[movecount].dest; 
			movecount++;
		}
		//fr->K &= ~1LL << in_K; 
	}
	//CASTLE
	if ( arg.info & cas_bit_K && !(ho->atack & cas_at_K) && !(arg.all_p & cas_occ_K) )
	{
			
			movearray[movecount].info = 1LL << 13 ^ stm;
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			movecount++;
	}
	if ( arg.info & cas_bit_Q && !(ho->atack & cas_at_Q) && !(arg.all_p & cas_occ_Q) )
	{
			movearray[movecount].info = 1LL << 14 ^ stm;
		//prenesi old_enp_sq i old castle
		movearray[movecount].info ^= (arg.info & 0x0000000000FFFF1E) << 32;
			movecount++;
	}
	//potrebno je odrolati pinned pieces 
	
	
	/*printf("ho.atackmap\n");
	printBits(8, &ho->atack);
	printf("checkgrid\n");
	printBits(8, &check_grid);*/


	return movecount;
}

U64 gen_ho_atack(board arg)
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
//		printf("white move\n");
	}
	else
	{
		fr = &arg.b;
		ho = &arg.w;
	at |= (ho->P & 0xFEFEFEFEFEFEFEFE) << 7;
	at |= (ho->P & 0x7F7F7F7F7F7F7F7F) << 9;
//		printf("black move\n");
	}
	//gledati napadnuta polja bez kralja zbog šahova
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
	
	//ho->atack &= ~arg.fr->pieces;		//!!!!!!!!!!!!!!!!!!!! samo prazna polja
	return at;
}

void do_move(board *b, move *m)
{
	U64 *p, *capt_p, *prom_p, stm, castle, cast_diff, hm, fm, m_cas, f_cas, empty = 0ULL, CK, CQ, KS, KR, QR, hKR, hQR, t;
	piece_set *fr, *ho; 
	int p_type, capt_type, prom_type, sq_from, sq_dest, enp;
	
	stm = b->info & 0x0000000000000001;
	fr = stm ? &b->w : &b->b;
	ho = stm ? &b->b : &b->w;
	KR = 0x0100000000000000 >> (stm*56);
	QR = 0x8000000000000000 >> (stm*56);
	hKR = 0x0000000000000001ULL << (stm*56);//stavi ULL da izbjegnes left shift warning
	hQR = 0x0000000000000080ULL << (stm*56);
	//castle = 0x000000000000001E;
	castle = 0LL;
	
	if (!(m->info & 0x0000000000007000))
	{

	
	p = m->info & 1LL      ? &fr->K : p;
	p = m->info & 1LL << 1 ? &fr->Q : p;
	p = m->info & 1LL << 2 ? &fr->R : p;
	p = m->info & 1LL << 3 ? &fr->B : p;
	p = m->info & 1LL << 4 ? &fr->N : p;
	p = m->info & 1LL << 5 ? &fr->P : p;
	
	
	
	// također uvjeti za zobrist broj key !!!!!!!!!!!!!!!!1111
	p_type = __builtin_ffsll( m->info )-1;
	sq_from = __builtin_ffsll( m->from)-1;
	sq_dest = __builtin_ffsll( m->dest)-1;
	//printf("p_t %d sq_fr %d sq_d %d\n", p_type, sq_from, sq_dest);
	
	b->zobrist ^= zobrist[ stm * 6*64 + p_type * 64 + sq_from];
	b->zobrist ^= zobrist[ stm * 6*64 + p_type * 64 + sq_dest];
	
	//castle
	/*t = (m->dest & hQR);
	printf("hqr-dest\n");
	printBits(8, &hQR);
	 0;t = ( m->info & 1LL << 26 );
	printf("hkr\n");
	printBits(8, &hKR);*/
	//provjera da li igra kralj ili je rokada fr gubi oba castla
	m_cas = 0x000000000000018 >> (stm<<1);
	f_cas = (m->info & 1LL) | (m->info & 0x000000000006000) >> 13;
	castle |= (castle & ~m_cas) | (-f_cas & m_cas);
	//provjera da li je top igrao fr gubi castle
	m_cas = 0x000000000000008 >> (stm<<1);
	f_cas = (m->info & 1LL << 2) && ( m->from & KR); 
	castle |= (castle & ~m_cas) | (-f_cas & m_cas);
	m_cas = 0x000000000000010 >> (stm<<1);
	f_cas = (m->info & 1LL << 2) && ( m->from & QR);
	castle |= (castle & ~m_cas) | (-f_cas & m_cas);
	//provjera da li je top pojeden ho gubi castle 
//nije provjereno jel je mask na vecoj mjeri od uvjeta
	m_cas = 0x000000000000002 << (stm<<1);
	f_cas = (m->dest & hKR) && ( m->info & 1LL << 26 );
	castle |= (castle & ~m_cas) | (-f_cas & m_cas);
	//printf("fcasK\n");
	//printBits(8, &f_cas);
	 
	m_cas = 0x000000000000004 << (stm<<1);
	f_cas = (m->dest & hQR) && ( m->info & 1LL << 26 );
	castle |= (castle & ~m_cas) | (-f_cas & m_cas);
	//printf("fcasQ\n");
	//printBits(8, &f_cas);



	//b->info & 0x000000000000006 (m->info & 1LL << 26 && (p->R & 0x0000000000000001) )

	//doing move
	//printf("prije\n");
	//printBits(8, p);
	*p &= ~m->from;
	*p |= m->dest;
	//printf("poslije\n");
	//printBits(8, p);
	
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
	//printf("cap %d\n", capt_type);
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
	//printf("prom %d\n", prom_type);
	b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + prom_type * 64 + 64 + sq_dest] : 0ULL;
	b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + 5 * 64 + sq_dest] : 0ULL;
	
	
	//update all_p
	b->all_p &= ~m->from;
	b->all_p |= m->dest;
	}
	else if (m->info & 0x0000000000001000)
	{
	//hm = ((m->info & 0x0000000000001000));
	//printf("enpuvjet\n");
	//printBits(8, &hm);

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
	//hm = ((m->info & 0x0000000000002000));
	//printf("casK\n");
	//printBits(8, &hm);

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
	//hm = ((m->info & 0x0000000000006000));
	//printf("casQ\n");
	//printBits(8, &hm);

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
	
	
	//half move korak je 1 ako potez nije capture ili pawn advance
	hm = m->info & 0x000000003E008000 ? -(b->info & 0x0000000000FF0000) : (1LL << 16);

	// b_info = stm ^ castle ^ doublepawnpush ^ halfmove ^ fullmove
	
	/*printf("b-dpp\n");
	t = b->info & 0x000000000000FF00;
	printBits(8, &t);
	printf("m-dpp\n");
	t = m->info & 0x0000000000FF0000;
	printBits(8, &t);*/

	
	//zobrist
	//printBits(8, &cast_diff);
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

void undo_move(board *b, move *m)
{
	U64 *p, *capt_p, *prom_p, stm, empty = 0LL, hm, CK, CQ, KS, KR, QR, cast_diff;
	piece_set *fr, *ho; 
	int p_type, sq_from, sq_dest, capt_type, prom_type, enp;
	
	stm = m->info >> 24 & 0x0000000000000001 ;
	//stm = !(b->info & 1LL) ;
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

	// također uvjeti za zobrist broj key !!!!!!!!!!!!!!!!1111
	p_type = __builtin_ffsll( m->info )-1;
	sq_from = __builtin_ffsll( m->from)-1;
	sq_dest = __builtin_ffsll( m->dest)-1;
	//printf("UN p_t %d sq_fr %d sq_d %d\n", p_type, sq_from, sq_dest);
	
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
	//printf("UNcap %d\n", capt_type);
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
	//printf("UNprom %d\n", prom_type);
	b->zobrist ^= prom_type ? zobrist[ stm * 6*64 + 5 * 64 + sq_dest] : 0ULL;

	//update all_p
	b->all_p &= m->info & 0x000000003E000000 ? ~0LL : ~m->dest;
	//b->all_p &= ~m->dest;
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
		//printf("UNzob_ind %llu, stm %llu sq_d %d\n", stm * 6*64 + 5 * 64 + sq_dest, stm, sq_dest);
		b->zobrist ^= zobrist[ stm * 6*64 + 5 * 64 + sq_dest];
		b->zobrist ^= zobrist[ stm * 6*64 + 5 * 64 + sq_from];
		b->zobrist ^= zobrist[ !stm * 6*64 + 5 * 64 + sq_dest + 8 - stm*16 ];

	}
	else if (m->info & 0x0000000000002000)
	{
	/*hm = ((m->info & 0x0000000000002000));
	printf("casK\n");
	printBits(8, &hm);*/

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
	/*hm = ((m->info & 0x0000000000006000));
	printf("casQ\n");
	printBits(8, &hm);*/

		KR = 0x0100000000000000 >> stm*56;//mozda && ili ?
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
	//printBits(8, &cast_diff);
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
	// b_info = old_castle old_enp old_hm | b_fm; 
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
	//int stm, capture, check, enp, castle, pawn_adv, dest, from, ;
	int piece_type, count = 0;
	short d_pa;
	char promotion, fr[3], dst[3];

		piece_type = __builtin_ffsll(m_l->info)-1;
		/*capture = m_l->info >> 6 & 1LL;
		check = m_l->info >> 7 & 1LL;
		promotion = (m_l->info & 0x0000000000000F00) >> 8;
		enp = m_l->info >> 12 & 1LL;
		castle = m_l->info >> 13 & 1LL ? 1 : 0;
		castle += m_l->info >> 14 & 1LL ? 2 : 0;
		pawn_adv = m_l->info >> 15 & 1LL;
		d_pa = (m_l->info & 0x0000000000FF0000) >> 16;
		stm = (m_l->info & 0x0000000001000000) >> 24;;*/
		//dest = __builtin_ffsll(m_l->dest)-1;
		//from = __builtin_ffsll(m_l->from)-1;
		square( __builtin_ffsll(m_l->dest)-1, dst);
		square( __builtin_ffsll(m_l->from)-1, fr);

		//printf("%s%s%s capt%d enp%d castle%d pawn_adv%d check%d", 
		//	piece(piece_type), fr, dst, capture,enp,castle,pawn_adv,check);
		if (piece_type == 13 || piece_type == 14) 	printf("%s ", piece(piece_type));
		else	printf("%s%s%s ", piece(piece_type), fr, dst);
		//printBits(1, &d_pa);
		//printBits(1, &promotion);
		//printBits(8, &m_l->info);
}

void print_move_xboard(Nmove *m)
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
		printf("%s%s\n", fr, dest);//printf("%s%s%s ", piece(5), fr, dst);
	else	
		printf("%s%s\n", fr, dest);

}


void printmoveN(Nmove *m)
{
	//int stm, capture, check, enp, castle, pawn_adv, dest, from, ;
	int piece_type, capt_type, from, dest, count = 0;
	short d_pa;
	char promotion, fr[3], dst[3];

		piece_type = *m & 0x0000000000000007;
		capt_type = (*m >> 3) & 0x0000000000000007;
                from = (*m >> 6) & 0x000000000000003F;
                dest = (*m >> 12) & 0x000000000000003F;
		square( dest, dst);
		square( from, fr);
        	/*printf("p %d capt %d\n", piece_type, capt_type);
	        printf("Dfr %d dst %d \n", from, dest);
	        printf("m\n");
	        printBits(4, &m);*/
	        
                

                
		//printf("%s%s%s capt%d enp%d castle%d pawn_adv%d check%d", 
		//	piece(piece_type), fr, dst, capture,enp,castle,pawn_adv,check);
		/*if (piece_type == 6 ) 	printf("%s ", piece(capt_type));
		else if (piece_type == 7 ) 	printf("%s%s%s ", piece(5), fr, dst);
		else	printf("%s%s%s ", piece(piece_type), fr, dst);*/
        
		if (piece_type == 7 ) 	printf("%s%s ", fr, dst);
		else	printf("%s%s ", fr, dst);

}

void printmovedetailsN(Nmove *ff)
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

char *piece(int index)
{
	switch (index) 
	{
	 case 0: return "K";
	 case 1: return "Q";
	 case 2: return "R";
	 case 3: return "B";
	 case 4: return "N";
	 case 5: return "p";
	 case 6: return "O-O";
	 case 7: return "O-O-O";
	 case 13: return "O-O";
	 case 14: return "O-O-O";
	}
}

void square(int index, char *sq)
{	
	//char sq[2];
	// h -- 104; a -- 97;
	sq[0] = 104 - index % 8;
	// 1 -- 49; 7 -- 55;
	sq[1] = index / 8 + 49;
	sq[2] = 0;
	
}

void square2(char *sq, int *index)
{
	*index = (sq[1]-49)*8 + (104-sq[0]);
}

void printBits(size_t const size, void const * const ptr)
{
    unsigned char *b = (unsigned char*) ptr;
    unsigned char byte;
    int i, j;

    for (i=size-1;i>=0;i--)
    {
        for (j=7;j>=0;j--)
        {
            byte = b[i] & (1<<j);
            byte >>= j;
            printf("%u", byte);
        }
    puts("");
    }
	printf("\n");
}

void print_state(Nboard arg)
{
  char i;
	for ( i = 0; i < 17; i++)
	{
	  printf("pieceset[   %d    ]\n", i);
	  printBits(8, &arg.pieceset[i]);
	  
	}
	  printBits(8, &arg.info);

}

