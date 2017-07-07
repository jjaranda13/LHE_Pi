/**
 * @file quantizer.c
 * @author JJGA
 * @date July 2017
 * @brief quantize one scanline using LHE.
 *
 * This module reads the image signal (Y,U,V) and transform one scanline into hops.
 *
 * @see https://github.com/jjaranda13/LHE_Pi
 */

#include "main.h"
#include "quantizer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>

// la cache realmente podria ser de 10KB cache_hops[255][7][6]; e incluso de la mitad (5KB) pues es simetrica
unsigned char cache_hops[255][7][6]; //10KB cache
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void init_quantizer(){
//this function pre computes the cache
//la cache es cache[Y][h1][hop] = 10.000 bytes = 10 KB

for (int hop0=0;hop0<=255;hop0++)
{
 for (int hop1=4; hop1<=10;hop1++)
 {
 //ratio for possitive hops
 double rpos = pow(0.8f*(255-hop0)/hop1,1.0f/3.0f);

 //ratio for negative hops
 double rneg = pow(0.8f*(hop0)/hop1,1.0f/3.0f);

 //compute hops 0,1,2,6,7,8
 //hops 3,4,5 are known
 cache_hops[hop0][hop1-4][0] = (unsigned char)(hop0-hop1*rneg*rneg*rneg);
 cache_hops[hop0][hop1-4][1] = (unsigned char)(hop0-hop1*rneg*rneg);
 cache_hops[hop0][hop1-4][2] = (unsigned char)(hop0-hop1*rneg);
 cache_hops[hop0][hop1-4][3] = (unsigned char)(hop0+hop1*rpos);
 cache_hops[hop0][hop1-4][4] = (unsigned char) (hop0+hop1*rpos*rpos);
 cache_hops[hop0][hop1-4][5] = (unsigned char)(hop0+hop1*rpos*rpos*rpos);

 }//for hop1
}//for hop0
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void quantize_scanline(char **image, int line,int width, char **hops) {
//this function quantize the luminances or chrominances of one scanline
// inputs : image, line, width,
// outputs: hops

char max_hop1=10;
char min_hop1=4;


char start_hop1=(max_hop1+min_hop1)<<1;


// to do

}
