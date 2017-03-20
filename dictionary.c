#include "dictionary.h"


/*
A -> 0000
C -> 0001
G -> 0010
T -> 0011
$ -> 0100
# -> 0101
@ -> 0110 -> indicates no information (used when only the first half of the byte is used)
*/

//	ENCODING

// Given a char, say c, returns a byte in this form: 0000 codeOf(c)
char getCode(char base){
	if (base == 'A' || base == 'a')
		return 0x00;

	else if(base == 'C' || base == 'c')
		return 0x01;

	else if(base == 'G' || base == 'g')
		return 0x02;

	else if(base == 'T' || base == 't')
		return 0x03;

	else if(base == '$')
		return 0x04;

	else if(base == '#') // useful?
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
	if (toDecode == 0x0)
		return 'A';

	else if(toDecode == 0x01)
		return 'C';

	else if(toDecode == 0x02)
		return 'G';

	else if(toDecode == 0x03)
		return 'T';

	else if(toDecode == 0x04)
		return '$';

	else if(toDecode == 0x05) // useful?
		return '#';

	else {
		//assuming no other char
		assert(toDecode == 0x06);
		return '@';
	}		
}

char getChar(char toDecode, bool odd) {
	char first = toDecode & 0xF0; // first AND 1111 0000
	first = first >> 4;
	char second = toDecode & 0x0F; // first AND 0000 1111

	first = charToCode(first);
	second = charToCode(second);
	if(odd)
		return second;
	else
		return first;


}


// readToDecode starts from 0, assuming is correct (i.e > 0 and < maxRead)
char *decode(FILE **filePointers, int readMaxLength, int readToDecode) {
	// Opening all streams in "read-only" mode
	const char *filepathTemplate = "./tests/arrays/T%d.txt";
	int sizeFilepath = (int)(ceil(log10(readMaxLength))+1);
	char *filepath = (char *)calloc(sizeFilepath + strlen(filepathTemplate) - 2, sizeof(char));
	for (int i = 0; i <= readMaxLength; ++i) {
		sprintf(filepath, filepathTemplate, i);
		filePointers[i] = fopen(filepath, "r");
	}

	// could be more efficient by storing the 
	// size of each read in an array and passing
	// it as a parameter	
	char *read = (char *)malloc(readMaxLength+1 * sizeof(char));
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
	read = realloc(read, readCounter * sizeof(char));

	// Close all streams and free memory
	for (int i = 0; i <= readMaxLength; ++i) {
		fclose(filePointers[i]); 	
	}

	free(filepath);	

	return read;

}