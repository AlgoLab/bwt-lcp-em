#include "bwt_lcp.h"



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

	while((i = kseq_read(seq)) >= 0) {
		// Read 2 reads each time
		// (+1 for null terminator)
		readOne = malloc((i+1) * sizeof(char));
		strcpy(readOne, seq -> seq.s);
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
		else
			readTwo = seq -> seq.s;

		// Reads both read starting from the right end and writes the correctly encoded characters (merge)
		// in the right file. When one of the two reads is completely read 
		// (happens when they are shorter the the longest read)
		// start writing '#' instead, which indicates blank
		int dxAlignCounter = readMaxLength;
		while(dxAlignCounter > 0) {
			if(i > 0 && j > 0) {
				toWrite = merge(readOne[i-1], readTwo[j-1]);
				fputc(toWrite, filePointers[readMaxLength - dxAlignCounter]);
				j--;
				i--;
			}
			else if(i <= 0 && j > 0) {
				toWrite = merge('#', readTwo[j-1]);
				fputc(toWrite, filePointers[readMaxLength - dxAlignCounter]);
				j--;
			}
			else if(i > 0 && j <= 0) {
				toWrite = merge(readOne[i-1], '#');
				fputc(toWrite, filePointers[readMaxLength - dxAlignCounter]);
				i--;
			}
			else {
				toWrite = 0x55; // encodes -> ##
				fputc(toWrite, filePointers[readMaxLength - dxAlignCounter]);
			}

			dxAlignCounter--;			
		}
		linesCounter+=2;
		free(readOne);
		if(mustFreeReadTwo)
			free(readTwo);

	}

	// write the last file (i.e the last column) with all termination characters ($) (with their encoding)
	// useless since we know that this file is full of $, 
	//can be optimized by assuming that the nth character of file filePointers[readMaxLength] is always $
	int sizeSentinels = (linesCounter / 2) + 1;
	char *sentinels = malloc(sizeSentinels * sizeof(char));
	for (int i = 0; i < sizeSentinels; ++i) {
		if((i == sizeSentinels -1) && (linesCounter % 2 == 1))
			sentinels[i] = 0x06; // $@
		else
			sentinels[i] = 0x00; // $$
	}
	fwrite(sentinels, sizeof(char), sizeSentinels, filePointers[readMaxLength]);
	free(sentinels);

	
	// Close all streams
	closeStreams(filePointers, readMaxLength);

	FILE **partialBWT = computePartialBWT(filePointers, readMaxLength, linesCounter);

	computeBWTLCP(partialBWT, readMaxLength, linesCounter);


	/* Testing decode
	int readToTest = 2;
	char *test = decodeRead(filePointers, readMaxLength, readToTest);
	printf("read[%d]: %s\n", readToTest, test);
	free(test);
	*/

	/*
  	FILE *bwtEncoding = fopen("bwtEncoding", "wb");
  	int enc[12] = {0, 2, 3, 3, 1, 2, 2, 1, 1, 0, 0, 3};

  	fwrite(enc, sizeof(int), 12, bwtEncoding);
  	fclose(bwtEncoding);
  	bwtEncoding = fopen("bwtEncoding", "rb");
  	reconstructInterleave(bwtEncoding, 4, 12, filePointers);	
  	fclose(bwtEncoding);
  	*/

	/* Testing getEncodedColumn
	char *column = getEncodedColumn(filePointers, 136);
	FILE *prova = fopen("./tests/prova", "w");
	int ttemp = linesCounter/2;
	if(linesCounter % 2)
		ttemp++;
	fwrite(column, sizeof(char), ttemp, prova);
	fclose(prova);
	free(column);
	*/

	// free of all memory
	free(filePointers);
	kseq_destroy(seq);
  	gzclose(fp);


  	return 0;
}
