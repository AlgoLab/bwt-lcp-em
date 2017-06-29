#include "algorithms.h"

// Algorithm 1 (paper) 
FILE *reconstructInterleave(FILE *encodingArray, int readMaxLength, int encodingLength, FILE **partialBWT) {
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
	return bwt;
}

// Algorithm 2(paper)
FILE **computePartialBWT(FILE **filePointers, int readMaxLength, int totLines) {
	FILE **outputFiles = malloc((readMaxLength+1) * sizeof(FILE *));
	FILE **supportFiles = malloc((readMaxLength+1) * sizeof(FILE *));
	FILE **supportLists = malloc(6 * sizeof(FILE *));

	char const *outputFilesTemplate = "./tests/outputFiles/B%d";
	char const *supportFilesTemplate = "./tests/supportFiles/N%d";
	char const *supportListsTemplate = "./tests/supportLists/P%d";

	//Calculates B0 (copies T0)
	openStream(filePointers, 0, "rb", "./tests/arrays/T%d");
	openStream(outputFiles, 0, "wb", outputFilesTemplate);
	copyFile(filePointers[0], outputFiles[0]);
	fclose(filePointers[0]);
	fclose(outputFiles[0]);

	//Calculates N0 -> sequence of natural numbers ending with m (= totLines)
	createFirstSupportFile(totLines, supportFiles[0], "./tests/supportFiles/N0");
	
	//Iteratively constructs Nl from Nl-1 and Bl-1, then Bl from Nl
	for (int l = 1; l <= readMaxLength; ++l) {
		openStream(outputFiles, l-1, "r", outputFilesTemplate);
		openStream(supportFiles, l-1, "rb", supportFilesTemplate);

		
		openStreams(supportLists, 5, "w+b", supportListsTemplate);

		openStream(outputFiles, l, "w", outputFilesTemplate);
		openStream(supportFiles, l, "w+b", supportFilesTemplate);

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

			
			fread(&firstIndex, sizeof(int), 1, supportFiles[l-1]);
			fwrite(&firstIndex, sizeof(int), 1, supportLists[firstChar]);

			if(secondChar != 0x06) {
				fread(&secondIndex, sizeof(int), 1, supportFiles[l-1]);
				fwrite(&secondIndex, sizeof(int), 1, supportLists[secondChar]);
			}

		}

		for (int i = 0; i < 6; ++i) {
			fflush(supportLists[i]);
			rewind(supportLists[i]);
		}

		//Can be optimized by not copying the files and when reading Nl sequentially(next "for" cycle)
		//A pointer points to this files and at the end of one just jumps to the next
		for (int i = 0; i < 6; ++i)
			copyFile(supportLists[i], supportFiles[l]);

		fflush(supportFiles[l]);
		rewind(supportFiles[l]);

		
		openStream(filePointers, l, "rb", "./tests/arrays/T%d");
		//using malloc here causes problem with valgrind
		//because he considers some values read by fread uninitialized
		char *fpl = calloc(numBytes, sizeof(char));
		fread(fpl, sizeof(char), numBytes, filePointers[l]);

		int index = 0;
		int adjIndex = 0;
		int oddIndex = 0;
		char toWrite = 0;
		for (int i = 0; i < numBytes; ++i) {
			fread(&index, sizeof(int), 1, supportFiles[l]);
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
				fread(&index, sizeof(int), 1, supportFiles[l]);
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
		closeStreams(supportLists, 5);
		fclose(filePointers[l]);
		fclose(outputFiles[l-1]);
		fclose(supportFiles[l-1]);
		fclose(outputFiles[l]);
		fclose(supportFiles[l]);	
	
	}
	free(supportFiles);
	free(supportLists);
	return(outputFiles);

}


//Creates a file containing the sequence of natural numbers ending with "totLines"
void createFirstSupportFile(int totLines, FILE *firstSupport, const char *filePath) {
	firstSupport = fopen(filePath, "wb");
	int bufferSize = 1024; // best value?

	int writeCycles = totLines / bufferSize;
	int lastBufferSize = totLines % bufferSize;


	int *buffer = malloc(bufferSize * sizeof(int));

	for(int i = 1; i <= writeCycles; i++) {
		for(int j = 0; j < bufferSize; j++) {
			buffer[j] = i * j;
		}
		fwrite(buffer, sizeof(int), bufferSize, firstSupport);
	}
	free(buffer);

	if(lastBufferSize) {
		int *lastBuffer = malloc(lastBufferSize * sizeof(int));
		int lastCycleValueStart = writeCycles * bufferSize;

		for (int i = 0; i < lastBufferSize; ++i) {
			lastBuffer[i] = lastCycleValueStart;
			lastCycleValueStart++;
		}
		fwrite(lastBuffer, sizeof(int), lastBufferSize, firstSupport);
		free(lastBuffer);
	}
	fclose(firstSupport);
}


// Copies the whole content of file origin into file destination
// Caller must open/close origin and destination streams
void copyFile(FILE *origin, FILE *destination) {
	//Calculate file size (Bytes)
	fseek(origin, 0, SEEK_END);
	int fileSize = ftell(origin);
	rewind(origin);

	if(!fileSize)
		return;

	int bufferSize = 1024; // best value?
	int writeCycles = fileSize / bufferSize;
	int lastBufferSize = fileSize % bufferSize;

	char *buffer = calloc(bufferSize, sizeof(char));
	for(int i = 0; i < writeCycles; i++) {
		fread(buffer, sizeof(char), bufferSize, origin);
		fwrite(buffer, sizeof(char), bufferSize, destination);
	}
	free(buffer);

	if(lastBufferSize) {
		char *lastBuffer = malloc(lastBufferSize * sizeof(char));
		fread(lastBuffer, sizeof(char), lastBufferSize, origin);
		fwrite(lastBuffer, sizeof(char), lastBufferSize, destination);
		free(lastBuffer);
	}


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

int min(int a, int b) {
	if(a <= b)
		return a;

	return b;
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
		//printf("%d\n", p);
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
				alfa[i] = min(alfa[i], lcpValue);
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