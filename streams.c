#include <math.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>

#include "streams.h"

// Opening all file streams in filePointers(columns of the reads) in mode *mode*
// (The streams will be fully buffered with a buffer of *setvBufferSize* size
// which the caller shall free after the streams have been closed)
// Default buffer size for each stream is MIN(file-system block size, BUFSIZ)
// where BUFSIZ and fs block size are variable on each system, on a "new"
// pc running linux with default settings BUFSIZ is 8092 and fs block size is 4096
// So, the default value should be 4096 can be forcefully changed with setvbuffer function
void openStreams(FILE **filePointers, int amount, char *mode, const char *filepathTemplate) {
	int sizeFilepath = (int)(ceil(log10(amount))+1);
	if(amount % 10 == 0 && amount > 10)
		sizeFilepath++;
	char *filepath = calloc(sizeFilepath + strlen(filepathTemplate) - 2, sizeof(char));

	/*
	int setvBufferSize = 512; // best value?
	char **setvBuffers = malloc((amount + 1) * sizeof(char *));
	for (int i = 0; i <= amount; ++i) {
		setvBuffers[i] = malloc(setvBufferSize * sizeof(char));
	}
	*/
	for (int i = 0; i <= amount; ++i) {
		sprintf(filepath, filepathTemplate, i);
		filePointers[i] = fopen(filepath, mode);
		//setvbuf (filePointers[i], setvBuffers[i], _IOFBF , setvBufferSize);
	}

	free(filepath);

	//return setvBuffers;
}

// open the filePointers[index] stream in mode *mode*
void openStream(FILE **filePointers, int index, char *mode, const char *filepathTemplate) {
	int sizeFilepath = 1;
	if(index)
		sizeFilepath = (int)(ceil(log10(index))+1);
	char *filepath = calloc(sizeFilepath + strlen(filepathTemplate) - 2 + 1, sizeof(char));
	sprintf(filepath, filepathTemplate, index);
	filePointers[index] = fopen(filepath, mode);
	free(filepath);
}

// Close all stream and associated buffers
void closeStreams(FILE **filePointers, int amount) {
	for (int i = 0; i <= amount; ++i)
		fclose(filePointers[i]);	
}

// Opening all file streams in filePointers(columns of the reads) in mode *mode*
// (The streams will be fully buffered with a buffer of *setvBufferSize* size
// which the caller shall free after the streams have been closed)
// Default buffer size for each stream is MIN(file-system block size, BUFSIZ)
// where BUFSIZ and fs block size are variable on each system, on a "new"
// pc running linux with default settings BUFSIZ is 8092 and fs block size is 4096
// So, the default value should be 4096 can be forcefully changed with setvbuffer function
void openStreams2(streams_t *streams, size_t length, char *mode, const char *filepathTemplate) {
	streams->f = malloc(length * sizeof(FILE *));
	streams->l = length;
	streams->i = 0;

	char *filepath;
	for (int i = 0; i < length ; ++i) {
		asprintf(&filepath, filepathTemplate, i);
		streams->f[i] = fopen(filepath, mode);
		free(filepath);
	}
}

// open the filePointers[index] stream in mode *mode*
void openStream2(streams_t *streams, size_t index, char *mode, const char *filepathTemplate) {
	streams->f = malloc(sizeof(FILE *));
	streams->l = 1;
	streams->i = 0;

	char *filepath;
	asprintf(&filepath, filepathTemplate, index);
	streams->f[0] = fopen(filepath, mode);
	free(filepath);
}

// Close all stream and associated buffers
void closeStreams2(streams_t *streams) {
	for (size_t i = 0; i < streams->l; ++i)
		fclose(streams->f[i]);
	free(streams->f);
	streams->f = NULL;
	streams->l = 0;
	streams->i = 0;
}

void truncateStreams2(streams_t *streams) {
	for (size_t i = 0; i < streams->l; ++i) {
		fflush(streams->f[i]);
		off_t pos = ftello(streams->f[i]);
		ftruncate(fileno(streams->f[i]), pos);
	}
}

void sread(void * ptr, size_t size, streams_t *streams) {
	if (streams->i >= streams->l) return;
	size_t n = fread_unlocked(ptr, size, 1, streams->f[streams->i]);
	if (n == 0) {
		++streams->i;
		sread(ptr, size, streams);
	}
}
