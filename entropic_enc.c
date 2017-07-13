#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <math.h>
//#include <limits.h>
#include "main.h"
#include "entropic_enc.h"

#define testBit(A,k) ((A & (1<<(k)))>>k)
unsigned char *huff_table;
unsigned char rlc_length, condition_length;

double timeval_diff(struct timeval *a, struct timeval *b) {
	return ((double)(a->tv_sec +(double)a->tv_usec/1000000)-(double)(b->tv_sec + (double)b->tv_usec/1000000));
}

void init_huffmann_table() {

	huff_table = malloc(9);
	huff_table[0] = 0b00000000;
	huff_table[1] = 0b00000010;
	huff_table[2] = 0b00001000;
	huff_table[3] = 0b00100000;
	huff_table[4] = 0b10000000;
	huff_table[5] = 0b01000000;
	huff_table[6] = 0b00010000;
	huff_table[7] = 0b00000100;
	huff_table[8] = 0b00000001;

}

/*
		switch (hops[line][i]) {
			case 4:

		}
*/




void entropic_enc(char **hops, int line) {

	unsigned int counterh0 = 0;
	unsigned int counterh0_prev = 0;
	unsigned char mode = 0;
	unsigned char mode_prev = mode;
	unsigned char rlc_length_ini = 4;
	unsigned char condition_length_ini = 7;

	rlc_length = rlc_length_ini;
	condition_length = condition_length_ini;

	for (int i = 0; i < sizeof(hops[line]); i++) {
		counterh0_prev = counterh0;
		if (hops[line][i]==4){
			counterh0+=1;
			if (mode==0 && counterh0==condition_length) {mode=1; counterh0=0;}
		} else {
			mode=0;
			counterh0=0;
		}

		if (mode==1 && counterh0==(unsigned char)(pow(2, rlc_length)-1)) {
			counterh0=0;
			rlc_length=5;
		}

		if (mode_prev==1 && mode==0) {
			condition_length=condition_length_ini;
			rlc_length=rlc_length_ini;
		}





	}

}

int main(int argc, char* argv[]) {

	struct timeval t_ini, t_fin;
	double secs;

	gettimeofday(&t_ini, NULL);

	init_huffmann_table();
	for (int j = 0; j < 9; j++){
		for (int i = 7; i >= 0; i--){
			printf("%u", testBit(huff_table[j], i));
			if (testBit(huff_table[j], i) == 1) {
				break;
			}
		}
		printf("\n");
	}

	gettimeofday(&t_fin, NULL);

	secs = timeval_diff(&t_fin, &t_ini)/10000;
	printf("%.16g ms\n", secs * 1000.0);

	return 0;
}