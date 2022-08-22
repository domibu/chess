#ifndef domibuEvaluation
#define domibuEvaluation

#include "data.h"

#ifndef EVAL_SCORE
#define EVAL_SCORE

typedef struct eval_score {
	int material_f_mg;
    int material_h_mg;
    int material_f_eg;
    int material_h_eg;
    int psqt_f_mg;
    int psqt_h_mg;
    int psqt_f_eg;
    int psqt_h_eg;
    int pawns_f_mg;
    int pawns_h_mg;
    int pawns_f_eg;
    int pawns_h_eg;
    int mobility_f_mg;
    int mobility_h_mg;
    int mobility_f_eg;
    int mobility_h_eg;
} eval_score;

#endif

int evaluate( board arg, int draft, int color, board *rb);
int neval( board *b);
void init_psqt();

#endif