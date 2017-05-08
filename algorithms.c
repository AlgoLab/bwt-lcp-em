#include "algorithms.h"

// Algorithm 1 (paper) 
char *reconstructInterleave(int *encodingArray, int n, int length, FILE **filePointers) {
	int *rank = (int *)calloc(n, sizeof(int));
	char *interleave = (char *)malloc(length+1 * sizeof(char));
	int i;
	for(int q = 0; q < length; q++) {
		i = encodingArray[q];
										// index   column
		interleave[q] = getCharFromColumn(rank[i], i, filePointers);
		rank[i] += 1;
	}
	interleave[length] = '\0';
	free(rank);
	return interleave;
}

// Algorithm 2(paper)
void partitionSuffixes(FILE **filePointers, int readMaxLength, int totLines) {
	FILE **outputFiles = (FILE **)malloc((readMaxLength+1) * sizeof(FILE *));
	FILE **supportFiles = (FILE **)malloc((readMaxLength+1) * sizeof(FILE *));
	FILE **supportLists = (FILE **)malloc(6 * sizeof(FILE *));

	char const *outputFilesTemplate = "./tests/outputFiles/B%d.txt";
	char const *supportFilesTemplate = "./tests/supportFiles/N%d.txt";
	char const *supportListsTemplate = "./tests/supportLists/L%d.txt";

	openStream(filePointers, 0, "rb", "./tests/arrays/T%d.txt");
	openStream(outputFiles, 0, "wb", outputFilesTemplate);
	copyFile(filePointers[0], outputFiles[0]);
	fclose(filePointers[0]);
	fclose(outputFiles[0]);

	createFirstSupportFile(totLines, supportFiles[0], "./tests/supportFiles/N0.txt");
	// end of "initialization" phase
	for (int l = 1; l <= readMaxLength; ++l) {
		openStream(outputFiles, l-1, "r", outputFilesTemplate);
		openStream(supportFiles, l-1, "rb", supportFilesTemplate);

		char **fileIOBuffers = openStreams(supportLists, 5, "w+b", supportListsTemplate);

		openStream(outputFiles, l, "w", outputFilesTemplate);
		openStream(supportFiles, l, "w+b", supportFilesTemplate);

		char c;
		int firstIndex;
		int secondIndex;
		unsigned char firstChar = 0;
		unsigned char secondChar = 0;
		int numBytes = totLines / 2;
		int odd = totLines % 2;
		if(odd)
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
		// works until here
		for (int i = 0; i < 6; ++i)
			fflush(supportLists[i]);

		for (int i = 0; i < 6; ++i)
			copyFile(supportLists[i], supportFiles[l]);

		fflush(supportFiles[l]);
		rewind(supportFiles[l]);

		
		openStream(filePointers, l, "rb", "./tests/arrays/T%d.txt");
		//using malloc here causes problem with valgrind
		//because he considers some values read by fread uninitialized
		char *fpl = (char *)calloc(numBytes, sizeof(char));
		fread(fpl, sizeof(char), numBytes, filePointers[l]);

		int index = 0;
		int adjIndex = 0;
		int oddIndex = 0;
		char toWrite = 0;
		//problema: chiocciola
		for (int i = 0; i < numBytes; ++i) {
			fread(&index, sizeof(int), 1, supportFiles[l]);
			adjIndex = index / 2;
			oddIndex = index % 2;
			if(oddIndex)
				toWrite = fpl[adjIndex] & 0x0f;
			else
				toWrite = fpl[adjIndex] >> 4;

			if(toWrite == 0x06)
				printf("chiocciola\n");
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

				if((toWrite & 0xf) == 0x06)
					printf("chiocciola\n");				
			}

			fputc(toWrite, outputFiles[l]);

		}
		free(fpl);
		closeStreams(supportLists, 5, fileIOBuffers);
		fclose(filePointers[l]);
		fclose(outputFiles[l-1]);
		fclose(supportFiles[l-1]);
		fclose(outputFiles[l]);
		fclose(supportFiles[l]);	
	
	}
	free(outputFiles);
	free(supportFiles);
	free(supportLists);

}

//used only once
// Creates a file containing 0 1 2 3 ... totlines-1; stored as int (4 bytes)
void createFirstSupportFile(int totLines, FILE *firstSupport, const char *filePath) {
	firstSupport = fopen(filePath, "wb");
	int bufferSize = 1024;

	int writeCycles = totLines / bufferSize;
	int lastBufferSize = totLines % bufferSize;


	int *buffer = (int *)malloc(bufferSize * sizeof(int));

	for(int i = 1; i <= writeCycles; i++) {
		for(int j = 0; j < bufferSize; j++) {
			buffer[j] = i * j;
		}
		fwrite(buffer, sizeof(int), bufferSize, firstSupport);
	}
	free(buffer);

	if(lastBufferSize) {
		int *lastBuffer = (int *)malloc(lastBufferSize * sizeof(int));
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

// Caller must open/close origin and destination streams
// Copies the whole content of file origin into file destination
void copyFile(FILE *origin, FILE *destination) {
	fseek(origin, 0, SEEK_END);
	int fileSize = ftell(origin);
	rewind(origin);

	if(fileSize == 0)
		return;

	int bufferSize = 512;
	int writeCycles = fileSize / bufferSize;
	int lastBufferSize = fileSize % bufferSize;

	char *buffer = (char *)malloc(bufferSize * sizeof(char));
	for(int i = 0; i < writeCycles; i++) {
		fread(buffer, sizeof(char), bufferSize, origin);
		fwrite(buffer, sizeof(char), bufferSize, destination);
	}
	free(buffer);

	if(lastBufferSize) {
		char *lastBuffer = (char *)malloc(lastBufferSize * sizeof(char));
		fread(lastBuffer, sizeof(char), lastBufferSize, origin);
		fwrite(lastBuffer, sizeof(char), lastBufferSize, destination);
		free(lastBuffer);
	}


}