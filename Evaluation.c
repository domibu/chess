#include <stdio.h>
#include <stdlib.h>
#include "BitBoardMagic.h"
#include "MoveGeneration.h"
#include "Evaluation.h"
#include "Search.h"

//king, queen, rook, bishop, knight, pawn
int material_score_mg[6] = {0, 2538, 1276, 825, 781, 124};
int material_score_eg[6] = {0, 2682, 1380, 915, 854, 206};

int mobility_knight_bonus_mg[9] = {
    -62,-53,-12,-4,3,13,22,28,33
};

int mobility_bishop_bonus_mg[14] = {
    -48,-20,16,26,38,51,55,63,63,68,81,81,91,98
};

int mobility_rook_bonus_mg[15] = {
    -60, -20, 2, 3, 3, 11, 22, 31,
	40, 41, 48, 57, 57, 57, 62
};

int mobility_queen_bonus_mg[28] = {
    -30,-12,-8,-9,20,23,23,35,38,53,64,65,65,66,67,67,72,72,77,79,93,108,108,108,110,114,114,116
};

int mobility_knight_bonus_eg[9] = {
    -81,-56,-31,-16,5,11,17,20,25
};

int mobility_bishop_bonus_eg[14] = {
    -59,-23,-3,13,24,42,54,57,65,73,78,86,88,97
};

int mobility_rook_bonus_eg[16] = {
    -78,-17,23,39,70,99,103,103,121,134,139,158,164,168,169,172
};

int mobility_queen_bonus_eg[28] = {
    -48,-30,-7,19,40,55,59,75,78,96,96,100,121,127,131,133,136,141,147,150,151,168,168,171,182,182,192,219
};


int knight_psqt_mg_flip[64];
int knight_psqt_mg[64] = {
	-201,-83,-56,-26,-26,-56,-83,-201
	-67,-27,4,37,37,4,-27,-67,
	-9,22,58,53,53,58,22,-9,
	-34,13,44,51,51,44,13,-34,
	-35,8,40,49,49,40,8,-35,
	-61,-17,6,12,12,6,-17,-61,
	-77,-41,-27,-15,-15,-27,-41,77,
	-175,-92,-74,-73,-73,-74,-92,-175
};

int bishop_psqt_mg_flip[64];
int bishop_psqt_mg[64] = {
	-48,1,-14,-23,-23,-14,1,-48,
	-17,-14,5,0,0,5,-14,-17,
	-16,6,1,11,11,1,6,-16,
	-12,29,22,31,31,22,29,-12,
	-5,11,25,39,39,25,11,-5,
	-7,21,-5,17,17,-5,21,-7,
	-15,8,19,4,4,19,8,-15,
	-53,-5,-8,-23,-23,-8,-5,-53
};

int rook_psqt_mg_flip[64];
int rook_psqt_mg[64] = {
	-17,-19,-1,9,9,-1,-19,-17,
	-2,12,16,18,18,16,12,-2,
	-22,-2,6,12,12,6,-2,-22,
	-27,-15,-4,3,3,-4,15,-27,
	-13,-5,-4,-6,-6,-4,-5,-13,
	-25,-11,-1,3,3,-1,-11,-25,
	-21,-13,-8,6,6,-8,-13,-21,
	-31,-20,-14,-5,-5,-14,-20,-31

};

int queen_psqt_mg_flip[64];
int queen_psqt_mg[64] = {
	-2,-2,1,-2,-2,1,-2,-2,
	-5,6,10,8,8,10,6,-5,
	-4,10,6,8,8,6,10,-4,
	0,14,12,5,5,12,14,0,
	4,5,9,8,8,9,5,4,
	-3,6,13,7,7,13,6,-3,
	-3,5,8,12,12,8,5,-3,
	3,-5,-5,4,4,-5,-5,3
};

int king_psqt_mg_flip[64];
int king_psqt_mg[64] = {
	59,89,45,-1,-1,45,89,59,
	88,120,65,33,33,65,120,88,
	123,145,81,31,31,81,145,123,
	154,179,105,70,70,105,179,154,
	164,190,138,98,98,138,190,164,
	195,258,169,120,120,169,258,195,
	278,303,234,179,179,234,303,278,
	271,327,271,198,198,271,327,271
};

int pawn_psqt_mg_flip[64];
int pawn_psqt_mg[64] = {
	0,0,0,0,0,0,0,0,
	-7,7,-3,-13,5,-16,10,-8,
	5,-12,-7,22,-8,-5,-15,-8,
	13,0,-13,1,11,-2,-13,5,
	-4,-23,6,20,40,17,4,-8,
	-9,-15,11,15,32,22,5,-2,
	3,3,10,19,16,19,7,-5,
	0,0,0,0,0,0,0,0
};

int pawn_psqt_eg_flip[64];
int pawn_psqt_eg[64] = {
	0,0,0,0,0,0,0,0,
	0,-11,12,21,25,19,4,7,
	28,20,21,28,30,7,6,13,
	10,5,4,-5,-5,-5,14,9,
	6,-2,-8,-4,-13,-12,-10,-9,
	-10,-10,-10,4,4,3,-6,-4,
	-10,-6,10,0,14,7,-5,-19,
	0,0,0,0,0,0,0,0
};

