
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
#include "include/camera_reader.h"
#include "include/video_encoder_simd.h"
#include "include/streamer.h"


#define min(a, b) (((a) < (b)) ? (a) : (b))
#define max(a, b) (((a) > (b)) ? (a) : (b))



void init_videoencoder(){




//tinfo = calloc(num_threads, sizeof(struct thread_info));
tinfo = calloc(num_threads, sizeof(struct thread_info));

//inicializacion del puntero frame_encoded, donde se guardará la señal de lo que se cuantiza
frame_encoded_Y=result_Y;
frame_encoded_U=result_U;
frame_encoded_V=result_V;

//aloca delta

delta_Y = malloc(height_down_Y*sizeof(unsigned char *));

	for (int i=0;i<height_down_Y;i++) {
		delta_Y[i]=malloc(width_down_Y*sizeof (unsigned char));
	}

	delta_U = malloc(height_down_UV*sizeof(unsigned char *));

	for (int i=0;i<height_down_UV;i++) {
		delta_U[i]=malloc(width_down_UV*sizeof (unsigned char));
	}

	delta_V = malloc(height_down_UV*sizeof(unsigned char *));

	for (int i=0;i<height_down_UV;i++) {
		delta_V[i]=malloc(width_down_UV*sizeof (unsigned char));
	}

tam_bits_Y = malloc(height_down_Y*sizeof(int));
tam_bits_U = malloc(height_down_UV*sizeof(int));
tam_bits_V = malloc(height_down_UV*sizeof(int));



}


