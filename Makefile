P = bwt_lcp
CC = gcc
CFLAGS = -Wall -lz -lm

$(P): $(P).c $(P).h kseq.h
	$(CC) $(CFLAGS) $(P).c -o $(P)

clean:
	@echo "Cleaning..."
	rm -rf *.o *.out *.exe $(P) tests/arrays/*