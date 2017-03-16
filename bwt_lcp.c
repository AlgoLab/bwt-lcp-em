#include "bwt_lcp.h"
// #include "dictionary.h" <- incompletoo

KSEQ_INIT(gzFile, gzread)

int main(int argc, char *argv[]) {
	gzFile fp;
	kseq_t *seq;
	int readMaxLength;
	int l;
	// Errore se usato senza argomenti
	if (argc == 1) {
    	fprintf(stderr, "Usage: %s <in.fasta>\n", argv[0]);
    	return 1;
  	}
	fp = gzopen(argv[1], "r");
	seq = kseq_init(fp);
	// calcolo della lunghezza del read più lungo (utilizzato per sapere quanti stream aprire)
	while((l = kseq_read(seq)) >= 0){
		if(l > readMaxLength)
			readMaxLength = l;
	}
	//tanti buffer per ogni file che scrivono in automatico
	const char *filepathTemplate = "./tests/arrays/T%d.txt";
	int sizeFilepath = (int)(ceil(log10(readMaxLength))+1);
	char *filepath = (char *)calloc(sizeFilepath + strlen(filepathTemplate) - 2, sizeof(char));
	// Array che contiene tutti i puntatori ai files
	FILE **filePointers = (FILE **)malloc((readMaxLength+1) * sizeof(FILE *));
	// Apertura di tutti gli stream in scrittura
	for (int i = 0; i <= readMaxLength; ++i) {
		sprintf(filepath, filepathTemplate, i);
		filePointers[i] = fopen(filepath, "w");
		//setbuf(filePointers[i], NULL); --> la libreria standard utilizza già buffer efficienti
	}
	// macro che ritorna all'inizio del file
	REWIND(seq, fp)
	int j = 0;
	int linesCounter = 0;
	while((j = kseq_read(seq)) >= 0){
		int dxAlignCounter = readMaxLength;
		while(dxAlignCounter > 0){
			//printf("%d\n", dxAlignCounter);
			//printf("%d\n\n", j);
			if(j > 0){
				fputc(seq -> seq.s[j-1], filePointers[dxAlignCounter-1]);
				j--;
				dxAlignCounter--;
			}
			else {//inserisci sentinella
				fputc('#', filePointers[dxAlignCounter-1]);
				dxAlignCounter--;
			}
		}
		linesCounter++;
	}
	// inserimento di tutti dollari nell'ultimo file (quindi in coda a tutte le stringhe)
	char *sentinels = (char *)malloc(linesCounter * sizeof(char));
	for (int i = 0; i < linesCounter; ++i) {
		sentinels[i] = '$';
	}
	fputs(sentinels, filePointers[readMaxLength]);
	
	// chiusura di tutti gli stream e free di tutta la memoria
	for (int i = 0; i <= readMaxLength; ++i) {
		fclose(filePointers[i]); 	
	}
	free(sentinels);
	free(filePointers);
	free(filepath);
	kseq_destroy(seq);
  	gzclose(fp);

  	/*
		ToDo:  2. Codificare l'alfabeto in 4 bit (1/2 di char)

  	*/

  	return 0;
}