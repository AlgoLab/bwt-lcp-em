#include <zlib.h>
#include <stdio.h>
#include <math.h>
#include <string.h>
#include "kseq.h"
#include "algorithms.h"
#include "dictionary.h"
#include "streams.h"



KSEQ_INIT(gzFile, gzread)

int main(int argc, char *argv[]) {
	// Initialization
	gzFile fp;
	kseq_t *seq;

	// Error if no file is given as input
	if (argc == 1) {
			fprintf(stderr, "Usage: %s <in.fasta>\n", argv[0]);
			return 1;
		}

	fp = gzopen(argv[1], "r");
	seq = kseq_init(fp);

	int readMaxLength = 0;
	int l;

	// Calculate the length of the longest read
	while((l = kseq_read(seq)) >= 0) {
		if(l > readMaxLength)
			readMaxLength = l;
	}

	if (readMaxLength > 253) {
		printf("Input reads are too long! Current = %d, max = 253\n", readMaxLength);
		kseq_destroy(seq);
		gzclose(fp);
		return 1;
	}

	printf("longest read length = %d\n", readMaxLength);

	// Array with pointers to all files
	FILE **filePointers = malloc((readMaxLength+1) * sizeof(FILE *));

	// Opening all streams in "write-only" mode
	openStreams(filePointers, readMaxLength, "w", "./tests/arrays/T%d");

	// Return the position indicator at the beginning of the file
	kseq_rewind(seq);
	gzrewind(fp);

	int linesCounter = 0;
	int i;
	int j;
	char *readOne;
	char *readTwo;
	char toWrite;
	int mustFreeReadTwo = 0;
	unsigned int *len_distr = calloc(readMaxLength + 1, sizeof(unsigned int));

	readOne = calloc(readMaxLength + 1, sizeof(char));
	while((i = kseq_read(seq)) >= 0) {
		// Read 2 reads each time
		// (+1 for null terminator)
		strcpy(readOne, seq -> seq.s);
		++len_distr[i];

		j = kseq_read(seq);
		// If the second read doesn't exist the file has an odd number of lines
		// which means that the last byte will store only half of valuable data (the upper half)
		// and the lower half will be the code of '@', the character that indicates no information
		if(j < 0) {
			readTwo = malloc(readMaxLength * sizeof(char));
			for (int k = 0; k < readMaxLength; ++k) {
				readTwo[k] = '@';
			}
			j = readMaxLength;
			linesCounter--;
			mustFreeReadTwo = 1;
		}
		else {
			readTwo = seq -> seq.s;
			++len_distr[j];
		}


		// Reads both read starting from the right end and writes the correctly encoded characters (merge)
		// in the right file. When one of the two reads is completely read 
		// (happens when they are shorter the the longest read)
		// start writing '#' instead, which indicates blank
		int dxAlignCounter = readMaxLength;
		while(dxAlignCounter >= 0) {
			if(i > 0 && j > 0) {
				toWrite = merge(readOne[i-1], readTwo[j-1]);
			}
			else if(i <= 0 && j > 0) {
				toWrite = merge(i == 0 ? '$': '#', readTwo[j-1]);
			}
			else if(i > 0 && j <= 0) {
				toWrite = merge(readOne[i-1], j == 0 ? '$': '#');
			}
			else {
				toWrite = merge(i == 0 ? '$': '#', mustFreeReadTwo ? '@' : (j == 0 ? '$': '#'));
			}
			fputc(toWrite, filePointers[readMaxLength - dxAlignCounter]);
			j--;
			i--;
			dxAlignCounter--;
		}
		linesCounter+=2;
	}
	free(readOne);
	if(mustFreeReadTwo)
		free(readTwo);
	printf("number of reads = %d\n", linesCounter);

	// Close all streams
	closeStreams(filePointers, readMaxLength);

	// free of all memory
	free(filePointers);
	kseq_destroy(seq);
	gzclose(fp);

	printf("Computing partial BWT\n");
	computePartialBWT(readMaxLength, linesCounter);

	printf("Computing BWT and LCP\n");
	computeBWTLCP(readMaxLength, linesCounter, len_distr);


	free(len_distr);

	return 0;
}
