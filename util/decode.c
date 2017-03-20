#include <stdio.h>

char decode_c(int c) {
  switch(c) {
  case 0x00: return 'A';
  case 0x01: return 'C';
  case 0x02: return 'G';
  case 0x03: return 'T';
  case 0x04: return '$';
  case 0x05: return '#';
  }
  return '@';
}

int main(int argc, char **argv) {

  if(argc != 2) {
    fprintf(stderr, "Usage: %s <inputfile.txt>\n", argv[0]);
    return 1;
  }

  FILE* pFile;
  int c;
  char c1, c2;

  pFile = fopen(argv[1], "r");
  if(pFile == NULL) fprintf(stderr, "Error while opening the file\n");
  else {
    do {
      c = fgetc(pFile);
      c1 = decode_c((c >> 4));
      c2 = decode_c((c & 0b1111));
      printf("%c\n", c1);
      printf("%c\n", c2);
    } while(c!=EOF);
  }
  return 0;
}
