#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include "main.h"
#include "downsampler.h"

/*Función de downsampling de un scanline*/
void down_avg_horiz(unsigned char *imagen_capturada, int line) {
	pppx = 2;
	switch(pppx){
		case 2:

			for (int i=0; i < sizeof(imagen_capturada[line]); i+=2) {
				scanlines[line][i] =  (imagen_capturada[line][i<<1]+imagen_capturada[line][((i+1)<<1)]) >>1; //px[1]+px[2]/2
			}

		default:

		    for (int i=0; i < sizeof(imagen_capturada[line]); i+=pppx) {
		    	scanlines[line][i] = 0;
		    	for (int j=0; j < pppx; j++) {
					scanlines[line][i] += imagen_capturada[line][(i+j)*pppx];
		    	}
	    		scanlines[line][i]/pppx;
			}
	}
}


