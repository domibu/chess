#ifndef domibuBitBoardMagic
#define domibuBitBoardMagic

typedef unsigned long long U64;

U64 magicNumberRook[64], magicNumberBishop[64];
U64 occupancyMaskRook[64], occupancyMaskBishop[64];
int magicNumberShiftsRook[64], magicNumberShiftsBishop[64];

U64 occupancyVariationRook[64][4096], occupancyVariationBishop[64][512];
U64 magicMovesRook[64][4096], magicMovesBishop[64][512];
U64 movesNight[64];
U64 movesKing[64];

void InitializeMoveDatabase();
void getsetBits(U64 a, int *p);
void generateOccupancyVariations(short isRook);
void generateMoveDatabase(short isRook);
void generatemovesNight();
void generatemovesKing();

#endif

