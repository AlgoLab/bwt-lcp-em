# bwt-lcp-em

[![Build Status](https://travis-ci.org/AlgoLab/bwt-lcp-em.svg?branch=master)](https://travis-ci.org/AlgoLab/bwt-lcp-em)

## Input and Output

The input for this program should be a FASTA file on the alphabet `{A, C, G, T}`.
Some examples are provided in the directory `tests/`.  
The output of the program are the BWT and the LCP array of the collection of strings in the input file.
After the main command execution, the binary-encoded BWT is contained in the file `tests/BWTbin`,
while the binary LCP array is contained in the file `tests/LCP`.
Each entry of the latter file is a 8-bit unsigned integer with the only exception of `-1` encoded as `0xFF`.
Both files can be decoded into human-readable files with the commands `decode_bwt` and `decode_lcp`.

## Compiling and running

After cloning the repository:

```
cd bwt-lcp-em
make subdirs bin
```

Now it is possible to run the program as follows:

```
./bwt_lcp <path to the FASTA file>
./decode_bwt tests/BWTbin > <path where to save readable BWT>
./decode_lcp tests/LCP > <path where to save readable LCP>
```

For example, for invoking the program on one of the provided examples,
we can enter the following commands:

```
make subdirs bin
./bwt_lcp tests/uploaded_test.fasta
# Optionally, to obtain the results in a human-readable format:
./decode_bwt tests/BWTbin > BWT.txt
./decode_lcp tests/LCP > LCP.txt
```

## Test instances

Some test instances are provided in the repository.
To run them, one can issue the command `make test`.

## Citation

The algorithm is described in:  
**Computing the BWT and LCP array of a Set of Strings in External Memory**.  
Paola Bonizzoni, Gianluca Della Vedova, Yuri Pirola, Marco Previtali, Raffaella Rizzi.  
[arXiv:1705.07756](https://arxiv.org/abs/1705.07756), 2017.
