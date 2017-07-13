#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <stdbool.h>
#include "globals.h"
#include "downsampler.h"

/*Función de downsampling de un scanline*/
void down_avg_horiz(char **imagen_capturada, int linea) {
pppx = 2;
switch(pppx){

case 2:



	for (int i=0; i < sizeof(imagen_capturada[linea]); i++) {//Esto habra que modificarlo dependiendo de como se almacenen los datos que nos da la camara
		//for (int j=0; j < pppx; j++) {
		//	suma += imagen_capturada[linea][(i*pppx)+j];
		//}
		//scanlines[linea][i] = suma/pppx;
		scanlines[linea][i] =  (imagen_capturada[linea][i<<1]+imagen_capturada[linea][((i+1)<<1)]) >>1;
		//>> 1;
		//char a=imagen_capturada[linea][i*pppx];
		//scanlines[linea][i] =  (a+a)>>1;
	}

default:

   for (int i=0; i < sizeof(imagen_capturada[linea])/2; i+=2) {//Esto habra que modificarlo dependiendo de como se almacenen los datos que nos da la camara

		scanlines[linea][i] =  (imagen_capturada[linea][i<<1]+imagen_capturada[linea][((i+1)<<1)]) >>1;

	}


}
}


