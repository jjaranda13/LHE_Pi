#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <limits.h>
#include "include/globals.h"
#include "include/entropic_enc.h"

#define testBit(A,k) ((A & (1UL<<(k)))>>k)

void init_entropic_enc(){

	bits_Y = malloc(height_down_Y*sizeof(unsigned char *));

	for (int i=0;i<height_down_Y;i++) {
		bits_Y[i]=malloc(width_down_Y*sizeof (unsigned char));
	}

	bits_U = malloc(height_down_UV*sizeof(unsigned char *));

	for (int i=0;i<height_down_UV;i++) {
		bits_U[i]=malloc(width_down_UV*sizeof (unsigned char));
	}

	bits_V = malloc(height_down_UV*sizeof(unsigned char *));

	for (int i=0;i<height_down_UV;i++) {
		bits_V[i]=malloc(width_down_UV*sizeof (unsigned char));
	}
} //Los pongo a cero porque voy a usar máscaras para colocar los bits

//#define LENGTH 32
#define LENGTH sizeof(mask)*CHAR_BIT

int entropic_enc(unsigned char **hops, unsigned long int **bits, unsigned int line, unsigned int line_width) {

	unsigned int counterh0 = 0;
	unsigned char mode = 0; //0=huffman, 1=rlc
	unsigned char mode_prev = mode;
	unsigned char rlc_length_ini = 4;
	unsigned char condition_length_ini = 7;
	unsigned int bytes = 0;
	unsigned int moves = 1;
	unsigned long int mask = 1UL << sizeof(mask)*(CHAR_BIT-1);
	unsigned long int aux = 0;
	int hop = 0;

	rlc_length = rlc_length_ini;
	condition_length = condition_length_ini;
	for (int i = 0; i < line_width; i++) {
        hop = hops[line][i];
		if (hop==4){
			counterh0+=1;
			if (mode==0 && counterh0==condition_length) {
				mode=1;
				counterh0=0;
				aux |= mask;
				mask = (mask >> 1) | (mask << (LENGTH-1));
				moves += 1;
				if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
			}
		} else {
            if (mode_prev == 0) {counterh0 = 0;}
			mode=0;
		}

        if (mode == 0 && mode_prev == 0) {
			switch (hop) {
                case 4:
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 5:
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 3:
                    mask = (mask >> 2) | (mask << (LENGTH-2));
                    moves += 2;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 6:
                    mask = (mask >> 3) | (mask << (LENGTH-3));
                    moves += 3;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 2:
                    mask = (mask >> 4) | (mask << (LENGTH-4));
                    moves += 4;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 7:
                    mask = (mask >> 5) | (mask << (LENGTH-5));
                    moves += 5;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 1:
                    mask = (mask >> 6) | (mask << (LENGTH-6));
                    moves += 6;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 8:
                    mask = (mask >> 7) | (mask << (LENGTH-7));
                    moves += 7;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 0:
                    mask = (mask >> 8) | (mask << (LENGTH-8));
                    moves += 8;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
            }
		} else if (mode_prev==1 && mode==0) {
			for (int b = rlc_length-1; b >= 0; b--){
				aux |= (testBit(counterh0, b) * mask);
				mask = (mask >> 1) | (mask << (LENGTH-1));
                moves += 1;
                if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
			}
			condition_length=condition_length_ini;
			rlc_length=rlc_length_ini;
			counterh0=0;

			switch (hop) {
                case 5:
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 3:
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 6:
                    mask = (mask >> 2) | (mask << (LENGTH-2));
                    moves += 2;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 2:
                    mask = (mask >> 3) | (mask << (LENGTH-3));
                    moves += 3;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 7:
                    mask = (mask >> 4) | (mask << (LENGTH-4));
                    moves += 4;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 1:
                    mask = (mask >> 5) | (mask << (LENGTH-5));
                    moves += 5;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 8:
                    mask = (mask >> 6) | (mask << (LENGTH-6));
                    moves += 6;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    aux |= mask;
                    mask = (mask >> 1) | (mask << (LENGTH-1));
                    moves += 1;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
                case 0:
                    mask = (mask >> 7) | (mask << (LENGTH-7));
                    moves += 7;
                    if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
                    break;
            }
		} else if (mode==1 && ((rlc_length==4 && counterh0==15) || (rlc_length==5 && counterh0==31))){
			counterh0=0;
			rlc_length=5;
			aux |= mask;
			mask = (mask >> 1) | (mask << (LENGTH-1));
			moves += 1;
            if (moves >= LENGTH) {bits[line][bytes]=aux; aux=0; bytes++; moves-=LENGTH;}
		}

		mode_prev = mode;

	}
	bits[line][bytes]=aux;
	int bits_count = (bytes-1)*32+moves;
	return bits_count;

}


/*
int main(int argc, char* argv[]) {

	//struct timeval t_ini, t_fin;
	//double secs;
	unsigned long int mask = 0;
	//gettimeofday(&t_ini, NULL);
	printf("Iniciando el coder entropico\n");
	init_entropic_enc();
	printf("Coder entropico inicializado. Se pasa a iniciar hops\n");
	unsigned char **hops;
	hops = malloc(2*sizeof(unsigned char *));
	for (int i = 0; i < 2; i++) {
		hops[i]=malloc(20);
	}
	hops[0][0] = 5;
	hops[0][1] = 8;
	hops[0][2] = 8;
	hops[0][3] = 8;
	hops[0][4] = 8;
	hops[0][5] = 4;
	hops[0][6] = 4;
	hops[0][7] = 4;
	hops[0][8] = 4;
	hops[0][9] = 4;
	hops[0][10] = 4;
	hops[0][11] = 4;
	hops[0][12] = 4;
	hops[0][13] = 4;
	hops[0][14] = 4;
	hops[0][15] = 4;
	hops[0][16] = 4;
	hops[0][17] = 4;
	hops[0][18] = 4;
	hops[0][19] = 6;
	hops[1][0] = 4;
	hops[1][1] = 4;
	hops[1][2] = 3;
	hops[1][3] = 2;
	hops[1][4] = 1;
	hops[1][5] = 3;
	hops[1][6] = 4;
	hops[1][7] = 4;
	hops[1][8] = 5;
	hops[1][9] = 7;
	hops[1][10] = 6;
	hops[1][11] = 8;
	hops[1][12] = 3;
	hops[1][13] = 5;
	hops[1][14] = 4;
	hops[1][15] = 4;
	hops[1][16] = 6;
	hops[1][17] = 2;
	hops[1][18] = 3;
	hops[1][19] = 4;
	printf("Hops inicializados se pasa a crear bits.\n");
	unsigned long int **bits;
	bits = malloc(2*sizeof(unsigned char*));
	for (int i = 0; i < 2; i++) {
		bits[i]=malloc(20*sizeof(unsigned char));
	}
	printf("Bits creado, se pasa a ejecutar el codificador.\n");
	int bytes = entropic_enc(hops, bits, 0, 20);

	for (int j = 0; j <= bytes; j++){
		for (int i = LENGTH-1; i >= 0; i--){
			printf("%lu", testBit(bits[0][j], i));
		}
		//printf("\n");
	}
	printf("\n");
	//gettimeofday(&t_fin, NULL);

	//secs = timeval_diff(&t_fin, &t_ini)/10000;
	//printf("%.16g ms\n", secs * 1000.0);

	return 0;
}
*/
