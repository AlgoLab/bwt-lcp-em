CC = gcc
CFLAGS = -std=c99 -Wall -Wno-char-subscripts -Wno-unused-result -O3 -DNDEBUG -D_GNU_SOURCE
CFLAGS_DEBUG = $(CFLAGS) -UNDEBUG -O0 -g3
LDFLAGS = -lz -lm

P := bwt_lcp decode_bwt decode_lcp
SUBDIRS := $(addprefix tests/, arrays supportBWT supportLists supportLCP)
TEST_INPUTS := $(addprefix tests/, paper_example.fasta test.fasta.gz test.odd.fasta.gz test.253.fasta.gz test.290.fasta.gz test.7seqs.var.fasta)
TEST_SUMS := $(addsuffix .sums, $(TEST_INPUTS))

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
bwt_lcp: kseq.h dictionary.h dictionary.c streams.h streams.c algorithms.h algorithms.c common_types.h
decode_lcp: common_types.h

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
	time -p -o .time ./bwt_lcp $@ > .log 2> .err ; \
	diff .log $@.log; \
	diff -y -W 40 .time $@.time; \
	rm -f .time .log .err; \
	( test -s $(@) || md5sum -c $@.sums; );

.PHONY: rebuild-test
rebuild-test: clean subdirs bin $(TEST_SUMS)

.PHONY: $(TEST_SUMS)
$(TEST_SUMS): clean subdirs bin
	@echo "Rebuilding test $@ (no output)" && \
	time -p -o $(basename $(@)).time ./bwt_lcp $(basename $(@)) > $(basename $(@)).log && \
	md5sum tests/BWTbin tests/LCP > $(@) || \
	true > $(@)

%.debug: %.c
	$(CC) $(CFLAGS_DEBUG) $^ -o $@ $(LDFLAGS)

bwt_lcp.debug: kseq.h dictionary.h dictionary.c streams.h streams.c algorithms.h algorithms.c common_types.h
decode_lcp.debug: common_types.h

.PHONY: debug
debug: subdirs bwt_lcp.debug decode_bwt.debug decode_lcp.debug
