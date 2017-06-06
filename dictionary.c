#include "dictionary.h"



//	ENCODING

// Given a char, say c, returns a byte in this form: 0000 codeOf(c)
char getCode(char base){	
	char mask[256];
	mask[36] = 0x00;	//$
	mask[65]= 0x01;		//A
	mask[97] = 0x01;	//a
	mask[67] = 0x02;	//C
	mask[99] = 0x02;	//c
	mask[71] = 0x03;	//G
	mask[103] = 0x03;	//g
	mask[84] = 0x04;	//T
	mask[116] = 0x04;	//t
	mask[35] = 0x05;	//#
	mask[64] = 0x06;	//@
	
	char res = mask[base];
	assert(res != 0);	

	return res;
}

// Given two encoded char (they only use the lower half of the byte) merge them together in one char
// so that the first "code" is in the upper half of the byte and the second in the lower half.
char merge(char first, char second){
	char res;
	first = getCode(first);
	second = getCode(second);

	first = first << 4;
	res = first | second;
	return res;	

}

//	DECODING

char charToCode(char toDecode) {
	char mask[7] = {'$', 'A', 'C', 'G', 'T', '#', '@'};

	return mask[toDecode];	
}

char getChar(char toDecode, bool select) {
	char first = toDecode & 0xF0; // first AND 1111 0000
	first = first >> 4;
	char second = toDecode & 0x0F; // first AND 0000 1111

	first = charToCode(first);
	second = charToCode(second);
	if(select)
		return second;
	else
		return first;


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
	char *decodedColumn = malloc(((strlen(encodedColumn) * 2) + 1) * sizeof(char));
	
	int j = 0;
	for (int i = 0; i < strlen(encodedColumn); ++i) {
		decodedColumn[j] = getChar(encodedColumn[i], 0);
		++j;
		decodedColumn[j] = getChar(encodedColumn[i], 1);
		++j;
	}

	if(decodedColumn[(strlen(encodedColumn) * 2) - 1] == '@')
		decodedColumn[(strlen(encodedColumn) * 2) - 1] = '\0';
	else
		decodedColumn[(strlen(encodedColumn) * 2)] = '\0';

	free(encodedColumn);
	
	return decodedColumn;

}