void flip_psqt_val(int *in, int *out)
{
	int x, y; 
	for (int i=0; i < 64;i++)
	{
		x = i % 8;
		y = i / 8;
		out[(7-y) * 8 + x] = in[i];
	}
}

void init_flipped_psqt()
{
	flip_psqt_val(pawn_psqt_mg, pawn_psqt_mg_flip);
	flip_psqt_val(knight_psqt_mg, knight_psqt_mg_flip);
	flip_psqt_val(bishop_psqt_mg, bishop_psqt_mg_flip);
	flip_psqt_val(rook_psqt_mg, rook_psqt_mg_flip);
	flip_psqt_val(queen_psqt_mg, queen_psqt_mg_flip);
	flip_psqt_val(king_psqt_mg, king_psqt_mg_flip);

	flip_psqt_val(pawn_psqt_eg, pawn_psqt_eg_flip);
}

eval_score init_eval()
{
	eval_score result;
	
	result.material_f_eg = 0;
	result.material_h_eg = 0;
	result.material_f_mg = 0;
	result.material_h_mg = 0;
	result.pawns_f_eg = 0;
	result.pawns_h_eg = 0;
	result.pawns_f_mg = 0;
	result.pawns_h_mg = 0;
	result.mobility_f_eg = 0;
	result.mobility_h_eg = 0;
	result.mobility_f_mg = 0;
	result.mobility_h_mg = 0;
	result.psqt_f_eg = 0;
	result.psqt_h_eg = 0;
	result.psqt_f_mg = 0;
	result.psqt_h_mg = 0;

	return result;
}

