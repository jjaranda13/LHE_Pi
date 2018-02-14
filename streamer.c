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
int total_frames=0;
int total_time=0;
bool newframe=false;

/*Leemos de un socket, los parámetros son:
-Descriptor del socket
-Estructura remoto: en la que se devolveran los datos del que ha enviado el mensaje que Leemos
-long remoto: De la estructura anterior
-Buffer de taos donde se quiere que aparezca el mensaje
-Longitud de datos del buffer anterior
Devuelve el numero de bytes leidos o -1 en caso de error.*/
/*
int read_socket(int fd, struct sockaddr *remote, socklen_t *long_remote, unsigned long int * data, int long_data){
  int leido=0;
  int aux = 0;
  //Check si los datos son correctos
  if((fd == -1)||(data == NULL)||(long_data < 1)||(remote == NULL)||(long_remote == NULL)){
    return -1;
  }
  //Mientras no hayamos todos los datos solicitados
  while (leido < long_data){
    aux = recvfrom(fd, data+leido, long_data - leido, 0, remote, long_remote);
	//printf("aux me da: %d\n",aux);

    if (aux > 0){
      //aumentamos leido con lo que hemos leido
      leido= leido + aux;
    }
    else {
      //Si el recvfrom devuelve 0, es que se ha cerrado el socket. Devolvemos lo leido hasta el momento
      if (aux == 0){
        return leido;
      }
      if (aux == -1){
        //En caso de error, tenemos el error en errno.
        //EINTR -> Si ha habido alguna interrupción del sistema antes de leer algun dato
        //EGAIN -> El socket no esta disponible de momento
        //En ambos casos se espera 100microseg y se vuelve a intentar
        //Si es otro error nos salimos
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
  //devolvemos el total de caracteres leidos
  return leido;
}
*/
/**
Escribe dato en el socket cliente.
- fd es el descriptor del socket.
- remote es el destinatario del mensaje, a quien se lo queremos enviar.
- Longitud_Remoto es el tamaño de Remoto en bytes.
- data es el mensaje que queremos enviar.
- Longitud_Datos es el tamaño del mensaje en bytes.
Devuelve numero de bytes escritos, o -1 si hay error.*/

/*
int write_socket (int fd, struct sockaddr *remote,	socklen_t long_remote, unsigned long int *data, int long_data){

	int escrito = 0;
	int aux = 0;

	//Comprobacion de los parametros de entrada
	if ((fd == -1) || (data == NULL) || (long_data < 1)||(remote == NULL)){
		return -1;
   }

	//Bucle hasta que hayamos escrito todos los caracteres que nos han indicado.
	while (escrito < long_data)	{
		aux = sendto (fd, data + escrito, long_data - escrito, 0, remote, long_remote);

    if (aux > 0){
			// Si hemos conseguido escribir caracteres, se actualiza la variable escrito
			escrito = escrito + aux;
            printf("Escribo %d bytes\n",aux);
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
  //Devolvemos el total de caracteres leidos
	return escrito;
}
int divup(int dividend, int divisor){//Division entera redondeando hacia arriba
  return (dividend+(divisor / 2) / divisor);
}
*/
/*
void initSocket(){//struct sockaddr_in server, socklen_t long_server, int socket_con_server){
    //Descriptor del socket y buffer de datos
  //int socket_con_server;
  //unsigned long int data;

  //Para llamadas a funciones de sockets
  int leidos;
  //struct sockaddr_in server; //Direccion del servidor
  //socklen_t long_server;     //Tamaño estr. server

  //Abrimos socket
  struct sockaddr_in direccion;
  int descriptor;
  descriptor = socket(AF_INET, SOCK_DGRAM, 0);
  if(descriptor == -1){socket_con_server = -1;}

  //Rellenamos la estructura de datos para hacer el bind
  direccion.sin_family = AF_INET; //Socket inet ipv4
  direccion.sin_addr.s_addr = htonl(INADDR_ANY); //Cualquier dirección ip
  direccion.sin_port = htons(3000);

  //Hacemos el bind
  if(bind(descriptor, (struct sockaddr *)&direccion, sizeof(direccion)) == -1){
    close(descriptor);
    socket_con_server = -1;
  }
  socket_con_server = descriptor;

  if (socket_con_server == -1){
    printf("No puedo establecer conexión con el servidor\n");
    exit(-1);
  }

  //Ya esta conectado el socket
  //Ahora rellenamos la estructura server para enviarle un mensaje
  server.sin_family = AF_INET;
  server.sin_port = htons(3000);
  inet_aton("192.168.43.169", &server.sin_addr.s_addr);//La direccion del otro ordenador
  long_server = sizeof(server);

  //Construccion paquete

  int tam_linea;
  paquete = malloc(height_down_Y*sizeof(unsigned char *));
  for (int i=0;i<height_down_Y;i++) {
    tam_linea = (tam_bytes_Y[i]/4)+(tam_bytes_U[i]/4)+(tam_bytes_V[i]/4);
    //tam_linea = (1+((tam_bytes_Y[i]-1)/4))+(1+((tam_bytes_U[i]-1)/4))+(1+((tam_bytes_V[i]-1)/4));
    //printf("La linea %d mide %d\n",i,tam_linea);
		paquete[i]=malloc((tam_linea+4)*sizeof (unsigned long int));//char));
  }
  //int i=0;
  for (int i=0;i<height_down_Y;i++){
    //printf("Tamaños redondeados: Y=%d U=%d V=%d\n",tamY,tamU,tamV);

    int tamY=tam_bytes_Y[i]/4;
    int tamU=tam_bytes_U[i]/4;
    int tamV=tam_bytes_V[i]/4;

    int l=0;
    paquete[i][0]=i;
    paquete[i][1]=tam_bytes_Y[i];
    while (l<tamY){//tam_bytes_Y[i]/4){
      paquete[i][l+2]=bits_Y[i][l];
      l++;
    }
    int k=0;
    //paquete[i][(tam_bytes_Y[i]/4)+2]=tam_bytes_U[i];
    paquete[i][(tamY)+2]=tam_bytes_U[i];
    while (k<tamU){//tam_bytes_U[i]/4){
      paquete[i][l+3]=bits_U[i][k];
      l++;
      k++;
    }
    int m=0;
    //paquete[i][((tam_bytes_Y[i]/4)+(tam_bytes_U[i]/4))+3]=tam_bytes_V[i];
    paquete[i][tamY+tamU+3]=tam_bytes_V[i];
    while (m<tamV){//tam_bytes_V[i]/4){
      paquete[i][l+4]=bits_V[i][m];
      l++;
      m++;
    }
  }
}
*/

