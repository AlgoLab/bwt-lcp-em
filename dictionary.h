/*
#ifndef DICTIONARY_H
#define DICTIONARY_H

char encode(char base){
	char encoded;
	if (base == 'A' || base == 'a')
		encoded = 0x00;

	else if(base == 'C' || base == 'c')
		encoded = 0x01;

	else if(base == 'G' || base == 'g')
		encoded = 0x02;

	else if(base == 'T' || base == 't')
		encoded = 0x03;

	else if(base == '$')
		encoded = 0x04;

	else {
		//assuming no other char
		assert(base == '#');
		encoded = 0x05;
	}

}

char merge(char first, char second){
	first = first << 4;
	return first & second;
}

/*
A -> 0000
C -> 0001
G -> 0010
T -> 0011
$ -> 0100
# -> 0101


#endif
*/