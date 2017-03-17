#include "bwt_lcp.h"
#include "dictionary.h"

KSEQ_INIT(gzFile, gzread)

int main(int argc, char *argv[]) {
	// Initialization
	gzFile fp;
	kseq_t *seq;
	int readMaxLength;
	int l;
	// Error if no file is given as input
	if (argc == 1) {
    	fprintf(stderr, "Usage: %s <in.fasta>\n", argv[0]);
    	return 1;
  	}

	fp = gzopen(argv[1], "r");
	seq = kseq_init(fp);

	// Calculate the length of the longest read (used mostly to know how many stream need to be opened)
	while((l = kseq_read(seq)) >= 0){
		if(l > readMaxLength)
			readMaxLength = l;
	}

	// Variables for file names
	const char *filepathTemplate = "./tests/arrays/T%d.txt";
	int sizeFilepath = (int)(ceil(log10(readMaxLength))+1);
	char *filepath = (char *)calloc(sizeFilepath + strlen(filepathTemplate) - 2, sizeof(char));

	// Array with pointers to all files
	FILE **filePointers = (FILE **)malloc((readMaxLength+1) * sizeof(FILE *));

	// Opening all streams in "write-only" mode
	for (int i = 0; i <= readMaxLength; ++i) {
		sprintf(filepath, filepathTemplate, i);
		filePointers[i] = fopen(filepath, "w");
		//setbuf(filePointers[i], NULL); --> la libreria standard utilizza già buffer efficienti
	}

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
		readOne = (char *)malloc(i * sizeof(char));
		strcpy(readOne, seq -> seq.s);
		j = kseq_read(seq);
		// If the second read doesn't exist the file has an odd number of lines
		// which means that the last byte will store only half of valuable data (the upper half)
		// and the lower half will be the code of '@', the character that indicates no information
		if(j < 0) { 
			readTwo = (char *)malloc(readMaxLength * sizeof(char));
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
		while(dxAlignCounter > 0){
			if(i > 0 && j > 0){
				toWrite = merge(readOne[i-1], readTwo[j-1]);
				fputc(toWrite, filePointers[dxAlignCounter-1]);
				j--;
				i--;
			}
			else if(i < 0 && j > 0) {
				toWrite = merge('#', readTwo[j-1]);
				fputc(toWrite, filePointers[dxAlignCounter-1]);
				j--;
			}
			else if(i > 0 && j < 0) {
				toWrite = merge(readOne[i], '#');
				fputc(toWrite, filePointers[dxAlignCounter-1]);
				i--;
			}
			else {
				toWrite = 0x55; // toWrite encodes -> ##
				fputc(toWrite, filePointers[dxAlignCounter-1]);
			}

			dxAlignCounter--;			
		}
		linesCounter+=2;

	}
	printf("%d\n", linesCounter);

	// write the last file (i.e the last column) with all termination characters ($) (with their encoding)
	int sizeSentinels = (linesCounter / 2) + 1;
	printf("%d\n", linesCounter % 2);
	char *sentinels = (char *)malloc(sizeSentinels * sizeof(char));
	for (int i = 0; i < sizeSentinels; ++i) {
		if((i == sizeSentinels -1) && (linesCounter % 2 == 1))
			sentinels[i] = 0x46; // $@
		else
			sentinels[i] = 0x44; // $$
	}
	fputs(sentinels, filePointers[readMaxLength]);

	
	// Close all streams and free of all memory
	for (int i = 0; i <= readMaxLength; ++i) {
		fclose(filePointers[i]); 	
	}
	free(readOne);
	if(mustFreeReadTwo)
		free(readTwo);
	free(sentinels);
	free(filePointers);
	free(filepath);
	kseq_destroy(seq);
  	gzclose(fp);

  	/*
		ToDo:  Decoding

  	*/

  	return 0;
}