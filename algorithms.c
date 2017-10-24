#include "algorithms.h"

// Algorithm 1 (paper)
void reconstructInterleave(FILE *encodingArray, int readMaxLength, int encodingLength, FILE **partialBWT) {
	FILE *bwt = fopen("./tests/B_W_T", "w");
	int i;
	openStreams(partialBWT, readMaxLength, "r", "./tests/outputFiles/B%d");
	char *supportBuffer = malloc((readMaxLength + 1) * sizeof(char));
	char toWrite = -1;
	for (int i = 0; i <= readMaxLength; ++i)
		supportBuffer[i] = -1; // ff
	char c;
	for(int q = 0; q < encodingLength; q++) {
		fread(&i, sizeof(int), 1, encodingArray);
		if(supportBuffer[i] == -1) {
			c = fgetc(partialBWT[i]);
			supportBuffer[i] = c & 0x0f;
			c = c >> 4;
		}
		else {
			c = supportBuffer[i];
			supportBuffer[i] = -1;
		}

		if(c == 0x05) //eliminates # in the bwt
			continue;

		//printf("%c ", charToCode(c));

		if(toWrite == -1){
			toWrite = c << 4;
			if(q == encodingLength-1){
				toWrite |= 0x06;
				fputc(toWrite, bwt);
			}

		}
		else {
			toWrite |= c;
			fputc(toWrite, bwt);
			toWrite = -1;
		}



	}
	closeStreams(partialBWT, readMaxLength);
	free(supportBuffer);
	fclose(bwt);
}

#define outputFilesTemplate "./tests/outputFiles/B%d"
#define supportListsTemplate "./tests/supportLists/P%d"
#define T_TPL "./tests/arrays/T%d"
#define B_TPL outputFilesTemplate
#define P_TPL(l) ((l) % 2 == 0) ? "./tests/supportLists/Peven_%d" : "./tests/supportLists/Podd_%d"

//Creates a file containing the sequence of natural numbers ending with "totLines"
void createFirstSupportFile(int totLines, FILE *firstSupport) {
	size_t bufferSize = 1024; // best value?

	int *buffer = malloc(bufferSize * sizeof(int));

  size_t i = 0;
  while (i < totLines) {
    size_t j = 0;
		while (j < bufferSize && i < totLines) {
			buffer[j] = i;
      ++i;
      ++j;
		}
		fwrite(buffer, sizeof(int), j, firstSupport);
  }
	free(buffer);
}

