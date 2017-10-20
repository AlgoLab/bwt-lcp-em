#include "dictionary.h"



//	ENCODING

// Given a char, say c, returns a byte in this form: 0000 codeOf(c)
static const char encode_mask[256] = {
  /*   0 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  16 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  32 */ 0, 0, 0, 0x05, 0x00, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  48 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  64 */ 0x06, 0x01, 0, 0x02, 0, 0, 0, 0x03, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  80 */ 0, 0, 0, 0, 0x04, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /*  96 */ 0, 0x01, 0, 0x02, 0, 0, 0, 0x03, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 112 */ 0, 0, 0, 0, 0x04, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 128 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 144 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 160 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 176 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 192 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 208 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 224 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
  /* 240 */ 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0,
};
char getCode(char base){
	return encode_mask[base];
}

// Given two encoded char (they only use the lower half of the byte) merge them together in one char
// so that the first "code" is in the upper half of the byte and the second in the lower half.
char merge(char first, char second){
	first = encode_mask[first];
	second = encode_mask[second];

	first = first << 4;
	return first | second;
}

//	DECODING
static const char decode_mask[7] = {'$', 'A', 'C', 'G', 'T', '#', '@'};
char charToCode(char toDecode) {
	return decode_mask[toDecode];
}

char getChar(char toDecode, bool select) {
  if (select) {
    return decode_mask[toDecode & 0x0F];
  } else {
    return decode_mask[(char)((toDecode & 0xF0) >> 4)];
  }
}


// readToDecode starts from 0, assuming is correct (i.e > 0 and < maxRead)
char *decodeRead(FILE **filePointers, int readMaxLength, int readToDecode) {
	// Opening all streams in "read-only" mode
	openStreams(filePointers, readMaxLength, "r", "./tests/arrays/T%d");

	// could be more efficient by storing the 
	// size of each read in an array and passing
	// it as a parameter	
	char *read = malloc(readMaxLength+2 * sizeof(char));
	int charIndex = (readToDecode / 2);
	bool odd = readToDecode % 2; // 0 if even, 1 if odd
	int readCounter = 0;
	for (int i = 0; i <= readMaxLength; ++i) {
		fseek(filePointers[i], charIndex, SEEK_SET);
		char reading = fgetc(filePointers[i]);
		char decoded = getChar(reading, odd);
		if(decoded != '#') {
			read[readCounter] = decoded;
			readCounter++;
		}
	}
	read[readCounter] = '\0';
	read = realloc(read, readCounter * sizeof(char));

	// Close all streams
	closeStreams(filePointers, readMaxLength);

	return read;

}

char getCharFromColumn(int index, int column, FILE **filePointers){
	// check if column < readmaxlength and if index < linesCounter+
	// by passing them as parameters?
	openStream(filePointers, column, "r", "./tests/arrays/T%d"); // used only with filePointers?
	int charIndex = index / 2;
	bool odd = index % 2;
	fseek(filePointers[column], charIndex, SEEK_SET);
	char reading = fgetc(filePointers[column]);
	char decoded = getChar(reading, odd);
	// Close stream
	fclose(filePointers[column]);

	return decoded;
}


char *getEncodedColumn(FILE **filePointers, int column) {
	fseek(filePointers[column], 0l, SEEK_END);
	int fileSize = ftell(filePointers[column]);
	rewind(filePointers[column]);
	char *encodedColumn = malloc(fileSize+1 * sizeof(char));
	encodedColumn = fgets(encodedColumn, fileSize+1, filePointers[column]);
	//check if != null
	return encodedColumn;
}

char *getDecodedColumn(FILE **filePointers, int column) {
	char *encodedColumn = getEncodedColumn(filePointers, column);
  size_t strlen_encodedColumn = strlen(encodedColumn);
	char *decodedColumn = malloc(((strlen_encodedColumn * 2) + 1) * sizeof(char));
	
	int j = 0;
	for (int i = 0; i < strlen_encodedColumn; ++i) {
		decodedColumn[j] = getChar(encodedColumn[i], 0);
		++j;
		decodedColumn[j] = getChar(encodedColumn[i], 1);
		++j;
	}

	if(decodedColumn[(strlen_encodedColumn * 2) - 1] == '@')
		decodedColumn[(strlen_encodedColumn * 2) - 1] = '\0';
	else
		decodedColumn[(strlen_encodedColumn * 2)] = '\0';

	free(encodedColumn);
	
	return decodedColumn;

}