void init_camera_video()
{
 CAMERA_OPTIONS options;
    MMAL_COMPONENT_T *camera;
    int status, y;
    options.width = 640;//640;//1280;
    options.height = 480;//480;//720;
    options.framerate = 30;
    options.cameraNum = 0;
    options.sensor_mode = 6;//7;//6
    DEBUG = false;
    yuv_model = 2; // 4:2:0

    camera = init_camera(&options);

}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void suma_delta_scanline(int y, int width,unsigned char ** last_frame_player, unsigned char ** delta_scan, unsigned char ** frame_encoded)
{
//suma last_frame_player+ frame_encoded
int delta, signo,tramo1,tramo2;
int image;

tramo1 = 52;//115;
tramo2 = 204;//140;

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

//            last_frame_player[y][x] = image;
            frame_encoded[y][x] = image;


        }// for x



}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void suma_delta(){

for (int y=0;y<height_down_Y;y++){
  suma_delta_scanline(y, width_down_Y, last_frame_player_Y,frame_encoded_Y, frame_encoded_Y);
}

for (int y=0;y<height_down_UV;y++){
  suma_delta_scanline(y, width_down_UV, last_frame_player_U, frame_encoded_U, frame_encoded_U);
  suma_delta_scanline(y, width_down_UV, last_frame_player_V, frame_encoded_V, frame_encoded_V);


}


}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void compute_delta(){
if (DEBUG)printf ("enter in compute_delta() \n");
for (int y=0;y<height_down_Y;y++){
  compute_delta_scanline(y, width_down_Y, orig_down_Y, last_frame_player_Y,delta_Y);
}
if (DEBUG)printf (" Y ok\n");
for (int y=0;y<height_down_UV;y++){
  compute_delta_scanline(y, width_down_UV, orig_down_U, last_frame_player_U, delta_U);
  compute_delta_scanline(y, width_down_UV, orig_down_V, last_frame_player_V, delta_V);


}
if (DEBUG)if (DEBUG) printf ("exit from compute_delta() \n");

}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void compute_delta_scanline(int y, int width, unsigned char ** orig_down, unsigned char ** last_frame_player, unsigned char ** delta){
/// this function computes delta image for diferential video


//calcula delta=orig_down-last_frame_player
int delta_int, signo,tramo1,tramo2;

tramo1 =52;//115;//52;
tramo2 =204;//140;// 204;

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
				delta_int = tramo1 + (delta_int>>2);
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
void compute_delta_scanline_simd(int y, int width, unsigned char ** orig_down, unsigned char ** last_frame_player, unsigned char ** delta)
{
    for (int x = 0; x < width; x+=8)
    {
        if( x> width-8)
        {
            x= width-8;
        }
        _compute_delta_simd( orig_down[y]+x, last_frame_player[y]+x, delta[y]+x);

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
if (DEBUG) printf ("enter in shift_frame %d %d %d %d \n",dx,dy,height_orig_UV, width_orig_UV);
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
if (DEBUG) printf ("kk \n");

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
  struct timeval t_ini, t_fin;
  double secs;


    downsampler_initialized=false;
    quantizer_initialized=false;


bool camera=true;


if (camera)
{

init_camera_video();
}
else{
 //carga la imagen lena

load_frame("../LHE_Pi/img/lena.bmp");
rgb2yuv(rgb,rgb_channels);
}




if (DEBUG) printf("frame loaded  \n");

//int encoder
pppx=2;
pppy=2;

init_framecoder(width_orig_Y,height_orig_Y,pppx,pppy);
init_videoencoder();



if (DEBUG) printf("encoder initialized  \n");

char buffer[100];

// bucle infinito de movimiento
// -----------------------------
int total_frames=1;//1000;
int total_bits=0;
for (int i=0 ; i<total_frames;i++){


  //desplazamos la imagen original
  // ------------------------------



  if (camera)
  {
  pthread_mutex_lock (&cam_down_mutex);
  pthread_cond_wait (&cam_down_cv,&cam_down_mutex);
  }
  else
  {shift_frame(4,4);
  }

  if (DEBUG) printf(" frame preparado %02d \n",i);

  //salvamos el fotograma original
  // -------------------------------
  //if (DEBUG)
  if (DEBUG) sprintf(buffer,"../LHE_Pi/video/lena%02d.bmp",i);
  //if (DEBUG)
  if (DEBUG) save_frame(buffer, width_orig_Y, height_orig_Y, 3, orig_Y,orig_U,orig_V,420);

  //downsampling del frame original
  // ------------------------------
  if (DEBUG) gettimeofday(&t_ini, NULL);
  // esta funcion hace downsampling sin SIMD
  //downsample_frame(pppx,pppy);
  downsample_frame_simd(pppx,pppy);


  if (camera) pthread_mutex_unlock (&cam_down_mutex);


  if (DEBUG) gettimeofday(&t_fin, NULL);
  if (DEBUG) secs = timeval_diff(&t_fin, &t_ini);
  if (DEBUG) printf(" downsampling: %.16g ms\n", secs * 1000.0);
  if (DEBUG) sprintf(buffer,"../LHE_Pi/video/lena_down%02d.bmp",i);
  if (DEBUG) save_frame(buffer, width_down_Y, height_down_Y, 3, orig_down_Y,orig_down_U,orig_down_V,420);

  //if (camera) pthread_mutex_unlock (&cam_down_mutex);

  //identificamos el target a codificar
  // -----------------------------------
  if (DEBUG) printf(" selecting target \n",i);
  gop_size=0;//1000;

  if (gop_size==0 || i%gop_size==0) {
    // frame I
    if (DEBUG) printf ("frame I \n");
    target_Y=orig_down_Y;
    target_U=orig_down_U;
    target_V=orig_down_V;

  }
  else {
    //frame P.
    if (DEBUG) printf ("frame P \n");
    if (DEBUG) gettimeofday(&t_ini, NULL);
    compute_delta();
    if (DEBUG) gettimeofday(&t_fin, NULL);
    secs = timeval_diff(&t_fin, &t_ini);
    if (DEBUG) printf(" delta computation: %.16g ms\n", secs * 1000.0);

    target_Y=delta_Y;
    target_U=delta_U;
    target_V=delta_V;
  }
  if (DEBUG) sprintf(buffer,"../LHE_Pi/video/lena_target%02d.bmp",i);
  if (DEBUG) save_frame(buffer, width_down_Y, height_down_Y, 3, target_Y,target_U,target_V,420);


  //ahora codificamos el target
  // ------------------------------
  if (DEBUG) printf(" cuantizando... \n",i);
  if (DEBUG) gettimeofday(&t_ini, NULL);
    // esta funcion hace las scanlines de arriba a abajo
    //   quantize_target_normal(frame_encoded_Y,frame_encoded_U,frame_encoded_V);
    // esta funcion cuantiza en orden salteado , robusto a perdidas
    //for (int k=0 ;k<1000;k++){
      quantize_target(frame_encoded_Y,frame_encoded_U,frame_encoded_V);
    //}

   for (int i=0;i<num_threads;i++)
   {
   //pthread_cond_wait (&cam_down_cv,&cam_down_mutex);
    pthread_mutex_lock(&th_done[i]);
    pthread_mutex_unlock(&th_done[i]);
    }

for (int i=0; i< num_threads;i++)
{
pthread_join(thread[i], NULL);
}
  if (DEBUG) gettimeofday(&t_fin, NULL);
  if (DEBUG) gettimeofday(&t_fin, NULL);
  secs = timeval_diff(&t_fin, &t_ini);
  if (DEBUG) printf(" LHE quantization: %.16g ms\n", secs * 1000.0);


   //loss_packets();



  //if (DEBUG)
  sprintf(buffer,"../LHE_Pi/video/result_video/frame_quant%02d.bmp",i);

  //if (DEBUG)
  save_frame(buffer, width_down_Y, height_down_Y, 3, frame_encoded_Y,frame_encoded_U,frame_encoded_V,420);

  // ahora entra el entropico para codificar los hops
  // -----------------------------------------------
  if (DEBUG) gettimeofday(&t_ini, NULL);

  //esto se hace ya en la fase de cuantizacion
  //total_bits+=entropic_enc_frame_normal();
  for (int i=0;i<num_threads;i++)
  {
  total_bits+=tinfo[i].bits_count;
  }
  if (DEBUG) gettimeofday(&t_fin, NULL);
  if (DEBUG) secs = timeval_diff(&t_fin, &t_ini);
  if (DEBUG) printf(" entropic encoding: %.16g ms\n", secs * 1000.0);



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
    if (DEBUG) gettimeofday(&t_ini, NULL);
    suma_delta();
    if (DEBUG) gettimeofday(&t_fin, NULL);
    secs = timeval_diff(&t_fin, &t_ini);
    if (DEBUG) printf(" delta sumation: %.16g ms\n", secs * 1000.0);
  }
if (DEBUG) printf(" calculando player image \n",i);


  //conmutamos el frame_encoded para el siguiente frame
  // --------------------------------------------------
  if (frame_encoded_Y==result_Y){
    frame_encoded_Y=result2_Y;
    frame_encoded_U=result2_U;
    frame_encoded_V=result2_V;

    last_frame_player_Y=result_Y;
    last_frame_player_U=result_U;
    last_frame_player_V=result_V;

    }else {
    frame_encoded_Y=result_Y;
    frame_encoded_U=result_U;
    frame_encoded_V=result_V;

    last_frame_player_Y=result2_Y;
    last_frame_player_U=result2_U;
    last_frame_player_V=result2_V;

  }
if (DEBUG) sprintf(buffer,"../LHE_Pi/video/lena_player_down%02d.bmp",i);
 if (DEBUG)  save_frame(buffer, width_down_Y, height_down_Y, 3, last_frame_player_Y,last_frame_player_U,last_frame_player_V,420);



if (DEBUG) printf(" escalando...\n",i);
  //expansion del last_frame_player. solo experimental. no se hace en encoder
  int umbral=38;
  if (pppy==1) {
  scale_epx_H2(last_frame_player_Y,height_down_Y,width_down_Y,scaled_Y,umbral);
  scale_epx_H2(last_frame_player_U,height_down_UV,width_down_UV,scaled_U,umbral);
  scale_epx_H2(last_frame_player_V,height_down_UV,width_down_UV,scaled_V,umbral);
  }
  else {
  if (DEBUG) printf(" escalando epx 22...\n",i);
  scale_epx(last_frame_player_Y,height_down_Y,width_down_Y,scaled_Y,umbral);
  scale_epx(last_frame_player_U,height_down_UV,width_down_UV,scaled_U,umbral);
  scale_epx(last_frame_player_V,height_down_UV,width_down_UV,scaled_V,umbral);


  }
if (DEBUG) printf(" escaled ok \n",i);


  //if (DEBUG)
  sprintf(buffer,"../LHE_Pi/video/result_video/frame_scaled_bin%02d.bmp",i);
  //if (DEBUG)
  save_frame(buffer, width_orig_Y, height_orig_Y, 3, scaled_Y,scaled_U,scaled_V,420);

    double psnr2;
  if (DEBUG) psnr2= get_PSNR_Y(scaled_Y,orig_Y, height_orig_Y,width_orig_Y);
  if (DEBUG) printf("psnr scaled: %2.2f dB\n ",(float)psnr2);

  stream_frame();

//if (camera) pthread_mutex_unlock (&cam_down_mutex);





}//end for frames

float bpp=(float)total_bits/(total_frames*width_orig_Y*height_orig_Y);
if (DEBUG) printf(" bpp= %f \n",bpp);
float bitrate=(30.0*(float)total_bits)/total_frames;
if (DEBUG) printf(" bitrate= %f \n",bitrate);


}


void loss_packets()
{

if (DEBUG) printf ("packet loss...\n");
float perdidas=0.2f;//1f;

int num_lost_lines=(int)(perdidas*height_down_Y);
bool flag= true;//false;//true;
int line=0;
int victims_Y[height_down_Y];
for (int i = 0; i < height_down_Y; i++){
    victims_Y[i] = 0;
}

int victims_UV[height_down_UV];
for (int i = 0; i < height_down_UV; i++){
    victims_UV[i] = 0;
}

for (int i=0;i<num_lost_lines;i++)
{

  line=rand() % height_down_Y;
  while (line<2 || line>height_down_Y-4)
    line=rand() % height_down_Y;

  victims_Y[line] = 1;

  for (int x=0;x<width_orig_Y;x++){

  frame_encoded_Y[line][x]=0;
  //if (flag) frame_encoded_Y[line][x]=(frame_encoded_Y[line-1][x]+frame_encoded_Y[line+1][x])/2;
  }

  for (int x=0;x<width_orig_UV;x++){
  frame_encoded_U[line/2][x]=0;
  frame_encoded_V[line/2][x]=0;
  victims_UV[line/2] = 1;

  //if (flag) frame_encoded_U[line/2][x]=(frame_encoded_U[line/2-1][x]+frame_encoded_U[line/2+1][x])/2;
  //if (flag)  frame_encoded_V[line/2][x]=(frame_encoded_V[line/2-1][x]+frame_encoded_V[line/2+1][x])/2;
  }
  }



for (int i = 0; i < height_down_Y; i++){

    int superior = i - 1;
    int inferior = i + 1;
    if (victims_Y[i] == 0) continue;
    else {
        int j = i-1;
        while(victims_Y[j] == 1 && j > 0) j--;
        superior = j;
        j = i+1;
        while(victims_Y[j]==1 && j < height_down_Y) j++;
        inferior = j;
    }

    if (superior > -1 && inferior < height_down_Y) {
        for (int x=0;x<width_orig_Y;x++){
            if (flag) frame_encoded_Y[i][x]=(frame_encoded_Y[superior][x]+frame_encoded_Y[inferior][x])/2;
        }
    } else if (superior > -1) {
        for (int x=0;x<width_orig_Y;x++){
            if (flag) frame_encoded_Y[i][x]=frame_encoded_Y[superior][x];
        }
    } else {
        for (int x=0;x<width_orig_Y;x++){
            if (flag) frame_encoded_Y[i][x]=frame_encoded_Y[inferior][x];
        }
    }
}

for (int i = 0; i < height_down_UV; i++){

    int superior = i - 1;
    int inferior = i + 1;
    if (victims_UV[i] == 0) continue;
    else {
        int j = i-1;
        while(victims_UV[j] == 1 && j > 0) j--;
        superior = j;
        j = i+1;
        while(victims_UV[j]==1 && j < height_down_UV) j++;
        inferior = j;
    }

    if (superior > -1 && inferior < height_down_UV) {
        for (int x=0;x<width_orig_UV;x++){
            if (flag) frame_encoded_U[i][x]=(frame_encoded_U[superior][x]+frame_encoded_U[inferior][x])/2;
            if (flag) frame_encoded_V[i][x]=(frame_encoded_V[superior][x]+frame_encoded_V[inferior][x])/2;
        }
    } else if (superior > -1) {
        for (int x=0;x<width_orig_UV;x++){
            if (flag) frame_encoded_U[i][x]=frame_encoded_U[superior][x];
            if (flag) frame_encoded_V[i][x]=frame_encoded_V[superior][x];
        }
    } else {
        for (int x=0;x<width_orig_UV;x++){
            if (flag) frame_encoded_V[i][x]=frame_encoded_V[inferior][x];
            if (flag) frame_encoded_U[i][x]=frame_encoded_U[inferior][x];
        }
    }
}

int count = 0;
for (int i = 0; i < height_down_Y; i++){
    if(victims_Y[i] == 1) count++;
}
//printf("LLLLLLLLLLLLLLLLLLLLLLcontador de perdidas: %d\n", (count*100)/height_down_Y);
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