/*
void sendData(){//struct sockaddr_in server, socklen_t long_server, int socket_con_server){

  int tam_linea;
  unsigned long int dir;
  unsigned long int next_dir;
  //Construccion de paquetes
  int payload=1000;
  int carga=0;
  int j = 0;


  //Mando cabecera del frame

  //int height_orig_Y;
//int width_orig_Y;

//int height_orig_UV;
//int width_orig_UV;

//ancho y alto de imagen downsampleada
//int height_down_Y;
//int height_down_UV;
//int width_down_Y;
//int width_down_UV;
  int cabecera[8]={height_orig_Y,height_orig_UV,width_orig_Y,width_orig_UV,height_down_Y,height_down_UV,width_down_Y,width_down_UV};
  printf("Tamaño paquete %d\n",sendto(socket_con_server, (int *)&cabecera[0], 256, 0, (struct sockaddr *)&server, long_server ));

  while(j<height_down_Y){
    tam_linea=(tam_bytes_Y[j]/4)+(tam_bytes_U[j]/4)+(tam_bytes_V[j]/4);
    //tam_linea = (1+((tam_bytes_Y[j]-1)/4))+(1+((tam_bytes_U[j]-1)/4))+(1+((tam_bytes_V[j]-1)/4));
    carga+=tam_linea;

    if(carga>payload){
      printf("Tamaño paquete %d\n",sendto(socket_con_server, (int *)&paquete[j][0], carga, 0, (struct sockaddr *)&server, long_server ));
      //sendto(socket_con_server, (int *)&paquete[j][0], carga, 0, (struct sockaddr *)&server, long_server );
      carga = 0;
      //j++;
    }
    j++;
  }

  //Print de la primera lines de los datos enviados
  //j=0;
  //int tamY=tam_bytes_Y[0]/4;
  //int tamU=tam_bytes_U[0]/4;
  //int tamV=tam_bytes_V[0]/4;
  //for(int i=0;i<(tam_bytes_Y[0]/4)+(tam_bytes_U[0]/4)+(tam_bytes_V[0]/4)+4;i++){
  //for(int i=0;i<(tamY)+(tamU)+(tamV)+4;i++){
    //printf("datos de paquete en linea 0,%i son %lu\n",i,paquete[0][i]);
    //j++;
  }

  //Pruebas de cosas pequeñas para el socket de windows
  //printf("Tamaño paquete %d\n",sendto(socket_con_server, (int *)&height_down_Y, 32, 0, (struct sockaddr *)&server, long_server ));

}
*/


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void init_streamer()
{
pthread_mutex_init(&stream_subframe_mutex,NULL);

nal_byte_counter=0;

frame_byte_counter=0;

}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void *mytask_stream(void *arg)
{
struct thread_streamer_info *tinfo = arg;
//(int start, int separation, int num_threads)
int start=tinfo->start;
int separation=tinfo->separation;

//mutex
//th_done[tinfo->id]=PTHREAD_MUTEX_INITIALIZER;
//pthread_mutex_init(&stream_subframe_mutex,NULL);

pthread_mutex_lock(&stream_subframe_mutex);

 //const uint8_t frame[] = {0x00, 0x00, 0x00,0x01, 0x65}; //nal tipo 5
 //son 3 bits, 1 bit para el forbiden cero, dos para ref idc y 5 bits para el tipo
 // es decir  xxx xx xxxxx -> 0 11 00001 nal tipo 1 (coded slice of a non idr picture)
 //-> 0 11 00111 nal tipo 7 (sequence parameter set)
 //-> 0 11 00101 nal tipo 5 (coded slice of idr picture)

 //fwrite(&frame,sizeof(uint8_t),5,stdout);



//luminancias
int line=start;

if (line==0 ) newframe=true;//flag de nuevo frame

while (line<height_down_Y)
{
  stream_line(bits_Y, tam_bits_Y[line],line);

  line+=separation;
}


//crominancias
line=start;
while (line<height_down_UV)
{
stream_line(bits_U, tam_bits_U[line],line);
stream_line(bits_V, tam_bits_V[line],line);
  line+=separation;
}



fflush(stdout);


pthread_mutex_unlock(&stream_subframe_mutex);

}


