#include "algorithms.h"
#include "common_types.h"

#include <sys/mman.h>

// Copies the whole content of file origin into file destination
// Caller must open/close origin and destination streams
void copyFile(FILE *origin, FILE *destination) {
	size_t bufferSize = BUFSIZ;
	char * buffer = malloc(bufferSize);
	size_t len = 0;
	while ((len = fread_unlocked(buffer, 1, bufferSize, origin)) > 0) {
		fwrite(buffer, 1, len, destination);
	}
	free(buffer);
}

// Copies the whole content of file origin into file destination
// Caller must open/close origin and destination streams
void copyLCPFile(FILE *origin, FILE *destination) {
	if (LCP_OUT_EL_SIZE == LCP_EL_SIZE) copyFile(origin, destination);
	size_t bufferSize = BUFSIZ;
	lcp_element_t *in_buffer = malloc(bufferSize * LCP_EL_SIZE);
	lcp_out_element_t *out_buffer = malloc(bufferSize * LCP_OUT_EL_SIZE);
	size_t len = 0;
	while ((len = fread(in_buffer, LCP_EL_SIZE, bufferSize, origin)) > 0) {
		for (size_t i = 0; i < len; ++i) {
			out_buffer[i] = in_buffer[i];
			if (in_buffer[i] == lcp_minus1) out_buffer[i] = -1;
		}
		fwrite(out_buffer, LCP_OUT_EL_SIZE, len, destination);
	}
	free(in_buffer);
	free(out_buffer);
}

