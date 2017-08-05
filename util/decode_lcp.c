#include <stdio.h>

int main(int argc, char const *argv[]) {
	if(argc != 2) {
    	fprintf(stderr, "Usage: %s <bwtfile>\n", argv[0]);
    	return 1;
  	}

  	FILE* pFile;
  	int c = 0;

  	pFile = fopen(argv[1], "rb");
  	if(pFile == NULL) {
    	fprintf(stderr, "Error while opening the file\n");
    	return 1;
  	}
  	else {
  		while(fread(&c, sizeof(int), 1, pFile) == 1)
  			printf("%d ", c);
  	}

  	printf("\n");

	return 0;
}