#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include "globals.h"
#include "downsampler.h"

/*Función de downsampling de un scanline*/
void down_avg_horiz(unsigned char **orig, unsigned char** dest,int line,int pppx) {

	switch(pppx){
		case 2:

			for (int i=0; i < width_orig; i+=2) {
				dest[line][i] =  (orig[line][i]+orig[line][i+1]) >>1; //px[1]+px[2]/2
			}
        case 4:

			for (int i=0; i < width_orig; i+=4) {
				dest[line][i] =  (orig[line][i]+orig[line][i+1]+orig[line][i+2]+orig[line][i+3]) >>2; //px[1]+px[2]/2
			}
		default:

		    for (int i=0; i < width_orig; i+=pppx) {
		    	dest[line][i] = 0;
		    	for (int j=0; j < pppx; j++) {
					dest[line][i] += orig[line][(i+j)];
		    	}
	    		dest[line][i]/pppx;
			}
	}
}


