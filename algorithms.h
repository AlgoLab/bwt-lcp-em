#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdlib.h>
#include <zlib.h>
#include <stdio.h>
#include <limits.h>
#include "dictionary.h"
#include "streams.h"


FILE *reconstructInterleave(FILE *encodingArray, int n, int length, FILE **partialBWT);
FILE **computePartialBWT(FILE **filePointers, int readMaxLength, int totLines);
void createFirstSupportFile(int totLines, FILE *firstSupport, const char *filePath);
void copyFile(FILE *origin, FILE *destination);
void createStartingFiles(int readMaxLength, int totLines, FILE *firstLCP, FILE *firstBWT, FILE *firstSupportBWT, FILE *firstSupportLCP);
void computeBWTLCP(FILE **partialBWT, int readMaxLength, int totLines);
int min(int a, int b);



#endif