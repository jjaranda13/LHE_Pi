
/**
 * @file video_encoder.c
 * @author JJGA,MAG
 * @date July 2017
 * @brief video coder .
 *
 * description:
 *
 * @see https://github.com/jjaranda13/LHE_Pi
 */


#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <stdbool.h>

#include "include/main.h"
#include "include/quantizer.h"

#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))



//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void compute_delta(unsigned char **frame_new,unsigned char **frame_prev, int y,int width, unsigned char **delta){
/// this function computes delta image for diferential video
/// inputs: frame_new (new frame of video)
///         frame_prev (rebuilt image but not scaled)
/// outputs: delta (delta image)

int interval1=24;
int interval2=186;
int dif_interval=interval2-interval1;
int dif=0;
int sign=1;
for (int x=0;x<width;x++)
  {
  dif=frame_new[y][x]-frame_prev[y][x];
  if (dif<0) {sign=-1;dif=-dif;}

  if (dif<interval1){}
  else if (dif<interval2)
    {
    dif=dif-interval1;
    dif=interval1+(dif>>1);
    }
  else
    {
    dif=dif-interval2;
    dif=interval1+(dif_interval>>1)+dif/3;
    }

  delta[y][x]=sign*dif+128;
  }

}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void VideoSimulation()
{
//esta funcion simula todo el proceso de captura, down, imagen diferencial, quantization y entropic
//ojo, esta version es monothread
//implica los bufferes adicionales: last_frame_player, result1, result2,
//podremos medir el bitrate con varios tamaños de GOP, incluyendo GOP=1 (solo imagenes I)



//carga la imagen lena


//downsample lena


// bucle infinito de movimiento


   //mueve en circulos la imagen down


   //if i% GOP =0 target = imagen down;

   //else target =calcula imagen diferencial (imagedown, lastframe_player)

   // vamos a tener dos resultados para alternar frames
   // if i%2 result1=lhe (target); last_frame_encoded=result1
   //else result2=lhe(target); last_frame_encoded=result2

   //if i% gop=0 last_frame_player=copy(last_frame_encoded)
   //else  last_frame_player=last_frame_player+last_frame_encoded

   //save image , incluyendo en el nombre %i

   //entropic: la ponemos para medir el bitrate

   //streamer; esta parte podemos quitarla para la prueba de concepto

}
