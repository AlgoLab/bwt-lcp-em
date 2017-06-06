P = bwt_lcp
CC = gcc
CFLAGS = -std=c99 -Wall -Wno-char-subscripts
LDFLAGS = -lz -lm

$(P): $(P).c $(P).h kseq.h dictionary.h dictionary.c streams.h streams.c algorithms.h algorithms.c
	$(CC) $(CFLAGS) $(P).c dictionary.c streams.c algorithms.c -o $(P) $(LDFLAGS) && mkdir -p tests/arrays tests/outputFiles tests/supportFiles tests/supportLists tests/BWT tests/LCP tests/supportBWT tests/supportLCP

clean:
	@echo "Cleaning..."
	rm -rf *.o *.out *.exe $(P) tests/arrays/* decode tests/arrays/* tests/outputFiles/* tests/supportFiles/* tests/supportLists/* tests/BWT/* tests/LCP/* tests/supportBWT/* tests/supportLCP/* tests/B_W_T

decode: util/decode.c
	$(CC) $(CFLAGS) util/decode.c -o decode $(LDFLAGS)

test:
	./$(P) tests/uploaded_test.fasta

debug: $(P).c $(P).h kseq.h dictionary.h dictionary.c streams.h streams.c algorithms.h algorithms.c
	$(CC) $(CFLAGS) -g $(P).c dictionary.c streams.c algorithms.c -o $(P) $(LDFLAGS) && mkdir -p tests/arrays tests/outputFiles tests/supportFiles tests/supportLists tests/BWT tests/LCP tests/supportBWT tests/supportLCP

