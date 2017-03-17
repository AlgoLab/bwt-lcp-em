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
