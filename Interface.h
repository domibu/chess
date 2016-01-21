#include<stdarg.h>
#include<stdio.h>

#ifndef domibuChessInterface
#define domibuChessInterface

#include "data.h"

char *printboard(board arg);
board NimportFEN(char *fen);
void resetboard(board *arg);

char *printBits(size_t const size, void const * const ptr);
char *piece(int index);
void square(int index, char *sq);
void square2(char *sq, int *index);

long int LargestPrime(long int n);
int InitializeLogId(char *log_path);
void Print(int vb, char *fmt, ...);

#endif
