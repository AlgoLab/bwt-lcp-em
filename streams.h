#ifndef STREAMS_T
#define STREAMS_T

#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>

void openStreams(FILE **filePointers, int amount, char *mode, const char *filepathTemplate);
void openStream(FILE **filePointers, int index, char *mode, const char *filepathTemplate);
void closeStreams(FILE **filePointers, int amount);

#endif