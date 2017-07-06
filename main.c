#include "main.h"
#include "camera_reader.h"
#include "downsampler.h"
#include "entropic_enc.h"
#include "quantizer.h"
#include "streamer.h"
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>

/*Función auxiliar para medir tiempos de ejecución*/
double timeval_diff(struct timeval *a, struct timeval *b) {
	return ((double)(a->tv_sec +(double)a->tv_usec/1000000)-(double)(b->tv_sec + (double)b->tv_usec/1000000));
}

/*Función de inicialización del espacio de memoria y variables globales*/

//Esta función tiene que ser llamada por la función que inicia el proceso de captura de la cámara, pasándole el tamaño de la imagen
//tanto en ancho como en largo para reservar la memoria necesaria para almacenar cada una de las líneas por separado,

static int coder_init(int width, int height) {

    pppx = 2;
    pppy = 2;
	scanlines = calloc((height/pppy),sizeof(char *));
	for (int i=0; i < height/pppy; i++) {
        scanlines[i] = calloc (width/pppx,sizeof(char));
    }

    return 0;
}

int main(int argc, char* argv[]) {

	struct timeval t_ini, t_fin;
	double secs;

	gettimeofday(&t_ini, NULL);
	coder_init(640, 480);
	gettimeofday(&t_fin, NULL);

    secs = timeval_diff(&t_fin, &t_ini);
	printf("%.16g ms\n", secs * 1000.0);
	printf("%hhu\n", scanlines[0][50]);

    for (int i=0;i<640/2;i++){
     for (int j=0;j<480/2;j++)
     {
      scanlines[j][i]=128;
     }
    }

    secs = timeval_diff(&t_fin, &t_ini);
	printf("%.16g ms\n", secs * 1000.0);

    gettimeofday(&t_ini, NULL);
    for (int j = 0; j < 10000; j++){
        for (int i=0;i<480/2;i++){
        down_avg_horiz(scanlines,i);
        }
    }
	gettimeofday(&t_fin, NULL);



	secs = timeval_diff(&t_fin, &t_ini)/10000;
	printf("%.16g ms\n", secs * 1000.0);

	return 0;
}
