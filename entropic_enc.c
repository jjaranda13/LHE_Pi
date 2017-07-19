#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <math.h>
#include "globals.h"
#include "entropic_enc.h"

#define testBit(A,k) ((A & (1<<(k)))>>k)

/*
double timeval_diff(struct timeval *a, struct timeval *b) {
	return ((double)(a->tv_sec +(double)a->tv_usec/1000000)-(double)(b->tv_sec + (double)b->tv_usec/1000000));
}
*/


/////////////////////////////////////////////////////////////////////////
//Hay que ponerle los valores correctos a rlc_length_ini y a rlc_length//
/////////////////////////////////////////////////////////////////////////


void init_huffman_table() {

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

void init_entropic_enc(){

	init_huffman_table();

	bits_Y = calloc(height_down_Y,sizeof(unsigned char *));

	for (int i=0;i<height_down_Y;i++) {
		bits_Y[i]=calloc(width_down_Y, sizeof (unsigned char));
	}

	bits_U = calloc(height_down_UV,sizeof(unsigned char *));

	for (int i=0;i<height_down_UV;i++) {
		bits_U[i]=calloc(width_down_UV, sizeof (unsigned char));
	}

	bits_V = calloc(height_down_UV,sizeof(unsigned char *));

	for (int i=0;i<height_down_UV;i++) {
		bits_V[i]=calloc(width_down_UV, sizeof (unsigned char));
	}
} //Los pongo a cero porque voy a usar máscaras para colocar los bits

int entropic_enc(unsigned char **hops, unsigned char **bits, int line) {

	unsigned int counterh0 = 0;
	unsigned char mode = 0; //0=huffman, 1=rlc
	unsigned char mode_prev = mode;
	unsigned char rlc_length_ini = 2;
	unsigned char condition_length_ini = 7;
	unsigned int bytes = 0;
	unsigned char mask = 0b10000000;

	rlc_length = rlc_length_ini;
	condition_length = condition_length_ini;

	for (int i = 0; i < 20; i++) {//Hay que cambiar el 20 por el ancho de la imagen
		printf("Simbolo: %u\n", hops[line][i]);
		if (hops[line][i]==4){
			counterh0+=1;
			if (mode==0 && counterh0==condition_length) {
				printf("Entra por primera vez en modo RLC, pone el bit que corresponde a 1, cambia de modo y el contador a cero\n");
				mode=1; 
				counterh0=0;
				bits[line][bytes] = bits[line][bytes] | mask;
				mask = mask >> 1;
				if (mask == 0) {bytes++; mask=128;}
			}
		} else {
			mode=0;
		}

		if (mode==1 && counterh0==(unsigned char)(pow(2, rlc_length)-1)) {
			counterh0=0;
			rlc_length=3;
			bits[line][bytes] = bits[line][bytes] | mask;
			mask = mask >> 1;
			if (mask == 0) {bytes++; mask=128;}
			printf("Se ha alcanzado la longitud maxima de RLC, se añade un 1 a los bits\n");
		}

		if (mode_prev==1 && mode==0) {
			for (int b = rlc_length-1; b >= 0; b--){
				bits[line][bytes] = bits[line][bytes] | (testBit(counterh0, b) * mask);
				printf("Byte: %u, %u\n", bits[line][bytes], bytes);
				printf("Mascara: %u\n", mask);
				mask = mask >> 1;
				if (mask == 0) {bytes++; mask=128;}
			}
			printf("Ha salido del modo RLC y se come el primer cero\n");
			condition_length=condition_length_ini;
			rlc_length=rlc_length_ini;
			counterh0=0;
			for (int b = 6; b >= 0; b--){
				//printf("%u\n", huff_table[hops[line][i]]);
				bits[line][bytes] = bits[line][bytes] | (testBit(huff_table[hops[line][i]], b) * mask);
				printf("Byte: %u, %u\n", bits[line][bytes], bytes);
				printf("Mascara: %u\n", mask);
				mask = mask >> 1;
				if (mask == 0) {bytes++; mask=128;}
				if (testBit(huff_table[hops[line][i]], b) == 1) {
					break;
				}
			}
		} else if (mode == 0 && mode_prev == 0) {
			printf("Funcionamiento normal de huffman\n");
			for (int b = 7; b >= 0; b--){
				//printf("%u\n", huff_table[hops[line][i]]);
				bits[line][bytes] = bits[line][bytes] | (testBit(huff_table[hops[line][i]], b) * mask);
				printf("Byte: %u, %u\n", bits[line][bytes], bytes);
				printf("Mascara: %u\n", mask);
				mask = mask >> 1;
				if (mask == 0) {bytes++; mask=128;}
				if (testBit(huff_table[hops[line][i]], b) == 1) {
					break;
				}
			}
		} else {

		}

		mode_prev = mode;


	}
	return bytes;

}

int main(int argc, char* argv[]) {

	//struct timeval t_ini, t_fin;
	//double secs;

	//gettimeofday(&t_ini, NULL);

	printf("Iniciando el coder entropico\n");
	init_entropic_enc();
	printf("Coder entropico inicializado. Se pasa a iniciar hops\n");
	unsigned char **hops;
	hops = malloc(2*sizeof(unsigned char *));
	for (int i = 0; i < 2; i++) {
		hops[i]=calloc(20, 1);
	}
	hops[0][0] = 5;
	hops[0][1] = 6;
	hops[0][2] = 4;
	hops[0][3] = 4;
	hops[0][4] = 4;
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
	unsigned char **bits;
	bits = malloc(2*sizeof(unsigned char*));
	for (int i = 0; i < 2; i++) {
		bits[i]=malloc(20*sizeof(unsigned char));
	}
	printf("Bits creado, se pasa a ejecutar el codificador.\n");
	int bytes = entropic_enc(hops, bits, 0);

	for (int j = 0; j <= bytes; j++){
		for (int i = 7; i >= 0; i--){
			printf("%u", testBit(bits[0][j], i));
		}
		//printf("\n");
	}
	printf("\n");
	//gettimeofday(&t_fin, NULL);

	//secs = timeval_diff(&t_fin, &t_ini)/10000;
	//printf("%.16g ms\n", secs * 1000.0);

	return 0;
}
