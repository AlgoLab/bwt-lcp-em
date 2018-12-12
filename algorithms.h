#ifndef ALGORITHMS_H
#define ALGORITHMS_H

#include <stdlib.h>
#include <zlib.h>
#include <stdio.h>
#include <limits.h>
#include "dictionary.h"
#include "streams.h"

#define T_TPL "./tests/arrays/T%d"
#define B_TPL "./tests/supportBWT/B%d"
#define P_TPL(l) ((l) % 2 == 0) ? "./tests/supportLists/Peven_%d" : "./tests/supportLists/Podd_%d"
#define Lpart_TPL(l) ((l) % 2 == 0) ? "./tests/supportLCP/Leven_%d" : "./tests/supportLCP/Lodd_%d"
#define Ipart_TPL(l) ((l) % 2 == 0) ? "./tests/supportLCP/Ieven_%d" : "./tests/supportLCP/Iodd_%d"
#define BWTbin_final "./tests/BWTbin"
#define LCP_final "./tests/LCP"

void computePartialBWT(int readMaxLength, int totLines);
void computeBWTLCP(size_t readMaxLength, size_t totLines, unsigned int *len_distr);

#endif