//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void lanza_streamer_subframe(int startline,int separation)
{
pthread_attr_t attr;
pthread_attr_init(&attr);
pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_JOINABLE);

//struct thread_streamer_info tinfo;

tsinfo[startline].start=startline;
tsinfo[startline].separation=separation;

//crear thread
//pthread_t mithread;

//printf ("streaming %d \n", startline);
//pthread_create(&mithread, &attr, &mytask_stream, &tsinfo[startline]);

pthread_create(&streamer_thread[startline], &attr, &mytask_stream, &tsinfo[startline]);
//pthread_join(mithread, NULL);


}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void stream_line(uint8_t ** bits, int bits_lenght, int line)
{

    uint8_t line_low =  line % 255;//comenzamos en line=1
    uint8_t line_high  =line >> 8 + 1;
    uint16_t line_size_bytes = (bits_lenght%8 == 0)? bits_lenght/8 : (bits_lenght/8)+1;
    uint8_t cero=0;
     //uint8_t kk= 1+(uint8_t) line >>8;
    // fprintf(stderr, " sale %d  \n",kk);

//fprintf (stderr," line: %d \n", line);
 //if (line==0) //este if no vale porque el subframe que hace la linea Y tambien hace la linea U y la V,
  //if (line==0 && frame_byte_counter>15000) //solo sirve si el frame es suficientemente grande
  if (newframe)
  {
  total_frames+=1;
  total_time+=frame_byte_counter;
  int average_frame=total_time/total_frames;

  fprintf (stderr," average frame bytes: %d , this frame:%d \n", average_frame, frame_byte_counter);
  frame_byte_counter=0;
  newframe=false;
  }
  frame_byte_counter+=line_size_bytes;

   if (nal_byte_counter>1000)

   {

   const uint8_t frame[] = {0x00, 0x00, 0x00,0x01, 0x65}; //nal tipo 5
   //son 3 bits, 1 bit para el forbiden cero, dos para ref idc y 5 bits para el tipo
   // es decir  xxx xx xxxxx -> 0 11 00001 nal tipo 1 (coded slice of a non idr picture)
   //-> 0 11 00111 nal tipo 7 (sequence parameter set)
   //-> 0 11 00101 nal tipo 5 (coded slice of idr picture)
   fwrite(&frame,sizeof(uint8_t),5,stdout);
   nal_byte_counter=line_size_bytes;

   }
   else nal_byte_counter+=line_size_bytes;






   // fputc(23,stdout);
    //fwrite(&cero,sizeof(uint8_t),1,stdout);
    //fwrite(&cero,sizeof(uint8_t),1,stdout);
    //fwrite(&line_size_bytes,sizeof(uint16_t),1,stdout);
    fwrite(&line_high,sizeof(uint8_t),1,stdout);
    fwrite(&line_low,sizeof(uint8_t),1,stdout);

    fwrite(bits[line], sizeof(uint8_t), line_size_bytes, stdout);
    //bits[line][0]=0;
    //fputc(255,stdout);
    //fwrite(bits[line], sizeof(char), line_size_bytes, stdout);

/*
    char *texto;
    texto = "Hola que ase asdljhkajsfhdjkah uiowyerouhkasjnf ljdflk ajsdlfn,m alsdfj lkajdfasdf aasdfasdfde\n";
    fwrite (texto, sizeof(char), line_size_bytes, stdout);
*/



    //fflush(stdout);
    //fclose(stdout);

}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
void stream_frame()
//esta funcion no se usa, salvo con fines experimentales y de medicion
{

    int separacion=8, line ;

//file_pipe=fdopen(pipe_desc[1],"w");

    for(int i =0 ; i <separacion; i++)
    {
        line =i;

        while (line<height_down_Y)
        {
           // printf("numero de linea: %d\n", line);
          stream_line(bits_Y, tam_bits_Y[line],line);

          line=line+separacion;

        }
    }
    for(int i =0 ; i <separacion; i++)
    {
        line =i;
        while (line<height_down_UV)
        {
          stream_line(bits_U, tam_bits_U[line],line);
          line=line+separacion;
        }
    }
    for(int i =0 ; i <separacion; i++)
    {
        line =i;
        while (line<height_down_UV)
        {
          stream_line(bits_V, tam_bits_V[line],line);
          line=line+separacion;

        }
    }
 //fclose (stdout);
 //fclose(file_pipe);
}

