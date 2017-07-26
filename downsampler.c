/**
 * @file downsampler.c
 * @author MAG,JJGA
 * @date July 2017
 * @brief downsample one scanline.
 *
 * This module reads the image signal (Y,U,V) and transform one scanline into smaller one.
 *
 * @see https://github.com/jjaranda13/LHE_Pi
 */



#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include "include/globals.h"
#include "include/downsampler.h"

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void init_downsampler()
{
if (DEBUG) printf("ENTER in init_downsampler...\n");
if (DEBUG) printf ("pppx=%d, pppy=%d \n ",pppx,pppy);

width_down_Y=width_orig_Y/pppx;
height_down_Y=height_orig_Y/pppy;

//aqui habria que comprobar el modelo de color con la variable yuv_model
//-----------------------------------------
width_down_UV=width_down_Y/2;
height_down_UV=height_down_Y/2;

//memory allocation
//------------------
orig_down_Y=malloc(height_down_Y*sizeof (unsigned char *));
orig_down_U=malloc(height_down_UV*sizeof (unsigned char *));
orig_down_V=malloc(height_down_UV*sizeof (unsigned char *));

for (int i=0;i<height_down_Y;i++)
  {
  orig_down_Y[i]=malloc(width_down_Y* sizeof (unsigned char));
  }
for (int i=0;i<height_down_UV;i++)
  {
  orig_down_U[i]=malloc(width_down_UV* sizeof (unsigned char));
  orig_down_V[i]=malloc(width_down_UV* sizeof (unsigned char));
  }


down_quant_flag=malloc(height_down_Y* sizeof (unsigned char));

downsampler_initialized=true;
if (DEBUG) printf(" downsampler initialized succesfully \n");

}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
/*Función de downsampling de un scanline*/
void down_avg_horiz(unsigned char **orig,int width_orig, unsigned char** dest,int line,int pppx,int pppy) {
/// this function downsample a single scanline. the result is stored in another scanline
/// depending on pppy (linedown is line/pppy)

if (DEBUG) printf("ENTER in down_avg_horiz...%d \n",line);

int line_down=line/pppy ;//linea correspondiente a la original en la imagen downsampleada (apuntada por *dest)
int pix=0;
//printf ("Linedown= %d",line_down);
int data=0;
	switch(pppx){
		case 2:

			for (int i=0; i < width_orig; i+=2) {
				dest[line_down][pix]=(orig[line][i]+orig[line][i+1]) >>1; //px[1]+px[2]/2
				//data=(orig[line][i]+orig[line][i+1]) >>1;
				//dest[line_down][pix]=orig[line][i];//data;
				pix++;
                //printf("k");
			}
            break;


        case 4:

			for (int i=0; i < width_orig; i+=4) {
				dest[line_down][pix] =  (orig[line][i]+orig[line][i+1]+orig[line][i+2]+orig[line][i+3]) >>2; //px[1]+px[2]/2
                //data=(orig[line][i]+orig[line][i+1]+orig[line][i+2]+orig[line][i+3]) >>2;
                //dest[line_down][pix] = orig[line][i];//data;
				pix++;
				//printf("k");
			}
			break;

		default:

		    for (int i=0; i < width_orig; i+=pppx) {
		    	//dest[line_down][i] = 0;
		    	data=0;
		    	for (int j=0; j < pppx; j++) {
					//dest[line_down][pix] += orig[line][(i+j)];
					data+=orig[line][(i+j)];
		    	}
	    		//dest[line_down][pix]=dest[line_down][i]/pppx;

	    		dest[line_down][pix]=(unsigned char) (data/pppx);
	    		pix++;
	    		//printf("k");
			}
			break;
        }

	if (DEBUG) printf("EXIT in down_avg_horiz...%d \n",line);

}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void close_downsampler()
{

//to do

}
