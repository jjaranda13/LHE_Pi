#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include <limits.h>
#include <stdint.h>
#include "include/globals.h"
#include "include/entropic_enc.h"

#define testBit(A,k) ((A & (1UL<<(k)))>>k)

void init_entropic_enc(){

	bits_Y = malloc(height_down_Y*sizeof(uint8_t *));

	for (int i=0;i<height_down_Y;i++) {
		bits_Y[i]=malloc(width_down_Y*2);// 2 porque el simbolo 0 ocupa 9 bits (=1.1 byte)
	}

	bits_U = malloc(height_down_UV*sizeof(uint8_t *));

	for (int i=0;i<height_down_UV;i++) {
		bits_U[i]=malloc(width_down_UV*2);
	}

	bits_V = malloc(height_down_UV*sizeof(uint8_t *));

	for (int i=0;i<height_down_UV;i++) {
		bits_V[i]=malloc(width_down_UV*2);
	}

    tam_bits_Y = malloc(height_down_Y*sizeof(int));
    tam_bits_U = malloc(height_down_UV*sizeof(int));
    tam_bits_V = malloc(height_down_UV*sizeof(int));

	//tam_bytes_Y=malloc(height_down_Y*sizeof(unsigned char *));//Para inicializar esta variable que se usa en el streamer
} //Los pongo a cero porque voy a usar máscaras para colocar los bits

//#define LENGTH 32
#define LENGTH sizeof(mask)*CHAR_BIT
#define HUFFMAN 0
#define RLC1    1
#define RLC2    2
int prueba = 0;









#ifndef AV_WB32
#   define AV_WB32(p, val) do {                 \
        uint32_t d = (val);                     \
        ((uint8_t*)(p))[3] = (d);               \
        ((uint8_t*)(p))[2] = (d)>>8;            \
        ((uint8_t*)(p))[1] = (d)>>16;           \
        ((uint8_t*)(p))[0] = (d)>>24;           \
    } while(0)
#endif

/**
 * Initialize the PutBitContext s.
 *
 * @param buffer the buffer where to put bits
 * @param buffer_size the size in bytes of buffer
 */
static inline void init_put_bits(PutBitContext *s, uint8_t *buffer,
                                 int buffer_size)
{
    if (buffer_size < 0) {
        buffer_size = 0;
        buffer      = NULL;
    }

    s->size_in_bits = 9 * buffer_size; // con 9 deberia valer pues nuestro simbolo mas largo mide 9 bits
    s->buf          = buffer;
    s->buf_end      = s->buf + buffer_size*2;
    s->buf_ptr      = s->buf;
    s->bit_left     = 32;
    s->bit_buf      = 0;
}

/**
 * @return the total number of bits written to the bitstream.
 */
static inline int put_bits_count(PutBitContext *s)
{
    return (s->buf_ptr - s->buf) * 8 +32 - s->bit_left;
}


/**
 * Write up to 31 bits into a bitstream.
 * Use put_bits32 to write 32 bits.
 */
static inline void put_bits(PutBitContext *s, int n, unsigned int value)
{
    unsigned int bit_buf;
    int bit_left;

    if (n <= 31 && value < (1U << n)){}
    else {
        printf("Error!!!!!!!! n > 31\n");
        abort();
    }

    bit_buf  = s->bit_buf;
    bit_left = s->bit_left;

    if (n < bit_left) {
        bit_buf     = (bit_buf << n) | value;
        bit_left   -= n;
    } else {
        bit_buf   <<= bit_left;
        bit_buf    |= value >> (n - bit_left);
        if (3 < s->buf_end - s->buf_ptr) {
            AV_WB32(s->buf_ptr, bit_buf);
            s->buf_ptr += 4;
        } else {
            printf("Internal error, put_bits buffer too small\n");
            abort();
        }
        bit_left   += 32 - n;
        bit_buf     = value;
    }

    s->bit_buf  = bit_buf;
    s->bit_left = bit_left;

}

/*
static inline void put_bits_flush(PutBitContext *s)
{
    unsigned int bit_buf;
    int bit_left;

    bit_buf  = s->bit_buf;
    bit_left = s->bit_left;

    if (bit_left==32 )return;

    bit_buf   <<= bit_left;
    if (3 < s->buf_end - s->buf_ptr)
    {
        AV_WB32(s->buf_ptr, bit_buf);
        s->buf_ptr += 4;
    }
    else
    {
        printf("Internal error, put_bits buffer too small\n");
        abort();
    }
}
*/
static inline void flush_put_bits(PutBitContext *s)
{

//printf("Ha llegado a flush\n");
#ifndef BITSTREAM_WRITER_LE
    if (s->bit_left < 32)
       s->bit_buf <<= s->bit_left;
#endif
   while (s->bit_left < 32) {
        /* XXX: should test end of buffer */
#ifdef BITSTREAM_WRITER_LE
        *s->buf_ptr++ = s->bit_buf;
        s->bit_buf  >>= 8;
#else
        *s->buf_ptr++ = s->bit_buf >> 24;
        s->bit_buf  <<= 8;
#endif
        s->bit_left  += 8;
    }
    s->bit_left = 32;
    s->bit_buf  = 0;
}

int entropic_enc(unsigned char **hops, uint8_t **bits, unsigned int line, unsigned int line_width) {

    int xini, yini, xfin_downsampled, yfin_downsampled, pix, dif_pix;
    PutBitContext s;

    int mode = HUFFMAN, h0_counter = 0, hops_counter = 0;
    int condition_length = 7;
    int rlc_length = 4;



    uint8_t hop = 0;
    //uint8_t number[9] = { 0,1,1,1,1,1,1,1,1 };
    //uint8_t longi[9] = { 8,7,5,3,1,2,4,6,8 };

    uint8_t number[9] = { 1,1,1,1,1,1,1,1,1 };
    uint8_t longi[9] = { 9,7,5,3,1,2,4,6,8 };


    init_put_bits(&s, bits[line], line_width);


    for (int x = 0; x < line_width;) {

        hop = hops[line][x];
        if (hop == 4) h0_counter++;

        switch(mode){
            case HUFFMAN:
                put_bits(&s, longi[hop], number[hop]);
                if(hop != 4) h0_counter = 0;
                if (h0_counter == condition_length) {
                    mode = RLC1;
                    h0_counter = 0;
                }
            break;
            case RLC1:
                if (hop == 4 && h0_counter == 15) {
                    put_bits(&s, 1, 1);
                    mode = RLC2;
                    rlc_length++;
                    h0_counter = 0;
                } else if (hop != 4) {
                    put_bits(&s, rlc_length+1, h0_counter);
                    put_bits(&s, longi[hop]-1, number[hop]);
                    h0_counter = 0;
                    mode = HUFFMAN;
                }
            break;
            case RLC2:
                if (hop == 4 && h0_counter == 31) {
                    put_bits(&s, 1, 1);
                    h0_counter = 0;
                } else if (hop != 4) {
                    put_bits(&s, rlc_length+1, h0_counter);
                    put_bits(&s, longi[hop]-1, number[hop]);
                    rlc_length = 4;
                    h0_counter = 0;
                    mode = HUFFMAN;
                }
            break;
        }
        if (hop >= 5 || hop <= 3)
            x++;
        else
            x = (x + 2) & ~(1);
    }

    if (h0_counter != 0 && mode != HUFFMAN) put_bits(&s, rlc_length+1, h0_counter);
    //put_bits_flush(&s);
    flush_put_bits(&s);
    return put_bits_count(&s);

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
