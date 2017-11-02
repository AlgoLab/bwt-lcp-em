CC = gcc
CFLAGS = -std=c99 -Wall -Wno-char-subscripts -Wno-unused-result -O3 -DNDEBUG -D_GNU_SOURCE
CFLAGS_DEBUG = $(CFLAGS) -UNDEBUG -O0 -g3
LDFLAGS = -lz -lm

P := bwt_lcp decode_bwt decode_lcp
SUBDIRS := $(addprefix tests/, arrays supportBWT supportLists supportLCP)
TEST_INPUTS := $(addprefix tests/, paper_example.fasta test.fasta.gz test.odd.fasta.gz test.253.fasta.gz test.290.fasta.gz)

# Make must run serially (even if -j N > 1 is given) (GNU make extension)
.NOTPARALLEL:

.PHONY: bin
bin: $(P)

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
	rm -rf *.o *.out *.exe *.debug $(P) $(SUBDIRS) tests/BWTbin tests/LCP

.PHONY: test
test: clean subdirs bin $(TEST_INPUTS)

.PHONY: $(TEST_INPUTS)
$(TEST_INPUTS): clean subdirs bin
	@echo "Starting test on file $@ (no output)" && \
	time -p -o .time ./bwt_lcp $@ > .log 2> .err && \
	( \
		diff .log $@.log; \
		diff -y -W 40 .time $@.time; \
		rm -f .time .log .err; \
		md5sum -c $@.sums; \
	)

%.debug: %.c
	$(CC) $(CFLAGS_DEBUG) $^ -o $@ $(LDFLAGS)

bwt_lcp.debug: kseq.h dictionary.h dictionary.c streams.h streams.c algorithms.h algorithms.c

.PHONY: debug
debug: subdirs bwt_lcp.debug decode_bwt.debug decode_lcp.debug
