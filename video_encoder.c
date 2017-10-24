
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

#include <pthread.h>
#include "include/globals.h"

#include "include/imgUtil.h"
#include "include/downsampler.h"
#include "include/quantizer.h"
#include "include/frame_encoder.h"
#include "include/entropic_enc.h"


#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void suma_delta_scanline(int y, int width,unsigned char ** last_frame_player, unsigned char ** delta_scan)
{
//suma last_frame_player+ frame_encoded
int delta, signo,tramo1,tramo2;
int image;

tramo1 = 52;
tramo2 = 204;

  for (int x = 0; x < width; x++) {
  delta=delta_scan[y][x];

  delta = delta-128;
            signo = 0;
            if (delta < 0) {
                signo = 1;
                delta = -delta;
            }

            //if (delta >= tramo2) delta = tramo2-1;

            if (delta < tramo1){
                if (signo == 0) image = last_frame_player[y][x] + delta;
                else image = last_frame_player[y][x]  - delta;
             } else  if (delta <= tramo1+(tramo2-tramo1)/2){
                delta = (delta - tramo1)*2;
                delta += tramo1;
                if (signo == 0) image = last_frame_player[y][x]  + delta;
                else image = last_frame_player[y][x]  - delta;
            } else {
                delta = (delta - (tramo2 - tramo1)/2 - tramo1)*4;
                delta += tramo2;
                if (signo == 0) image = last_frame_player[y][x] + delta;
                else image = last_frame_player[y][x]  - delta;
            }

            if (image > 255)
            {
                image = 255;
            }
            else if (image < 1)
            {
                image = 1;
            }

            last_frame_player[y][x] = image;


        }// for x



}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void suma_delta(){

for (int y=0;y<height_down_Y;y++){
  suma_delta_scanline(y, width_down_Y, last_frame_player_Y,delta_Y);
}

for (int y=0;y<height_down_UV;y++){
  suma_delta_scanline(y, width_down_UV, last_frame_player_U, delta_U);
  suma_delta_scanline(y, width_down_UV, last_frame_player_V, delta_V);


}


}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void compute_delta(){

for (int y=0;y<height_down_Y;y++){
  compute_delta_scanline(y, width_down_Y, orig_down_Y, last_frame_player_Y,delta_Y);
}

for (int y=0;y<height_down_UV;y++){
  compute_delta_scanline(y, width_down_UV, orig_down_U, last_frame_player_U, delta_U);
  compute_delta_scanline(y, width_down_UV, orig_down_V, last_frame_player_V, delta_V);


}


}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void compute_delta_scanline(int y, int width, unsigned char ** orig_down, unsigned char ** last_frame_player, unsigned char ** delta){
/// this function computes delta image for diferential video


//calcula delta=orig_down-last_frame_player
int delta_int, signo,tramo1,tramo2;

tramo1 = 52;
tramo2 = 204;

		for (int x = 0; x < width; x++) {


			delta_int = orig_down[y][x] - last_frame_player[y][x];

			signo = 1;
			if (delta_int < 0) {
				signo = -1;
				delta_int = -delta_int;
			}

			if (delta_int >= tramo2) delta_int = tramo2-1;

			if (delta_int < tramo1) {
				//if (delta_int <= 6) delta_int = 0;
			} else {// if (delta_int < tramo2) {
				delta_int = delta_int - tramo1;
				delta_int = tramo1 + delta_int/2;
			} /*else {
				delta_int = delta_int - tramo2;
				delta_int = dif_tramo + delta_int/4;
				av_log(NULL, AV_LOG_INFO, "3\n");
			}
			*/
			delta_int = signo*delta_int+128;


			delta[y][x] = delta_int;

		}

}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void encode_video_from_file_sequence()
{

//pedir al usuario el directorio de frames

// obtener lista de frame_files ordenada por nombre


//inicializacion del encoder

//codificar primer frame I
  //aqui invocamos a encode_frame_fromfile()


//bucle de procesamiento



}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void shift_frame(int dx, int dy)
{
printf ("enter in shift_frame %d %d %d %d \n",dx,dy,height_orig_UV, width_orig_UV);
for (int y=dy;y<height_orig_Y;y++){
  for (int x=dx;x<width_orig_Y;x++){

  if (y<0) continue;
  if (y>=height_orig_Y) continue;
  if (y-dy<0) continue;
  if (y-dy>=height_orig_Y) continue;

  if (x<0) continue;
  if (x>=width_orig_Y) continue;
  if (x-dx<0) continue;
  if (x-dx>=width_orig_Y) continue;


  orig_Y[y-dy][x-dx]=orig_Y[y][x];
  }
}
printf ("kk \n");

for (int y=dy;y<height_orig_UV;y++){
  for (int x=dx;x<width_orig_UV;x++)
  {

  if (y<0) continue;
  if (y>height_orig_UV) continue;
  if (y-dy<0) continue;
  if (y-dy>height_orig_UV) continue;

  if (x<0) continue;
  if (x>=width_orig_UV) continue;
  if (x-dx<0) continue;
  if (x-dx>=width_orig_UV) continue;

  orig_U[y-dy][x-dx]=orig_U[y][x];
  orig_V[y-dy][x-dx]=orig_V[y][x];
  }
}


}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%


