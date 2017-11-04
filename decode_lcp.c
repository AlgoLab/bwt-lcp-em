#include <stdio.h>

#include "common_types.h"

int main(int argc, char const *argv[]) {
	if(argc != 2) {
		fprintf(stderr, "Usage: %s <lcpfile>\n", argv[0]);
		return 1;
	}

	FILE* pFile;
	pFile = fopen(argv[1], "rb");
	if(pFile == NULL) {
		fprintf(stderr, "Error while opening file '%s'\n", argv[1]);
		return 1;
	}
	else {
		lcp_out_element_t c = 0;
		int o = 0;
		while(fread(&c, LCP_OUT_EL_SIZE, 1, pFile) == 1) {
			o = c;
			if (c == (lcp_out_element_t)(-1)) {
				o = -1;
			}
			printf("%d\n", o);
		}
	}

	return 0;
}
