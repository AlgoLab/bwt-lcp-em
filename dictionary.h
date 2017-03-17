#ifndef DICTIONARY_H
#define DICTIONARY_H

#include <assert.h>
char getCode(char base);
char merge(char first, char second);
// todo: decoding


/*
A -> 0000
C -> 0001
G -> 0010
T -> 0011
$ -> 0100
# -> 0101
@ -> 0110 -> indicates no information (used when only the first half of the byte is used)
*/

#endif