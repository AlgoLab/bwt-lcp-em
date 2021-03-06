#include <stdio.h>

char codeToChar(char toDecode) {
  char mask[7] = {'$', 'A', 'C', 'G', 'T', '#', '@'};

  return mask[toDecode];  
}

int main(int argc, char **argv) {

  if(argc != 2) {
    fprintf(stderr, "Usage: %s <bwtfile>\n", argv[0]);
    return 1;
  }

  FILE* pFile;
  int c;
  char c1, c2;

  pFile = fopen(argv[1], "r");
  if(pFile == NULL) {
    fprintf(stderr, "Error while opening the file\n");
    return 1;
  }
  else {
    while((c = fgetc(pFile)) != EOF) {
      c1 = codeToChar((c >> 4));
      c2 = codeToChar((c & 0b1111));
      printf("%c", c1);
      if(c2 != '@')
        printf("%c", c2);
    }
  }
  fclose(pFile);
  return 0;
}
