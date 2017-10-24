#ifndef STREAMS_T
#define STREAMS_T

#include <stdio.h>

typedef struct streams_t {
  FILE **f;
  size_t l;
  size_t i;
} streams_t;

void openStreams(FILE **filePointers, int amount, char *mode, const char *filepathTemplate);
void openStream(FILE **filePointers, int index, char *mode, const char *filepathTemplate);
void closeStreams(FILE **filePointers, int amount);

void openStreams2(streams_t *streams, size_t length, char *mode, const char *filepathTemplate);
void openStream2(streams_t *streams, size_t index, char *mode, const char *filepathTemplate);
void closeStreams2(streams_t *streams);
void sread(void * ptr, size_t size, streams_t *streams);

#endif
