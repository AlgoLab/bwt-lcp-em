P = bwt_lcp
CC = gcc
CFLAGS = -Wall
LDFLAGS = -lz -lm

$(P): $(P).c $(P).h kseq.h dictionary.h
	$(CC) $(CFLAGS) $(P).c dictionary.c -o $(P) $(LDFLAGS)

clean:
	@echo "Cleaning..."
	rm -rf *.o *.out *.exe $(P) tests/arrays/*
