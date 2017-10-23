P = bwt_lcp
CC = gcc
CFLAGS = -std=c99 -Wall -Wno-char-subscripts -O2 -DNDEBUG
CFLAGS_DEBUG = $(CFLAGS) -UNDEBUG -O0 -g3
LDFLAGS = -lz -lm

SUBDIRS := $(addprefix tests/, arrays outputFiles supportFiles supportLists BWT LCP supportBWT supportLCP)

TEST_INPUT := tests/test.fasta.gz

.PHONY: bin
bin: subdirs bwt_lcp decode_bwt decode_lcp

.PHONY: subdirs
subdirs: $(SUBDIRS)

# General rule for building the executables
%: %.c
	$(CC) $(CFLAGS) $^ -o $@ $(LDFLAGS)
# Additional requirements for bwt_lcp
bwt_lcp: kseq.h dictionary.h dictionary.c streams.h streams.c algorithms.h algorithms.c

$(SUBDIRS):
	mkdir -p $@

.PHONY: clean
clean:
	@echo "Cleaning..."
	rm -rf *.o *.out *.exe *.debug bwt_lcp decode_bwt decode_lcp $(SUBDIRS) tests/B_W_T

.PHONY: test
test: clean bin
	@echo "Starting test on file $(TEST_INPUT) (no output)" && \
	time -p ./bwt_lcp $(TEST_INPUT) > .log 2> .time && \
	diff .log $(TEST_INPUT).log; \
	diff -y .time $(TEST_INPUT).time; \
	rm -rf .log .time; \
	md5sum -c $(TEST_INPUT).sums

%.debug: %.c
	$(CC) $(CFLAGS_DEBUG) $^ -o $@ $(LDFLAGS)

bwt_lcp.debug: kseq.h dictionary.h dictionary.c streams.h streams.c algorithms.h algorithms.c

.PHONY: debug
debug: subdirs bwt_lcp.debug decode_bwt.debug decode_lcp.debug