// Algorithm 2(paper)
FILE **computePartialBWT(FILE **filePointers, int readMaxLength, int totLines) {
	FILE **outputFiles = malloc((readMaxLength+1) * sizeof(FILE *));
	streams_t supportLists;
	streams_t supportLists_prev;


	//Calculates B0 (copies T0)
	openStream(filePointers, 0, "rb", T_TPL);
	openStream(outputFiles, 0, "wb", B_TPL);
	copyFile(filePointers[0], outputFiles[0]);
	fclose(filePointers[0]);
	fclose(outputFiles[0]);

	//Calculates N0 -> sequence of natural numbers ending with m (= totLines)
  openStreams2(&supportLists_prev, 6, "w", P_TPL(0));
	createFirstSupportFile(totLines, supportLists_prev.f[0]);
  closeStreams2(&supportLists_prev);

	//Iteratively constructs Nl from Nl-1 and Bl-1, then Bl from Nl
	for (int l = 1; l <= readMaxLength; ++l) {
		openStream(outputFiles, l-1, "r", B_TPL);
		openStreams2(&supportLists_prev, 6, "r", P_TPL(l-1));
		openStreams2(&supportLists, 6, "w", P_TPL(l));

		openStream(outputFiles, l, "w", B_TPL);

		char c;
		int firstIndex;
		int secondIndex;
		unsigned char firstChar = 0;
		unsigned char secondChar = 0;
		int numBytes = totLines / 2;
		int odd = totLines % 2;
		if(odd) // every outpufile will contain in the last 4 bits of the last byte "0x06", code for "@"
			numBytes++;
		// now numBytes should be the size of outputfiles[l-1]
		for (int i = 0; i < numBytes; ++i) {
			c = fgetc(outputFiles[l-1]);
			secondChar = c & 0x0f;
			firstChar = c >> 4;


			sread(&firstIndex, sizeof(int), &supportLists_prev);
			fwrite(&firstIndex, sizeof(int), 1, supportLists.f[firstChar]);

			if(secondChar != 0x06) {
				sread(&secondIndex, sizeof(int), &supportLists_prev);
				fwrite(&secondIndex, sizeof(int), 1, supportLists.f[secondChar]);
			}

		}

    closeStreams2(&supportLists_prev);
    closeStreams2(&supportLists);
    openStreams2(&supportLists, 6, "r", P_TPL(l));

		openStream(filePointers, l, "rb", T_TPL);
		//using malloc here causes problem with valgrind
		//because he considers some values read by fread uninitialized
		char *fpl = malloc(numBytes * sizeof(char));
		fread(fpl, sizeof(char), numBytes, filePointers[l]);

		int index = 0;
		int adjIndex = 0;
		int oddIndex = 0;
		char toWrite = 0;
		for (int i = 0; i < numBytes; ++i) {
			sread(&index, sizeof(int), &supportLists);
			adjIndex = index / 2;
			oddIndex = index % 2;
			if(oddIndex)
				toWrite = fpl[adjIndex] & 0x0f;
			else
				toWrite = fpl[adjIndex] >> 4;

			toWrite = toWrite << 4;

			if(i == numBytes-1 && odd) {
				toWrite |= 0x06; // @
			}
			else {
				sread(&index, sizeof(int), &supportLists);
				adjIndex = index / 2;
				oddIndex = index % 2;

				if(oddIndex)
					toWrite |= fpl[adjIndex] & 0x0f;
				else
					toWrite |= fpl[adjIndex] >> 4;

			}

			fputc(toWrite, outputFiles[l]);

		}
		free(fpl);
		fclose(filePointers[l]);
		fclose(outputFiles[l-1]);
		fclose(outputFiles[l]);
		closeStreams2(&supportLists);

	}
	return(outputFiles);

}


// Copies the whole content of file origin into file destination
// Caller must open/close origin and destination streams
void copyFile(FILE *origin, FILE *destination) {
	size_t bufferSize = BUFSIZ;
	char * buffer = malloc(bufferSize);
  size_t len = 0;
  while ((len = fread(buffer, 1, bufferSize, origin)) > 0) {
		fwrite(buffer, 1, len, destination);
  }
	free(buffer);
}

void createStartingFiles(int readMaxLength, int totLines, FILE *firstLCP, FILE *firstBWT, FILE *firstSupportBWT, FILE *firstSupportLCP) {
	int *buffer = malloc(totLines * sizeof(int));
	for (int i = 0; i <= readMaxLength; ++i) {
		for (int j = 0; j < totLines; ++j) {
			buffer[j] = i;
		}
		fwrite(buffer, sizeof(int), totLines, firstBWT);
	}

	memset(buffer, 0, totLines * 4);
	fwrite(buffer, sizeof(int), totLines, firstSupportBWT);
	buffer[0] = -1;

	fwrite(buffer, sizeof(int), totLines, firstSupportLCP);

	fwrite(buffer, sizeof(int), totLines, firstLCP);

	buffer[0] = 0;
	for (int i = 0; i < readMaxLength; ++i)
		fwrite(buffer, sizeof(int), totLines, firstLCP);

	free(buffer);

}