// Algorithm 1 (paper)
void reconstructInterleave(streams_t *Iprev, int readMaxLength, int encodingLength) {
	FILE *bwt = fopen(BWTbin_final, "w");
	int i;
	streams_t Bpart;
	openStreams2(&Bpart, readMaxLength+1, "r", B_TPL);
	char *supportBuffer = malloc((readMaxLength + 1) * sizeof(char));
	char toWrite = -1;
	for (int i = 0; i <= readMaxLength; ++i)
		supportBuffer[i] = -1; // ff
	unsigned char c;
	for(int q = 0; q < encodingLength; q++) {
		sread(&i, sizeof(int), Iprev);
		if(supportBuffer[i] == -1) {
			c = getc_unlocked(Bpart.f[i]);
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
	closeStreams2(&Bpart);
	free(supportBuffer);
	fclose(bwt);
}

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
void computePartialBWT(int readMaxLength, int totLines) {
	FILE **outputFiles = malloc((readMaxLength+1) * sizeof(FILE *));
	streams_t supportLists;
	streams_t supportLists_prev;
	streams_t T;

	const size_t numBytes = (totLines / 2) + (totLines % 2);
	const bool odd = (totLines % 2) == 1;
	// numBytes should be the size of outputfiles[l]

	//Calculates B0 (copies T0)
	openStream2(&T, 0, "rb", T_TPL);
	openStream(outputFiles, 0, "wb", B_TPL);
	copyFile(T.f[0], outputFiles[0]);
	closeStreams2(&T);
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

		unsigned char c;
		int firstIndex;
		int secondIndex;
		unsigned char firstChar = 0;
		unsigned char secondChar = 0;
		for (size_t i = 0; i < numBytes; ++i) {
			c = getc(outputFiles[l-1]);
			secondChar = c & 0x0f;
			firstChar = c >> 4;


			sread(&firstIndex, sizeof(int), &supportLists_prev);
			fwrite_unlocked(&firstIndex, sizeof(int), 1, supportLists.f[firstChar]);

			if(secondChar != 0x06) {
				sread(&secondIndex, sizeof(int), &supportLists_prev);
				fwrite_unlocked(&secondIndex, sizeof(int), 1, supportLists.f[secondChar]);
			}

		}
		fclose(outputFiles[l-1]);

		closeStreams2(&supportLists_prev);
		closeStreams2(&supportLists);
		openStreams2(&supportLists, 6, "r", P_TPL(l));

		openStream2(&T, l, "rb", T_TPL);
		char *fpl = mmap(NULL, numBytes, PROT_READ, MAP_PRIVATE, fileno(T.f[0]), 0);

		int index = 0;
		int adjIndex = 0;
		int oddIndex = 0;
		unsigned char toWrite = 0;
		for (size_t i = 0; i < numBytes; ++i) {
			sread(&index, sizeof(int), &supportLists);
			adjIndex = index / 2;
			oddIndex = index % 2;
			if(oddIndex)
				toWrite = (fpl[adjIndex] & 0x0f) << 4;
			else
				toWrite = (fpl[adjIndex] & 0xf0);

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
		munmap(fpl, numBytes);
		closeStreams2(&T);
		fclose(outputFiles[l]);
		closeStreams2(&supportLists);
	}
	free(outputFiles);
}

void createStartingFiles(size_t readMaxLength, size_t totLines) {
	idx_element_t *idx_buffer = malloc(totLines * IDX_EL_SIZE);

	streams_t Iprev;
	openStreams2(&Iprev, 6, "w", Ipart_TPL(0));
	for (size_t i = 0; i <= readMaxLength; ++i) {
		for (size_t j = 0; j < totLines; ++j) {
			idx_buffer[j] = i;
		}
		fwrite_unlocked(idx_buffer, IDX_EL_SIZE, totLines, Iprev.f[(i == 0) ? 0 : 1]);
	}
	closeStreams2(&Iprev);

	memset(idx_buffer, 0, totLines * IDX_EL_SIZE);

	streams_t Icur;
	openStreams2(&Icur, 6, "w", Ipart_TPL(1));
	fwrite_unlocked(idx_buffer, IDX_EL_SIZE, totLines, Icur.f[0]);
	closeStreams2(&Icur);

	free(idx_buffer);

	lcp_element_t *lcp_buffer = malloc(totLines * LCP_EL_SIZE);
	memset(lcp_buffer, 0, totLines * LCP_EL_SIZE);

	streams_t Lcur;
	lcp_buffer[0] = lcp_minus1;
	openStreams2(&Lcur, 6, "w", Lpart_TPL(1));
	fwrite_unlocked(lcp_buffer, LCP_EL_SIZE, totLines, Lcur.f[0]);
	closeStreams2(&Lcur);

	streams_t Lprev;
	openStreams2(&Lprev, 6, "w", Lpart_TPL(0));
	fwrite_unlocked(lcp_buffer, LCP_EL_SIZE, totLines, Lprev.f[0]);
	lcp_buffer[0] = 0;
	for (size_t i = 1; i <= readMaxLength; ++i) {
		fwrite_unlocked(lcp_buffer, LCP_EL_SIZE, totLines, Lprev.f[1]);
	}
	closeStreams2(&Lprev);

	free(lcp_buffer);
}

void computeBWTLCP(size_t readMaxLength, size_t totLines) {

	const size_t num_bytes = (readMaxLength + 1) * totLines;
	createStartingFiles(readMaxLength, totLines);

	streams_t Iprev, Icur, Lprev, Lcur, Bpart;

	lcp_element_t maxLCP = 0;
	lcp_element_t p = 0;
	char *supportBuffer = malloc((readMaxLength+1) * sizeof(char));
	while (maxLCP == p) {
		printf("Starting iteration n° %d\n", p);
		fflush(stdout);

		openStreams2(&Iprev, 6, "r", Ipart_TPL(p));
		openStreams2(&Lprev, 6, "r", Lpart_TPL(p));
		// r+ so that we can write but the file is not truncated
		openStreams2(&Icur, 6, "r+", Ipart_TPL(p+1));
		openStreams2(&Lcur, 6, "r+", Lpart_TPL(p+1));

		lcp_element_t s = 0;
		for (int i = 1; i < 5; ++i) {
			fwrite_unlocked(&s, LCP_EL_SIZE, 1, Lcur.f[i]);
		}
		// should be of size 5 but because of how the alfabet is encoded
		// it's useful to use the 5 index from 1 to 5 and ignore the first one (0);
		lcp_element_t alfa[6] = {lcp_minus1, lcp_minus1, lcp_minus1, lcp_minus1, lcp_minus1, lcp_minus1};


		lcp_element_t lcpValue;
		idx_element_t l;
		unsigned char c;
		for (size_t i = 0; i <= readMaxLength; ++i)
			supportBuffer[i] = -1; // ff

		openStreams2(&Bpart, readMaxLength + 1, "r", B_TPL);
		for (size_t i = 0; i < num_bytes; ++i) {
			sread(&l, IDX_EL_SIZE, &Iprev);
			if (supportBuffer[l] == -1) {
				c = getc(Bpart.f[l]);
				supportBuffer[l] = c & 0x0f;
				c = c >> 4;
			} else {
				c = supportBuffer[l];
				supportBuffer[l] = -1;
			}

			int decoded = charToCode(c);

			if (decoded != '$') {
				l++;
				fwrite_unlocked(&l, IDX_EL_SIZE, 1, Icur.f[c]);
			}

			lcpValue = 0;
			sread(&lcpValue, LCP_EL_SIZE, &Lprev);
			if (lcpValue != lcp_minus1) {
				for (int i = 1; i < 6; ++i) {
					alfa[i] = (alfa[i] == lcp_minus1)
						? alfa[i]
						: (alfa[i] <= lcpValue) ? alfa[i] : lcpValue;
				}
			}

			if (decoded != '$' && decoded != '#' && alfa[c] != lcp_minus1) {
				lcp_element_t a = alfa[c] + 1;
				maxLCP = (maxLCP <= a) ? a : maxLCP;
				fwrite_unlocked(&a, LCP_EL_SIZE, 1, Lcur.f[c]);
			}

			alfa[c] = readMaxLength + 1;	// i.e. infinity ...
		}
		closeStreams2(&Bpart);
		closeStreams2(&Iprev);
		closeStreams2(&Lprev);
		fseek(Icur.f[0], 0, SEEK_END);
		truncateStreams2(&Icur);
		closeStreams2(&Icur);
		fseek(Lcur.f[0], 0, SEEK_END);
		truncateStreams2(&Lcur);
		closeStreams2(&Lcur);

		p++;
	}
	free(supportBuffer);
	FILE *lcp;
	lcp = fopen(LCP_final, "wb");
	openStreams2(&Lprev, 6, "rb", Lpart_TPL(p));
	for (int i = 0; i < 6; ++i) {
		copyLCPFile(Lprev.f[i], lcp);
	}
	closeStreams2(&Lprev);
	fclose(lcp);

	openStreams2(&Iprev, 6, "rb", Ipart_TPL(p));
	reconstructInterleave(&Iprev, readMaxLength, (readMaxLength + 1) * totLines);
	closeStreams2(&Iprev);
}


//algorithm 4 is working, need to handle the # in LCP
