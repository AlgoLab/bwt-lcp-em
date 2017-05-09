#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdlib.h>
#include <zlib.h>
#include <stdio.h>
#include "dictionary.h"
#include "streams.h"


char *reconstructInterleave(int *encodingArray, int n, int length, FILE **filePointers);
void computePartialBWT(FILE **filePointers, int readMaxLength, int totLines);
void createFirstSupportFile(int totLines, FILE *firstSupport, const char *filePath);
void copyFile(FILE *origin, FILE *destination);



#endif