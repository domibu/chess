#include<stdarg.h>
#include<stdio.h>

#ifndef domibuChessInterface
#define domibuChessInterface

#include "data.h"
#include "data-1.h"

void objtoarr(board_1 *arg, board *to);
void arrtoobj(board *to, board_1 *arg);

char *printboard(board arg);
char *printboard_1(board_1 arg);
board_1 importFEN(char *fen);
board NimportFEN(char *fen);
void resetboard_1(board_1 *arg);
void resetboard(board *arg);

char *printBits(size_t const size, void const * const ptr);
char *piece(int index);
void square(int index, char *sq);
void square2(char *sq, int *index);

long int LargestPrime(long int n);
int InitializeLogId(char *log_path);
void Print(int vb, char *fmt, ...);

#endif