int evaluate( board arg, int draft, int color, board *rb)
{
	U64 blank = 0LL;
	int move_count = 0, piece_count = 0;
	eval_score eval;

	eval = init_eval();

	U64 stm;
	U64 *fr, *ho;
	U64 f_cas_bit_K, f_cas_bit_Q, f_cas_at_K, f_cas_at_Q, f_cas_occ_K, f_cas_occ_Q;
	U64 f_promotion, f_ENProw, f_enp_P, f_enp;
	unsigned char f_enp_sq;
	U64 f_mobility_area_mask;

	stm = (arg.info >> 11) & 0x0000000000000008;

	fr = stm ? &arg.pieceset[0] : &arg.pieceset[8];
	f_cas_bit_K = stm ? 0x0000000000000010 : 0x0000000000000040;
	f_cas_bit_Q = stm ? 0x0000000000000020 : 0x0000000000000080;
	f_cas_at_K =  stm ? 0x000000000000000E : 0x0E00000000000000;
	f_cas_at_Q =  stm ? 0x0000000000000038 : 0x3800000000000000;
	f_cas_occ_K = stm ? 0x0000000000000006 : 0x0600000000000000;
	f_cas_occ_Q = stm ? 0x0000000000000070 : 0x7000000000000000;
	f_promotion = stm ? 0xFF00000000000000 : 0x00000000000000FF;
	f_enp_sq = stm ? (arg.info >> 1 & 0x0000000000000007) + 40 : (arg.info >> 1 & 0x0000000000000007) + 16;
	f_enp = (1LL << f_enp_sq)*(arg.info & 1LL);
	f_enp_P = stm ? f_enp >> 8 : f_enp << 8;
	f_ENProw = stm ? 0x000000FF00000000 : 0x00000000FF000000;

	f_mobility_area_mask = stm ? 0x0000000000FFFFFF : 0xFFFFFF0000000000;

	U64 h_cas_bit_K, h_cas_bit_Q, h_cas_at_K, h_cas_at_Q, h_cas_occ_K, h_cas_occ_Q;
	U64 h_promotion, h_ENProw, h_enp_P, h_enp;
	unsigned char h_enp_sq;
	U64 h_mobility_area_mask;
 
	ho = stm ? &arg.pieceset[8] : &arg.pieceset[0];
	h_cas_bit_K = stm ? 0x0000000000000040 : 0x0000000000000040;
	h_cas_bit_Q = stm ? 0x0000000000000080 : 0x0000000000000080;
	h_cas_at_K =  stm ? 0x0E00000000000000 : 0x0E00000000000000;
	h_cas_at_Q =  stm ? 0x3800000000000000 : 0x3800000000000000;
	h_cas_occ_K = stm ? 0x0600000000000000 : 0x0600000000000000;
	h_cas_occ_Q = stm ? 0x7000000000000000 : 0x7000000000000000;
	h_promotion = stm ? 0x00000000000000FF : 0x00000000000000FF;
	h_enp_sq = stm ? (arg.info >> 1 & 0x0000000000000007) + 16 : (arg.info >> 1 & 0x0000000000000007) + 16;
	//disabled --> 0LL enp
	h_enp = (1LL << h_enp_sq)*(arg.info & 0LL);
	h_enp_P = stm ? h_enp >> 8 : h_enp << 8;
	h_ENProw = stm ? 0x000000FF00000000 : 0x00000000FF000000;

	h_mobility_area_mask = stm ? 0xFFFFFF0000000000 : 0x0000000000FFFFFF;

	//score = eval_side_score(arg, 0, color, &arg, &mob_f_mg, &mob_f_eg);
	//arg.info ^= 0x0000000000000008 << 11;

	//building atack map pieces, later wont be necessary becuase it will be updated incrementally by make, unmake move<$1>
	fr[6] = fr[5] ^ fr[4] ^ fr[2] ^ fr[3] ^ fr[0] ^ fr[1];
	ho[6] = ho[5] ^ ho[4] ^ ho[2] ^ ho[3] ^ ho[0] ^ ho[1];
	arg.pieceset[16] = fr[6] ^ ho[6];


	ho[7] = generate_hostile_atackmap(arg);
	arg.info ^= 0x0000000000000008 << 11;
	fr[7] = generate_hostile_atackmap(arg);
	//TODO make stm switch macro
	arg.info ^= 0x0000000000000008 << 11;

	U64 check_grid = ~0LL, ho_check_grid = ~0LL;

	check_grid &= generate_check_grid(&arg, stm);
	ho_check_grid &= generate_check_grid(&arg, stm ^ (0x0000000000000008 << 11));

	int f_king, h_king;
	f_king = __builtin_ffsll(fr[0])-1;
	h_king = __builtin_ffsll(ho[0])-1;

	//pieces pinned diagonaly<1$>
	U64 f_all_pp = 0LL, f_ppb = 0LL, f_ppr = 0LL;
	int in_at_b, at, in_at, in_k, in_pp, mpp;
	U64 at_b, bb, mpb;

	in_at_b = ( blank * magicNumberBishop[f_king] ) >> magicNumberShiftsBishop[f_king];
	at_b = magicMovesBishop[f_king][in_at_b] & (ho[3] ^ ho[1]); //friendly pieces<$1>
	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.pieceset[16] & occupancyMaskBishop[f_king]) * magicNumberBishop[f_king]) >> magicNumberShiftsBishop[f_king];
		f_ppb = magicMovesBishop[f_king][in_k] & magicMovesBishop[at][in_at];

		f_all_pp ^= f_ppb;

		in_pp = __builtin_ffsll(f_ppb)-1;
		bb = arg.pieceset[16] & ~ (1LL << in_pp);

		in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		in_k = ((bb & occupancyMaskBishop[f_king]) * magicNumberBishop[f_king]) >> magicNumberShiftsBishop[f_king];
		mpb = magicMovesBishop[f_king][in_k] & magicMovesBishop[at][in_at] &  ~f_ppb ^ (1LL << at);
		mpb &= check_grid;

		if (__builtin_popcountll( (fr[1] ^ fr[3]) & f_ppb )) //maybe popcount isn't neccessary<1$>
		{

				//TODO mobility area not defined yet
				eval.mobility_f_mg += (fr[3] & f_ppb) ? mobility_bishop_bonus_mg[__builtin_popcountll(mpb /*& mobility_area*/)] : mobility_queen_bonus_mg[__builtin_popcountll(mpb /*& mobility_area*/)];
				eval.mobility_f_eg += (fr[3] & f_ppb) ? mobility_bishop_bonus_eg[__builtin_popcountll(mpb /*& mobility_area*/)] : mobility_queen_bonus_eg[__builtin_popcountll(mpb /*& mobility_area*/)];


			move_count += __builtin_popcountll(mpb);
		}
		//checking for pawn<1$>
		if ( fr[5] & f_ppb)
		{
			in_pp = __builtin_ffsll(f_ppb)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
			if (stm) mpp =__builtin_ffsll( mpb & (((f_ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((f_ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & ~f_promotion & check_grid);
			else mpp = __builtin_ffsll( mpb & (((f_ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((f_ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & ~f_promotion & check_grid );
			//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>
			//without promotion<1$>
			if (mpp)
				move_count += __builtin_popcountll(mpp);
			else
			{
				//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>
				if (stm) mpp =__builtin_ffsll( mpb & (((f_ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((f_ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & ho[6] & f_promotion & check_grid);
				else mpp = __builtin_ffsll( mpb & (((f_ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((f_ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & ho[6] & f_promotion & check_grid );
				//promotion
				if (mpp)
					move_count += __builtin_popcountll(mpp);
				else
				{
					if (stm) mpp =__builtin_ffsll( mpb & (((f_ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((f_ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (ho[6]^f_enp) & check_grid);
					else mpp = __builtin_ffsll( mpb & (((f_ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((f_ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (ho[6] ^ f_enp) & check_grid);
					//promotion excluded when en'passan<1$>
					if (mpp)
						move_count += __builtin_popcountll(mpp);
				}
			}
		}
		at_b &= ~(1LL << at);
	}

	U64 h_all_pp = 0LL, h_ppb = 0LL, h_ppr = 0LL;

	in_at_b = ( blank * magicNumberBishop[h_king] ) >> magicNumberShiftsBishop[h_king];
	at_b = magicMovesBishop[h_king][in_at_b] & (fr[3] ^ fr[1]); //friendly pieces<$1>
	while (__builtin_popcountll(at_b) )
	{
		at = __builtin_ffsll(at_b)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.pieceset[16] & occupancyMaskBishop[h_king]) * magicNumberBishop[h_king]) >> magicNumberShiftsBishop[h_king];
		h_ppb = magicMovesBishop[h_king][in_k] & magicMovesBishop[at][in_at];

		h_all_pp ^= h_ppb;

		in_pp = __builtin_ffsll(h_ppb)-1;
		bb = arg.pieceset[16] & ~ (1LL << in_pp);

		in_at = ((bb & occupancyMaskBishop[at]) * magicNumberBishop[at]) >> magicNumberShiftsBishop[at];
		in_k = ((bb & occupancyMaskBishop[h_king]) * magicNumberBishop[h_king]) >> magicNumberShiftsBishop[h_king];
		mpb = magicMovesBishop[h_king][in_k] & magicMovesBishop[at][in_at] &  ~h_ppb ^ (1LL << at);
		mpb &= ho_check_grid;

		if (__builtin_popcountll( (ho[1] ^ ho[3]) & h_ppb )) //maybe popcount isn't neccessary<1$>
		{
				//TODO mobility area not defined yet
				eval.mobility_h_mg += (ho[3] & h_ppb) ? mobility_bishop_bonus_mg[__builtin_popcountll(mpb /*& mobility_area*/)] : mobility_queen_bonus_mg[__builtin_popcountll(mpb /*& mobility_area*/)];
				eval.mobility_h_eg += (ho[3] & h_ppb) ? mobility_bishop_bonus_eg[__builtin_popcountll(mpb /*& mobility_area*/)] : mobility_queen_bonus_eg[__builtin_popcountll(mpb /*& mobility_area*/)];

		}
		//checking for pawn<1$>
		// if ( ho[5] & h_ppb)
		// {
		// 	in_pp = __builtin_ffsll(h_ppb)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
		// 	if (!stm) mpp =__builtin_ffsll( mpb & (((h_ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((h_ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & fr[6] & ~h_promotion & ho_check_grid);
		// 	else mpp = __builtin_ffsll( mpb & (((h_ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((h_ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & fr[6] & ~h_promotion & ho_check_grid );
		// 	//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>
		// 	//without promotion<1$>
		// 	if (mpp)
		// 		move_count += __builtin_popcountll(mpp);
		// 	else
		// 	{
		// 		//add limit for a,h file depending of direction of capture, possible that limits aren't needed<1$>
		// 		if (!stm) mpp =__builtin_ffsll( mpb & (((h_ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((h_ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & fr[6] & h_promotion & ho_check_grid);
		// 		else mpp = __builtin_ffsll( mpb & (((h_ppb & 0x7F7F7F7F7F7F7F7F) >> 7) ^ ((h_ppb & 0xFEFEFEFEFEFEFEFE) >> 9)) & fr[6] & h_promotion & ho_check_grid );
		// 		//promotion
		// 		if (mpp)
		// 			move_count += __builtin_popcountll(mpp);
		// 		else
		// 		{
		// 			if (!stm) mpp =__builtin_ffsll( mpb & (((h_ppb & 0x7F7F7F7F7F7F7F7F)<<9) ^ ((h_ppb & 0xFEFEFEFEFEFEFEFE)<<7)) & (fr[6] ^ h_enp) & ho_check_grid);
		// 			else mpp = __builtin_ffsll( mpb & (((h_ppb & 0x7F7F7F7F7F7F7F7F)>>7) ^ ((h_ppb & 0xFEFEFEFEFEFEFEFE)>>9)) & (fr[6] ^ h_enp) & ho_check_grid);
		// 			//promotion excluded when en'passan<1$>
		// 			if (mpp)
		// 				move_count += __builtin_popcountll(mpp);
		// 		}
		// 	}
		// }
		at_b &= ~(1LL << at);
	}

	//pieces pinned by file or rank<1$>
	int in_at_r;
	U64 at_r, mpr;


	in_at_r = ( blank * magicNumberRook[f_king] ) >> magicNumberShiftsRook[f_king];
	at_r = magicMovesRook[f_king][in_at_r] & (ho[2] ^ ho[1]); //friendly pieces<$1>
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.pieceset[16] & occupancyMaskRook[f_king]) * magicNumberRook[f_king]) >> magicNumberShiftsRook[f_king];
		f_ppr= magicMovesRook[f_king][in_k] & magicMovesRook[at][in_at];
		f_all_pp ^= f_ppr;

		in_pp = __builtin_ffsll(f_ppr)-1;
		bb = arg.pieceset[16] & ~ (1LL << in_pp);
		in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		in_k = ((bb & occupancyMaskRook[f_king]) * magicNumberRook[f_king]) >> magicNumberShiftsRook[f_king];
		mpr = magicMovesRook[f_king][in_k] & magicMovesRook[at][in_at] &  ~f_ppr ^ (1LL << at);
		mpr &= check_grid;

		if (__builtin_popcountll( (fr[1] ^ fr[2]) & f_ppr ))
		{
				eval.mobility_f_mg += (fr[2] & f_ppr) ? mobility_rook_bonus_mg[__builtin_popcountll(mpr /*& mobility_area*/)] : mobility_queen_bonus_mg[__builtin_popcountll(mpr /*& mobility_area*/)];
				eval.mobility_f_eg += (fr[2] & f_ppr) ? mobility_rook_bonus_eg[__builtin_popcountll(mpr /*& mobility_area*/)] : mobility_queen_bonus_eg[__builtin_popcountll(mpr /*& mobility_area*/)];
	
				move_count += __builtin_popcountll(mpr);
		}
		//checking for pawn<1$>
		if ( fr[5] & f_ppr)
		{
			in_pp = __builtin_ffsll(f_ppr)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
			if (stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & f_ppr) << 8) & ~arg.pieceset[16]) << 8) & ~arg.pieceset[16] & check_grid);//include just two rank step<1$>
			else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & f_ppr) >> 8) & ~arg.pieceset[16]) >> 8) & ~arg.pieceset[16] & check_grid);//include just two rank step<1$>
			//pawn advance or pawn capture<1$>
			//without promotion<1$>
			if (mpp)
				move_count += __builtin_popcountll(mpp);
			//else
			mpp = (stm) ? __builtin_ffsll( mpr & (f_ppr << 8) & ~arg.pieceset[16]) & check_grid
				: __builtin_ffsll( mpr & (f_ppr >> 8) & ~arg.pieceset[16]) & check_grid;//include just one rank step<1$>
			if (mpp)
				move_count += __builtin_popcountll(mpp);
		}
		at_r &= ~(1LL << at);
	}
	
	in_at_r = ( blank * magicNumberRook[h_king] ) >> magicNumberShiftsRook[h_king];
	at_r = magicMovesRook[h_king][in_at_r] & (fr[2] ^ fr[1]); //friendly pieces<$1>
	while (__builtin_popcountll(at_r) )
	{
		at = __builtin_ffsll(at_r)-1;
		in_at = ((arg.pieceset[16] & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		//index of king can be outside of while loop<$1>
		in_k = ((arg.pieceset[16] & occupancyMaskRook[h_king]) * magicNumberRook[h_king]) >> magicNumberShiftsRook[h_king];
		h_ppr= magicMovesRook[h_king][in_k] & magicMovesRook[at][in_at];
		h_all_pp ^= h_ppr;

		in_pp = __builtin_ffsll(h_ppr)-1;
		bb = arg.pieceset[16] & ~ (1LL << in_pp);
		in_at = ((bb & occupancyMaskRook[at]) * magicNumberRook[at]) >> magicNumberShiftsRook[at];
		in_k = ((bb & occupancyMaskRook[h_king]) * magicNumberRook[h_king]) >> magicNumberShiftsRook[h_king];
		mpr = magicMovesRook[h_king][in_k] & magicMovesRook[at][in_at] &  ~h_ppr ^ (1LL << at);
		mpr &= ho_check_grid;

		if (__builtin_popcountll( (ho[1] ^ ho[2]) & h_ppr ))
		{
				//TODO mobility area not defined yet
				eval.mobility_h_mg += (ho[2] & h_ppr) ? mobility_rook_bonus_mg[__builtin_popcountll(mpr /*& mobility_area*/)] : mobility_queen_bonus_mg[__builtin_popcountll(mpr /*& mobility_area*/)];
				eval.mobility_h_eg += (ho[2] & h_ppr) ? mobility_rook_bonus_eg[__builtin_popcountll(mpr /*& mobility_area*/)] : mobility_queen_bonus_eg[__builtin_popcountll(mpr /*& mobility_area*/)];
	
			//move_count += __builtin_popcountll(mpr);
		}
		//checking for pawn<1$>
		// if ( ho[5] & h_ppr)
		// {
		// 	in_pp = __builtin_ffsll(h_ppr)-1;  //maybe possible to use pp from above because one at has one pinned_piece<1$>
		// 	if (!stm)	mpp = __builtin_ffsll( mpr & ((((0x000000000000FF00 & h_ppr) << 8) & ~arg.pieceset[16]) << 8) & ~arg.pieceset[16] & ho_check_grid);//include just two rank step<1$>
		// 	else mpp = __builtin_ffsll( mpr & ((((0x00FF000000000000 & h_ppr) >> 8) & ~arg.pieceset[16]) >> 8) & ~arg.pieceset[16] & ho_check_grid);//include just two rank step<1$>
		// 	//pawn advance or pawn capture<1$>
		// 	//without promotion<1$>
		// 	//if (mpp)
		// 		move_count += __builtin_popcountll(mpp);
		// 	//else
		// 	mpp = (!stm) ? __builtin_ffsll( mpr & (h_ppr << 8) & ~arg.pieceset[16]) & ho_check_grid
		// 		: __builtin_ffsll( mpr & (h_ppr >> 8) & ~arg.pieceset[16]) & ho_check_grid;//include just one rank step<1$>
		// 	if (mpp)
		// 		move_count += __builtin_popcountll(mpp);
		// }
		at_r &= ~(1LL << at);
	}
	
	U64 f_PE, f_PW, oPE, oPW;
	U64 f_mP1, f_mP2, f_mPE, f_mPW, f_mENP, f_mP1_prom, f_mPE_prom, f_mPW_prom;
	//PAWN

	piece_count = __builtin_popcountll(fr[5]);
	eval.material_f_mg += material_score_mg[5] * piece_count;
	eval.material_f_eg += material_score_eg[5] * piece_count;
	piece_count = __builtin_popcountll(ho[5]);
	eval.material_h_mg += material_score_mg[5] * piece_count;
	eval.material_h_eg += material_score_eg[5] * piece_count;

	fr[5] &= ~ f_all_pp;
	ho[5] &= ~ h_all_pp;
	if (stm)
	{
		f_PE = (fr[5] & 0xFEFEFEFEFEFEFEFE) << 7;
		f_PW = (fr[5] & 0x7F7F7F7F7F7F7F7F) << 9;

		f_mP2 = ((fr[5] & 0x000000000000FF00) << 8 & ~arg.pieceset[16]) << 8 & ~arg.pieceset[16] & check_grid;
		f_mP1 = fr[5] << 8 & ~arg.pieceset[16] & check_grid;
		f_mPE = f_PE & ho[6] & check_grid;
		f_mPW = f_PW & ho[6] & check_grid;
		f_mENP = (f_enp & 0xFEFEFEFEFEFEFEFE & check_grid) >> 9 & fr[5] ^ (f_enp & 0x7F7F7F7F7F7F7F7F & check_grid) >> 7 & fr[5];
		f_mENP |= (f_enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5] ^ (f_enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5];
		f_mP1_prom = f_mP1 & f_promotion;
		f_mPE_prom = f_mPE & f_promotion;
		f_mPW_prom = f_mPW & f_promotion;
		f_mP1 &= ~f_promotion;
		f_mPE &= ~f_promotion;
		f_mPW &= ~f_promotion;

		//opponent with promo
		oPE = (ho[5] & 0xFEFEFEFEFEFEFEFE) >> 9;
		oPW = (ho[5] & 0x7F7F7F7F7F7F7F7F) >> 7;
	}
	else
	{
		f_PE = (fr[5] & 0xFEFEFEFEFEFEFEFE) >> 9;
		f_PW = (fr[5] & 0x7F7F7F7F7F7F7F7F) >> 7;

		f_mP2 = ((fr[5] & 0x00FF000000000000) >> 8 & ~arg.pieceset[16]) >> 8 & ~arg.pieceset[16] & check_grid;
		f_mP1 = fr[5] >> 8 & ~arg.pieceset[16] & check_grid;
		f_mPE = f_PE & ho[6] & check_grid;
		f_mPW = f_PW & ho[6] & check_grid;
		f_mENP = (f_enp & 0x7F7F7F7F7F7F7F7F & check_grid) << 9 & fr[5] ^ (f_enp & 0xFEFEFEFEFEFEFEFE & check_grid) << 7 & fr[5];
		f_mENP |= (f_enp_P & 0x7F7F7F7F7F7F7F7F & check_grid) << 1 & fr[5] ^ (f_enp_P & 0xFEFEFEFEFEFEFEFE & check_grid) >> 1 & fr[5];
		f_mP1_prom = f_mP1 & f_promotion;
		f_mPE_prom = f_mPE & f_promotion;
		f_mPW_prom = f_mPW & f_promotion;
		f_mP1 &= ~f_promotion;
		f_mPE &= ~f_promotion;
		f_mPW &= ~f_promotion;

		//opponent with promo
		oPE = (ho[5] & 0xFEFEFEFEFEFEFEFE) << 7;// & ho_check_grid;
		oPW = (ho[5] & 0x7F7F7F7F7F7F7F7F) << 9;// & ho_check_grid;
	}

	U64 f_mobility_area = 0LL, h_mobility_area = 0LL;

	f_mobility_area = 0xFFFFFFFFFFFFFFFF & (~fr[0] & ~fr[1] & ~oPE & ~oPW & ~(fr[5] & f_mobility_area_mask) & ~(fr[5] & arg.pieceset[16] >> 8) & ~f_ppb & ~f_ppr);   
	h_mobility_area = 0xFFFFFFFFFFFFFFFF & (~ho[0] & ~ho[1] & ~f_PE & ~f_PW & ~(ho[5] & h_mobility_area_mask) & ~(ho[5] & arg.pieceset[16] << 8) & ~h_ppb & ~h_ppr);   
	//Print(1, "mobility_area=\n%s", print_binary(mobility_area));

	move_count +=  __builtin_popcountll(f_mP1 | f_mP2 | f_mPE | f_mPW | f_mENP);

	//KNIGHT
	int in_N;
	U64 mN_q;

	piece_count = __builtin_popcountll(fr[4]);
	eval.material_f_mg += material_score_mg[4] * piece_count;
	eval.material_f_eg += material_score_eg[4] * piece_count;

	fr[4] &= ~ f_all_pp;
	while (__builtin_popcountll(fr[4]))
	{
		in_N = __builtin_ffsll(fr[4])-1;
		mN_q = movesNight[in_N] & ~fr[6] & check_grid;

		eval.mobility_f_mg += mobility_knight_bonus_mg[__builtin_popcountll(mN_q & f_mobility_area)];
		eval.mobility_f_eg += mobility_knight_bonus_eg[__builtin_popcountll(mN_q & f_mobility_area)];

		if (__builtin_popcountll(mN_q))
			move_count += __builtin_popcountll(mN_q);

		fr[4] &= ~(1LL << in_N);
	}

	piece_count = __builtin_popcountll(ho[4]);
	eval.material_h_mg += material_score_mg[4] * piece_count;
	eval.material_h_eg += material_score_eg[4] * piece_count;

	ho[4] &= ~ h_all_pp;
	while (__builtin_popcountll(ho[4]))
	{
		in_N = __builtin_ffsll(ho[4])-1;
		mN_q = movesNight[in_N] & ~ho[6] & ho_check_grid;

		eval.mobility_h_mg += mobility_knight_bonus_mg[__builtin_popcountll(mN_q & h_mobility_area)];
		eval.mobility_h_eg += mobility_knight_bonus_eg[__builtin_popcountll(mN_q & h_mobility_area)];

		//if (__builtin_popcountll(mN_q))
			//move_count += __builtin_popcountll(mN_q);

		ho[4] &= ~(1LL << in_N);
	}	

	int in_B, in;
	U64 mB_q;
	//BISHOP

	piece_count = __builtin_popcountll(fr[3]);
	eval.material_f_mg += material_score_mg[3] * piece_count;
	eval.material_f_eg += material_score_eg[3] * piece_count;

	fr[3] &= ~ f_all_pp;
	while (__builtin_popcountll(fr[3]))
	{
		in_B = __builtin_ffsll(fr[3])-1;
		in = ((arg.pieceset[16] & occupancyMaskBishop[in_B]) * magicNumberBishop[in_B]) >> magicNumberShiftsBishop[in_B];
		U64 mB_ray = magicMovesBishop[in_B][in] & check_grid;
		mB_q = mB_ray & ~ fr[6];

		eval.mobility_f_mg += mobility_bishop_bonus_mg[__builtin_popcountll(mB_ray & f_mobility_area)];
		eval.mobility_f_eg += mobility_bishop_bonus_eg[__builtin_popcountll(mB_ray & f_mobility_area)];

		if (__builtin_popcountll(mB_q))
			move_count += __builtin_popcountll(mB_q);

		fr[3] &= ~(1LL << in_B);
	}

	piece_count = __builtin_popcountll(ho[3]);
	eval.material_h_mg += material_score_mg[3] * piece_count;
	eval.material_h_eg += material_score_eg[3] * piece_count;

	ho[3] &= ~ h_all_pp;
	while (__builtin_popcountll(ho[3]))
	{
		in_B = __builtin_ffsll(ho[3])-1;
		in = ((arg.pieceset[16] & occupancyMaskBishop[in_B]) * magicNumberBishop[in_B]) >> magicNumberShiftsBishop[in_B];
		U64 mB_ray = magicMovesBishop[in_B][in] & ho_check_grid;
		mB_q = mB_ray & ~ ho[6];

		eval.mobility_h_mg += mobility_bishop_bonus_mg[__builtin_popcountll(mB_ray & h_mobility_area)];
		eval.mobility_h_eg += mobility_bishop_bonus_eg[__builtin_popcountll(mB_ray & h_mobility_area)];

		//if (__builtin_popcountll(mB_q))
			//move_count += __builtin_popcountll(mB_q);

		ho[3] &= ~(1LL << in_B);
	}

	int in_R;
	U64 mR_q;
	//ROOK

	piece_count = __builtin_popcountll(fr[2]);
	eval.material_f_mg += material_score_mg[2] * piece_count;
	eval.material_f_eg += material_score_eg[2] * piece_count;

	fr[2] &= ~ f_all_pp;
	while (__builtin_popcountll(fr[2]))
	{
		in_R = __builtin_ffsll(fr[2])-1;
		in = ((arg.pieceset[16] & occupancyMaskRook[in_R]) * magicNumberRook[in_R]) >> magicNumberShiftsRook[in_R];
		U64 mR_ray = magicMovesRook[in_R][in] & check_grid;
		mR_q = mR_ray & ~ fr[6];

		eval.mobility_f_mg += mobility_rook_bonus_mg[__builtin_popcountll(mR_ray & f_mobility_area)];
		eval.mobility_f_eg += mobility_rook_bonus_eg[__builtin_popcountll(mR_ray & f_mobility_area)];

		if (__builtin_popcountll(mR_q))
			move_count += __builtin_popcountll(mR_q);

		fr[2] &= ~1LL << in_R;
	}

	piece_count = __builtin_popcountll(ho[2]);
	eval.material_h_mg += material_score_mg[2] * piece_count;
	eval.material_h_eg += material_score_eg[2] * piece_count;

	ho[2] &= ~ h_all_pp;
	while (__builtin_popcountll(ho[2]))
	{
		in_R = __builtin_ffsll(ho[2])-1;
		in = ((arg.pieceset[16] & occupancyMaskRook[in_R]) * magicNumberRook[in_R]) >> magicNumberShiftsRook[in_R];
		U64 mR_ray = magicMovesRook[in_R][in] & ho_check_grid;
		mR_q = mR_ray & ~ ho[6];

		eval.mobility_h_mg += mobility_rook_bonus_mg[__builtin_popcountll(mR_ray & h_mobility_area)];
		eval.mobility_h_eg += mobility_rook_bonus_eg[__builtin_popcountll(mR_ray & h_mobility_area)];

		//if (__builtin_popcountll(mR_q))
			//move_count += __builtin_popcountll(mR_q);

		ho[2] &= ~1LL << in_R;
	}

	int in_Q;
	U64 mQ_q;
	//QUEEN

	piece_count = __builtin_popcountll(fr[1]);
	eval.material_f_mg += material_score_mg[1] * piece_count;
	eval.material_f_eg += material_score_eg[1] * piece_count;

	fr[1] &= ~ f_all_pp;
	while (__builtin_popcountll(fr[1]))
	{
		in_Q = __builtin_ffsll(fr[1])-1;
		in_R = ((arg.pieceset[16] & occupancyMaskRook[in_Q]) * magicNumberRook[in_Q]) >> magicNumberShiftsRook[in_Q];
		in_B = ((arg.pieceset[16] & occupancyMaskBishop[in_Q]) * magicNumberBishop[in_Q]) >> magicNumberShiftsBishop[in_Q];
		U64 mQ_ray = (magicMovesRook[in_Q][in_R] ^ magicMovesBishop[in_Q][in_B]) & check_grid;
		mQ_q = mQ_ray & ~ fr[6];

		eval.mobility_f_mg += mobility_queen_bonus_mg[__builtin_popcountll(mQ_ray & f_mobility_area)];
		eval.mobility_f_eg += mobility_queen_bonus_eg[__builtin_popcountll(mQ_ray & f_mobility_area)];

		if (__builtin_popcountll(mQ_q))
			move_count += __builtin_popcountll(mQ_q);

		fr[1] &= ~1LL << in_Q;
	}

	piece_count = __builtin_popcountll(ho[1]);
	eval.material_h_mg += material_score_mg[1] * piece_count;
	eval.material_h_eg += material_score_eg[1] * piece_count;

	ho[1] &= ~ h_all_pp;
	while (__builtin_popcountll(ho[1]))
	{
		in_Q = __builtin_ffsll(ho[1])-1;
		in_R = ((arg.pieceset[16] & occupancyMaskRook[in_Q]) * magicNumberRook[in_Q]) >> magicNumberShiftsRook[in_Q];
		in_B = ((arg.pieceset[16] & occupancyMaskBishop[in_Q]) * magicNumberBishop[in_Q]) >> magicNumberShiftsBishop[in_Q];
		U64 mQ_ray = (magicMovesRook[in_Q][in_R] ^ magicMovesBishop[in_Q][in_B]) & ho_check_grid;
		mQ_q = mQ_ray & ~ ho[6];

		eval.mobility_h_mg += mobility_queen_bonus_mg[__builtin_popcountll(mQ_ray & h_mobility_area)];
		eval.mobility_h_eg += mobility_queen_bonus_eg[__builtin_popcountll(mQ_ray & h_mobility_area)];

		//if (__builtin_popcountll(mQ_q))
			//move_count += __builtin_popcountll(mQ_q);

		ho[1] &= ~1LL << in_Q;
	}

	U64 mK_q;
	//fr_KING
	mK_q = movesKing[f_king] & ~fr[6] & ~ho[7];

	if (__builtin_popcountll(mK_q))
			move_count += __builtin_popcountll(mK_q);

	//ho_KING
	mK_q = movesKing[h_king] & ~ho[6] & ~fr[7];

	if (__builtin_popcountll(mK_q))
			//move_count += __builtin_popcountll(mK_q);

	if (!move_count)
	{
//stand pat
		return fr[0] & ho[7] ? - WIN + draft : DRAW_SCORE;
	}

// found_move
	// Print(1, "score:         %4d\n", eval.material_f_mg - eval.material_h_mg + eval.psqt_f_mg - eval.psqt_h_mg 
	// 											+ eval.mobility_f_mg - eval.mobility_h_mg + eval.pawns_f_mg - eval.pawns_h_mg);
	// Print(1, "material_mg: %4d %4d\n", eval.material_f_mg, eval.material_h_mg);
	// Print(1, "psqt_mg:     %4d %4d\n", eval.psqt_f_mg, eval.psqt_h_mg);
	// Print(1, "mobility_mg: %4d %4d\n", eval.mobility_f_mg, eval.mobility_h_mg);
	// Print(1, "pawns_mg:    %4d %4d\n", eval.pawns_f_mg, eval.pawns_h_mg);

	return eval.material_f_mg - eval.material_h_mg + eval.psqt_f_mg - eval.psqt_h_mg 
					+ eval.mobility_f_mg - eval.mobility_h_mg + eval.pawns_f_mg - eval.pawns_h_mg;
}

int neval( board *b)
{
	int w_score = 0, b_score = 0;

	w_score += 2538 * __builtin_popcountll( b->pieceset[1] );
	w_score += 1276 * __builtin_popcountll( b->pieceset[2] );
	w_score += 825 * __builtin_popcountll( b->pieceset[3] );
	w_score += 781 * __builtin_popcountll( b->pieceset[4] );
	w_score += 124 * __builtin_popcountll( b->pieceset[5] );

	b_score -= 2538 * __builtin_popcountll( b->pieceset[9] );
	b_score -= 1276 * __builtin_popcountll( b->pieceset[10] );
	b_score -= 825 * __builtin_popcountll( b->pieceset[11] );
	b_score -= 781 * __builtin_popcountll( b->pieceset[12] );
	b_score -= 124 * __builtin_popcountll( b->pieceset[13] );

	count++;
	return w_score + b_score;
}