void computeBWTLCP(FILE **partialBWT, int readMaxLength, int totLines) {

	FILE **supportLCP = malloc(6 * sizeof(FILE *));
	FILE **supportBWT = malloc(6 * sizeof(FILE *));
	FILE *lcp;
	FILE *bwt;

	const char *supportLCPTemplate = "./tests/supportLCP/L%d";
	const char *supportBWTTemplate = "./tests/supportBWT/I%d";
	const char *lcpTemplate = "./tests/LCP/LCP";
	const char *bwtTemplate = "./tests/BWT/BWT";

	lcp = fopen(lcpTemplate, "wb");
	bwt = fopen(bwtTemplate, "wb");
	openStream(supportBWT, 0, "wb", supportBWTTemplate);
	openStream(supportLCP, 0, "wb", supportLCPTemplate);
	createStartingFiles(readMaxLength, totLines, lcp, bwt, supportBWT[0], supportLCP[0]);
	fclose(lcp);
	fclose(bwt);
	fclose(supportBWT[0]);
	fclose(supportLCP[0]);
	//not closing supportBWT[0] and supportLCP[0], will be done together with others supportBWT elements

	int maxLCP = 0;
	int p = 0;
	while(maxLCP == p) {
		printf("Starting iteration n° %d\n", p);
		int s = 0;
		for (int i = 1; i < 6; ++i) {
			openStream(supportLCP, i, "wb", supportLCPTemplate);
			if(i != 5)
				fwrite(&s, sizeof(int), 1, supportLCP[i]);
		}
		// should be of size 5 but because of how the alfabet is encoded
		// it's useful to use the 5 index from 1 to 5 and ignore the first one (0);
		int alfa[6] = {-1, -1, -1, -1, -1, -1};


		int lcpValue;
		int l;
		char c;
		char decoded;
		char *supportBuffer = malloc((readMaxLength+1) * sizeof(char));
		for (int i = 0; i <= readMaxLength; ++i)
			supportBuffer[i] = -1; // ff

		openStreams(partialBWT, readMaxLength, "r", "./tests/outputFiles/B%d");
		for (int i = 1; i < 6; ++i)
			openStream(supportBWT, i, "wb", supportBWTTemplate);
		bwt = fopen(bwtTemplate, "rb");
		lcp = fopen(lcpTemplate, "rb");
		for (int i = 0; i < (readMaxLength + 1) * totLines; ++i) {
			fread(&l, sizeof(int), 1, bwt);
			if(supportBuffer[l] == -1) {
				c = fgetc(partialBWT[l]);
				supportBuffer[l] = c & 0x0f;
				c = c >> 4;
			}
			else {
				c = supportBuffer[l];
				supportBuffer[l] = -1;
			}

			//c = MASK[c];

			decoded = charToCode(c);

			if(decoded != '$') {
				l++;
				fwrite(&l, sizeof(int), 1, supportBWT[c]);
			}

			fread(&lcpValue, sizeof(int), 1, lcp);
			for (int i = 1; i < 6; ++i) {
				alfa[i] = alfa[i] <= lcpValue ? alfa[i] : lcpValue;
			}

			if(decoded != '$' && decoded != '#' && alfa[c] >= 0) {
				int a = alfa[c] + 1;
				if(a > maxLCP)
					maxLCP = a;
				fwrite(&a, sizeof(int), 1, supportLCP[c]);
			}

			alfa[c] = INT_MAX;



		}
		for (int i = 1; i < 6; ++i)
			fclose(supportBWT[i]);
		for (int i = 1; i < 6; ++i)
			fclose(supportLCP[i]);
		fclose(bwt);
		fclose(lcp);
		closeStreams(partialBWT, readMaxLength);



		bwt = fopen(bwtTemplate, "wb");
		lcp = fopen(lcpTemplate, "wb");
		openStreams(supportBWT, 5, "rb", supportBWTTemplate);
		openStreams(supportLCP, 5, "rb", supportLCPTemplate);
		for (int i = 0; i < 6; ++i) {
			copyFile(supportBWT[i], bwt);
			copyFile(supportLCP[i], lcp);
		}
		fclose(lcp);
		fclose(bwt);
		closeStreams(supportLCP, 5);
		closeStreams(supportBWT, 5);
		free(supportBuffer);
		p++;
	}
	bwt = fopen(bwtTemplate, "rb");
	reconstructInterleave(bwt, readMaxLength, (readMaxLength + 1) * totLines, partialBWT);
	fclose(bwt);
	free(supportBWT);
	free(supportLCP);
	free(partialBWT);
}


//algorithm 4 is working, need to handle the # in LCP
