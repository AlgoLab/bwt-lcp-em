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
