#ifndef BWT_LCP_H
#define BWT_LCP_H
#include <zlib.h>
#include <stdio.h>
#include <math.h>
#include "kseq.h"

#define REWIND(kseq, gzfp) \
	kseq_rewind(kseq); 	\
	gzrewind(gzfp);

#endif