int main(int argc, char* argv[]) {

VideoSimulation();

}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void VideoSimulation()
 {
 //esta funcion simula todo el proceso de captura, down, imagen diferencial, quantization y entropic
 //ojo, esta version es monothread
 //implica los bufferes adicionales: last_frame_player, result1, result2,
 //podremos medir el bitrate con varios tamaños de GOP, incluyendo GOP=1 (solo imagenes I)

  DEBUG=false;
    downsampler_initialized=false;
    quantizer_initialized=false;

 //carga la imagen lena
load_frame("../LHE_Pi/img/lena.bmp");
printf("frame loaded  \n");

//int encoder
pppx=2;
pppy=1;
init_framecoder(width_orig_Y,height_orig_Y,pppx,pppy);
//init_videoencoder(width_orig_Y,height_orig_Y,pppx,pppy);
//incluir en init videoencoder
frame_encoded_Y=result_Y;
frame_encoded_U=result_U;
frame_encoded_V=result_V;

rgb2yuv(rgb,rgb_channels);
printf("encoder initialized  \n");

char buffer[100];

// bucle infinito de movimiento
// -----------------------------
int total_frames=10;
int total_bits=0;
for (int i=1 ; i<total_frames;i++){


  //desplazamos la imagen original
  // ------------------------------
  shift_frame(4,4);
  printf(" shifted %02d \n",i);

  //salvamos el fotograma original
  // -------------------------------
  sprintf(buffer,"../LHE_Pi/video/lena%02d.bmp",i);
  save_frame(buffer, width_orig_Y, height_orig_Y, 1, orig_Y,orig_U,orig_V,420);

  //downsampling del frame original
  // ------------------------------
  downsample_frame(pppx,pppy);
  sprintf(buffer,"../LHE_Pi/video/lena_down%02d.bmp",i);
  save_frame(buffer, width_down_Y, height_down_Y, 1, orig_down_Y,orig_down_U,orig_down_V,420);



  //identificamos el target a codificar
  // -----------------------------------
  printf(" selecting target \n",i);
  gop_size=1000;
  if (gop_size==0 || i%gop_size==0) {
    // frame I
    target_Y=orig_down_Y;
    target_U=orig_down_U;
    target_V=orig_down_V;
  }
  else {
    //frame P.
    compute_delta();
    target_Y=delta_Y;
    target_U=delta_U;
    target_V=delta_V;
  }


  //ahora codificamos el target
  // ------------------------------
  printf(" cuantizando... \n",i);
  quantize_target_normal(frame_encoded_Y,frame_encoded_U,frame_encoded_V);

  printf(" cuantizado ok \n",i);
  sprintf(buffer,"../LHE_Pi/video/lena_quant%02d.bmp",i);
  save_frame(buffer, width_down_Y, height_down_Y, 1, frame_encoded_Y,frame_encoded_U,frame_encoded_V,420);

  // ahora entra el entropico para codificar los hops
  // -----------------------------------------------
  total_bits+=entropic_enc_frame_normal();


  //calculamos la imagen del player
  // ------------------------------
  if (gop_size==0 || i%gop_size==0) {
    // frame I
    last_frame_player_Y=frame_encoded_Y;
    last_frame_player_U=frame_encoded_U;
    last_frame_player_V=frame_encoded_V;
  }
  else {
    //frame P.
   suma_delta();
  }


  //expansion del last_frame_player. solo experimental. no se hace en encoder
  int umbral=38;
  scale_epx_H2(last_frame_player_Y,height_down_Y,width_down_Y,scaled_Y,umbral);
  scale_epx_H2(last_frame_player_U,height_down_UV,width_down_UV,scaled_U,umbral);
  scale_epx_H2(last_frame_player_V,height_down_UV,width_down_UV,scaled_V,umbral);
  sprintf(buffer,"../LHE_Pi/video/lena_scaled%02d.bmp",i);
  save_frame(buffer, width_orig_Y, height_orig_Y, 3, scaled_Y,scaled_U,scaled_V,420);

  //conmutamos el frame_encoded para el siguiente frame
  // --------------------------------------------------
  if (frame_encoded_Y==result_Y){
    frame_encoded_Y=result2_Y;
    frame_encoded_U=result2_U;
    frame_encoded_V=result2_V;
    }else {
    frame_encoded_Y=result_Y;
    frame_encoded_U=result_U;
    frame_encoded_V=result_V;
  }
  double psnr2= get_PSNR_Y(scaled_Y,orig_Y, height_orig_Y,width_orig_Y);
  printf("psnr scaled: %2.2f dB\n ",(float)psnr2);


}//end for frames

float bpp=(float)total_bits/(total_frames*width_orig_Y*height_orig_Y);
printf(" bpp= %f \n",bpp);
float bitrate=(30.0*(float)total_bits)/total_frames;
printf(" bitrate= %f \n",bitrate);


}
/*
    //mueve en circulos la imagen
    frame= mueve_frame(frame, x,y)

    //down(frame)
    esto deja el resultado en orig_down

    //if i% GOP =0 target = orig_down
    conclusion: necesitamos un puntero llamado target que apunta a orig_down

    //else target =calcula imagen diferencial (orig_down, lastframe_player)
    conclusion necesitamos una funcion llamada computedelta() y un buffer llamado last_frame_player
    y un buffer llamado dif_image

    // vamos a tener dos resultados para alternar frames
    // if i%2 result1=lhe(target); last_frame_encoded=result1
    // else result2=lhe(target); last_frame_encoded=result2
    para los hops no necesitamos dos bufferes, solo para las luminancias/crominancias

    despues de cada iteracion podemos hacer if last=result1 then last=result2 else last=result1

    //if i% gop=0 last_frame_player=copy(last_frame_encoded)
    no hariamos una copia, simplemente last_frame_player=last_frame_encoded

    //else  last_frame_player=last_frame_player+last_frame_encoded
    hay que hacer una funcion de suma que de momento estara vacia
    last_frame_player=suma (last_frame_player, last_frame_encoded)

    //save image , incluyendo en el nombre %i

    //entropic: la ponemos para medir el bitrate

    //streamer; esta parte podemos quitarla para la prueba de concepto

 }
*/
