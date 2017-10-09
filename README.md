# bwt-lcp-em

[![Build Status](https://travis-ci.org/AlgoLab/bwt-lcp-em.svg?branch=master)](https://travis-ci.org/AlgoLab/bwt-lcp-em)

# Input and Output

The input for this program should be a FASTA file on the alphabet {A, C, G, T}. An example can be found in test/uploaded_test.fasta
The output of the program are the BWT and the LCP array of the collection of strings in the input file. After the main command execution the file containing the encoded BWT is: `tests/B_W_T`, the file containing the LCP (binary file) is: `tests/LCP/LCP`. Both of these file can be decoded into ASCII with the commands `decode_bwt` and `decode_lcp`.

# Compiling and running

After cloning the repository:
```
cd bwt-lcp-em
make
make decode_bwt
make decode_lcp

```

Now it's possible to run the program as follows:

```
./bwt_lcp <path/to/file>
./decode_bwt tests/B_W_T > <path where to save readable BWT>
./decode_lcp tests/LCP/LCP > <path where to save readable LCP>

```



Sample invokation

Test instances


The algorithm is described in the paper [Computing the BWT and LCP array of a Set of Strings in External Memory](https://arxiv.org/abs/1705.07756).

The full citation is:
**Computing the BWT and LCP array of a Set of Strings in External Memory**.
Paola Bonizzoni, Gianluca Della Vedova, Yuri Pirola, Marco Previtali, Raffaella Rizzi.
[arXiv:1705.07756](https://arxiv.org/abs/1705.07756), 2017.
