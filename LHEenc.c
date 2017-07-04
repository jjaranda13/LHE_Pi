#include LHEenc.h
#include <stdio.h>
#include <stdlib.h>
#include <vector>
#include <time.h>

//////////////////////////////////////////////
// Esto tiene que ser movido al .h          //
// Lo pongo aqui para que todos sepamos las //
// variables globales y definiciones que    //
// creamos y podamos comentarlas facilmente //
//////////////////////////////////////////////
#define pppx 2
#define pppy 2

vector<uint8_t> scanlines;

/*Función auxiliar para medir tiempos de ejecución*/
double timeval_diff(struct timeval *a, struct timeval *b) {
	return ((double)(a->tv_sec +(double)a->tv_usec/1000000)-(double)(b->tv_sec + (double)b->tv_usec/1000000));
}

/*Función de inicialización del espacio de memoria y variables globales*/







//Esta función tiene que ser llamada por la función que inicia el proceso de captura de la cámara, pasándole el tamaño de la imagen
//tanto en ancho como en largo para reservar la memoria necesaria para almacenar cada una de las líneas por separado, 

static int coder_init(int width, int height) {
	
	scanlines.resize(height/pppy);
	for (int i=0; i < height/pppy; i++) {
        scanlines[i] = malloc (width/pppx);
    }

    return 0;
}

/*Función de captura de la cámara*/






/*Función de downsampling de un scanline*/
static void down_avg_horiz(uint8_t *imagen_capturada, int linea) {

	uint8_t suma = 0;
	for (int i=0; i < imagen_capturada[linea].length()/pppx; i++) {//Esto habra que modificarlo dependiendo de como se almacenen los datos que nos da la camara
		for (int j=0; j < pppx; j++) {
			suma += imagen_capturada[linea][(i*pppx)+j];
		}
		scanlines[linea][i] = suma/pppx;
	}
}

/*Función del encoder*/









/*Función del coder entrópico*/
static void coder_entropico() {

}

/*Función del streamer*/










static int main(int argc, char* argv[]) {

	struct timeval t_ini, t_fin;
	double secs;

	gettimeofday(&t_ini, NULL);
	coder_init(1920, 1080);
	gettimeofday(&t_fin, NULL);

	secs = timeval_diff(&t_fin, &t_ini);
	printf("%.16g ms\n", secs * 1000.0);

	printf(scanlines[0][1]);
	

	return 0;
}