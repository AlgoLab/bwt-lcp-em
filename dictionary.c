#include "dictionary.h"



//	ENCODING

// Given a char, say c, returns a byte in this form: 0000 codeOf(c)
char getCode(char base){
	if (base == '$')
		return 0x00;

	else if(base == 'A' || base == 'a')
		return 0x01;

	else if(base == 'C' || base == 'c')
		return 0x02;

	else if(base == 'G' || base == 'g')
		return 0x03;

	else if(base == 'T' || base == 'T')
		return 0x04;

	else if(base == '#')
		return 0x05;

	else {
		//assuming no other char
		assert(base == '@');
		return 0x06;
	}
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
	if (toDecode == 0x00)
		return '$';

	else if(toDecode == 0x01)
		return 'A';

	else if(toDecode == 0x02)
		return 'C';

	else if(toDecode == 0x03)
		return 'G';

	else if(toDecode == 0x04)
		return 'T';

	else if(toDecode == 0x05)
		return '#';

	else {
		//assuming no other char
		//printf("%hhx\n", toDecode);
		assert(toDecode == 0x06);
		return '@';
	}		
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
	char **fileIOBuffers = openStreams(filePointers, readMaxLength, "r", "./tests/arrays/T%d.txt");

	// could be more efficient by storing the 
	// size of each read in an array and passing
	// it as a parameter	
	char *read = (char *)malloc(readMaxLength+2 * sizeof(char));
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
	closeStreams(filePointers, readMaxLength, fileIOBuffers);

	return read;

}

char getCharFromColumn(int index, int column, FILE **filePointers){
	// check if column < readmaxlength and if index < linesCounter+
	// by passing them as parameters?
	//openStream(filePointers, column, "r", "./tests/arrays/T%d.txt");
	int charIndex = index / 2;
	bool odd = index % 2;
	fseek(filePointers[column], charIndex, SEEK_SET);
	char reading = fgetc(filePointers[column]);
	char decoded = getChar(reading, odd);
	// Close stream and free memory
	//fclose(filePointers[column]);

	return decoded;
}


char *getEncodedColumn(FILE **filePointers, int column) {
	fseek(filePointers[column], 0l, SEEK_END);
	int fileSize = ftell(filePointers[column]);
	rewind(filePointers[column]);
	char *encodedColumn = (char *)malloc(fileSize+1 * sizeof(char));
	encodedColumn = fgets(encodedColumn, fileSize+1, filePointers[column]);
	//check if != null
	return encodedColumn;
}

char *getDecodedColumn(FILE **filePointers, int column) {
	char *encodedColumn = getEncodedColumn(filePointers, column);
	char *decodedColumn = (char *)malloc(((strlen(encodedColumn) * 2) + 1) * sizeof(char));
	
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