#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>

/**
 * @file streamer.h
 * @author JRD
 * @date Jul 2017
 * @brief image utilities.
 *
 * This module makes udp socket for exchange of bits between raspberry and pc
 *
 * @see https://github.com/jjaranda13/LHE_Pi
 */

#include <time.h>
#include <unistd.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <sys/un.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>

#include "include/globals.h"
#include "include/streamer.h"


/*Leemos de un socket, los parámetros son:
-Descriptor del socket
-Estructura remoto: en la que se devolveran los datos del que ha enviado el mensaje que Leemos
-long remoto: De la estructura anterior
-Buffer de taos donde se quiere que aparezca el mensaje
-Longitud de datos del buffer anterior
Devuelve el numero de bytes leidos o -1 en caso de error.*/

int read_socket(int fd, struct sockaddr *remote, socklen_t *long_remote, unsigned long int * data, int long_data){
  int leido=0;
  int aux = 0;
  //Check si los datos son correctos
  if((fd == -1)||(data == NULL)||(long_data < 1)||(remote == NULL)||(long_remote == NULL)){
    return -1;
  }
  /*Mientras no hayamos todos los datos solicitados*/
  while (leido < long_data){
    aux = recvfrom(fd, data+leido, long_data - leido, 0, remote, long_remote);
	//printf("aux me da: %d\n",aux);

    if (aux > 0){
      /*aumentamos leido con lo que hemos leido*/
      leido= leido + aux;
    }
    else {
      /*Si el recvfrom devuelve 0, es que se ha cerrado el socket. Devolvemos lo leido hasta el momento*/
      if (aux == 0){
        return leido;
      }
      if (aux == -1){
        /*En caso de error, tenemos el error en errno.
        EINTR -> Si ha habido alguna interrupción del sistema antes de leer algun dato
        EGAIN -> El socket no esta disponible de momento
        En ambos casos se espera 100microseg y se vuelve a intentar
        Si es otro error nos salimos*/
        switch (errno){
          case EINTR:
          case EAGAIN:
            usleep(100);
            break;
          default:
            return -1;
        }
      }
    }
  }
  /*devolvemos el total de caracteres leidos*/
  return leido;
}

/**
Escribe dato en el socket cliente.
- fd es el descriptor del socket.
- remote es el destinatario del mensaje, a quien se lo queremos enviar.
- Longitud_Remoto es el tamaño de Remoto en bytes.
- data es el mensaje que queremos enviar.
- Longitud_Datos es el tamaño del mensaje en bytes.
Devuelve numero de bytes escritos, o -1 si hay error.*/


int write_socket (int fd, struct sockaddr *remote,	socklen_t long_remote, unsigned long int *data, int long_data){

	int escrito = 0;
	int aux = 0;

	/*Comprobacion de los parametros de entrada	*/
	if ((fd == -1) || (data == NULL) || (long_data < 1)||(remote == NULL)){
		return -1;
   }

	//Bucle hasta que hayamos escrito todos los caracteres que nos han indicado.
	while (escrito < long_data)	{
		aux = sendto (fd, data + escrito, long_data - escrito, 0, remote, long_remote);

    if (aux > 0){
			// Si hemos conseguido escribir caracteres, se actualiza la variable escrito
			escrito = escrito + aux;
		}
		else{
			//Si se ha cerrado el socket, devolvemos el numero de caracteres leidos.
      //Si ha habido error, devolvemos -1
			if (aux == 0)
				return escrito;
			else {
                printf("aux es: %d\n", aux);
				return -1;
         }
		}
	}
	escrito = sendto (fd, data, long_data, 0, remote, long_remote);
  /*Devolvemos el total de caracteres leidos*/
	return escrito;
}

void sendData(){
  //Descriptor del socket y buffer de datos
  int socket_con_server;
  unsigned long int data;

  //Para llamadas a funciones de sockets
  int leidos;
  struct sockaddr_in server; //Direccion del servidor
  socklen_t long_server;     //Tamaño estr. server

  //Abrimos socket
  struct sockaddr_in direccion;
  int descriptor;
  descriptor = socket(AF_INET, SOCK_DGRAM, 0);
  if(descriptor == -1){socket_con_server = -1;}

  /*Rellenamos la estructura de datos para hacer el bind*/
  direccion.sin_family = AF_INET; //Socket inet ipv4
  direccion.sin_addr.s_addr = htonl(INADDR_ANY); //Cualquier dirección ip
  direccion.sin_port = htons(3000);

  /*Hacemos el bind*/
  if(bind(descriptor, (struct sockaddr *)&direccion, sizeof(direccion)) == -1){
    close(descriptor);
    socket_con_server = -1;
  }
  socket_con_server = descriptor;

  if (socket_con_server == -1){
    printf("No puedo establecer conexión con el servidor\n");
    exit(-1);
  }

  /*Ya esta conectado el socket
  Ahora rellenamos la estructura server para enviarle un mensaje*/
  server.sin_family = AF_INET;
  server.sin_port = htons(3000);
  inet_aton("192.168.1.52", &server.sin_addr.s_addr);//La direccion del otro ordenador
  long_server = sizeof(server);

  //Enviar imagen en paquetes de ul
  //int k=0;//Con esto cuento todos los elementos
  int elems_linea;
  for (int i=0;i<height_down_Y;i++){
    elems_linea=tam_bytes_Y[i]/4;
    for(int j=0;j<elems_linea;j++){
        //printf("%d,%d=>%lu\n",i,j,bits_Y[i][j]);
        //Envío losdatos de cada bits_Y
        data=bits_Y[i][j];
        //printf("Escribo %d bytes\n",write_socket(socket_con_server, (struct sockaddr *)&server, long_server, (unsigned long int *)&data, sizeof(data)));
        //write_socket(socket_con_server, (struct sockaddr *)&server, long_server, (unsigned long int *)&data, sizeof(data));
        sendto(socket_con_server, (unsigned long int *)&data, sizeof(data), 0, (struct sockaddr *)&server, long_server );
        //printf("%d\n",k);
        //k++;
    }
  }
  //Enviar imagen en paquetes de 1024 bytes (hay q conservar el k anterior)
  /*int i=0;
  data=bits_Y[0];
  int elems;
  if ((k%256)==0)
        elems=k/256;
  else{
        elems=(k/256)+1;
  }
  while(i<elems){
    write_socket(socket_con_server, (struct sockaddr *)&server, long_server, (unsigned long int *)&data, sizeof(data)*256);
    data=data+256;
    i++;
  }*/

}
