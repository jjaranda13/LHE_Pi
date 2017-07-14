#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include "globals.h"
#include "downsampler.h"

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void init_downsampler()
{
if (DEBUG) printf("ENTER in init_downsampler...\n");

if (DEBUG) printf ("pppx=%d, pppy=%d \n ",pppx,pppy);
width_down_Y=width_orig/pppy;
height_down_Y=height_orig/pppx;

//aqui hay que comprobar el modelo de color con la variable yuv_model
//-----------------------------------------
width_down_UV=width_down_Y/2;
height_down_UV=height_down_Y/2;

//memory allocation
//------------------
orig_down_Y=malloc(height_down_Y*sizeof (unsigned char *));
orig_down_U=malloc(height_down_UV*sizeof (unsigned char *));
orig_down_V=malloc(height_down_UV*sizeof (unsigned char *));

for (int i=0;i<height_orig;i++)
{
orig_down_Y[i]=malloc(width_down_Y* sizeof (unsigned char));
orig_down_U[i]=malloc(width_down_UV* sizeof (unsigned char));
orig_down_V[i]=malloc(width_down_UV* sizeof (unsigned char));
}



//inicializacion de prueba
int cosa=0;
for (int line=0;line<height_down_Y;line++)
{
for (int x=0;x<width_down_Y;x++)
{
 orig_down_Y[line][x]=1;
 cosa+=orig_down_Y[line][x];
}
}
downsampler_initialized=true;
if (DEBUG) printf(" downsampler initialized succesfully \n");

}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
/*Función de downsampling de un scanline*/
void down_avg_horiz(unsigned char **orig, unsigned char** dest,int line,int pppx,int pppy) {
/// this function downsample a single scanline. the result is stored in another scanline
/// depending on pppy (linedown is line/pppy)

if (DEBUG) printf("ENTER in down_avg_horiz...%d \n",line);

int line_down=line/pppy;
//printf ("Linedown= %d",line_down);
	switch(pppx){
		case 2:

			for (int i=0; i < width_orig; i+=2) {
				//dest[line_down][i]
				printf ("pix= %d \n",i+1);
                char cosa=  (orig[line][i]+orig[line][i+1]) >>1; //px[1]+px[2]/2
			}
        case 4:

			for (int i=0; i < width_orig; i+=4) {
				dest[line_down][i] =  (orig[line][i]+orig[line][i+1]+orig[line][i+2]+orig[line][i+3]) >>2; //px[1]+px[2]/2
			}
		default:

		    for (int i=0; i < width_orig; i+=pppx) {
		    	dest[line_down][i] = 0;
		    	for (int j=0; j < pppx; j++) {
					dest[line_down][i] += orig[line][(i+j)];
		    	}
	    		dest[line_down][i]=dest[line_down][i]/pppx;
			}
	}

	if (DEBUG) printf("EXIT in down_avg_horiz...%d \n",line);

}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
