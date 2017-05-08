#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <assert.h>
#include <stdbool.h>
#include <stdio.h>
#include <math.h>
#include <stdlib.h>
#include <string.h>
#include "streams.h"

char getCode(char base);
char merge(char first, char second);
char charToCode(char toDecode);
char getChar(char toDecode, bool odd);
char *decodeRead(FILE **filePointers, int readMaxLength, int toDecode);
char getCharFromColumn(int index, int column, FILE **filePointers);
char *getEncodedColumn(FILE **filePointers, int column);
char *getDecodedColumn(FILE **filePointers, int column);


#endif