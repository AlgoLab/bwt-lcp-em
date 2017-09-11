from __future__ import division
from six.moves import xrange

import argparse
import numpy as np


parser = argparse.ArgumentParser()
parser.add_argument('--sequences', type=int, default=100)
parser.add_argument('--length', type=int, default=100)
parser.add_argument('--fname', default='test2.fasta')

args = parser.parse_args()


if __name__ == '__main__':
    
    seqs = np.zeros((args.sequences), dtype=object)
    choices = np.array(['A','C','G','T'], dtype='S1')

    for i in xrange(args.sequences):
        var = args.length // 10
        seqs[i] = np.random.choice(choices, size=args.length)

    with open(args.fname, 'w') as f:
        for i in xrange(args.sequences):
            f.write('>seq%d\n%s\n' % (i, seqs[i].tostring().decode('utf-8')))

    







