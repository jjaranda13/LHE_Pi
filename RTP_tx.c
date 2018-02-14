

#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/time.h>
#include <math.h>
#include <stdbool.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

char payload_buffer[2000]={'a'};
char header_buffer[16];
uint16_t sequence=0;

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
char* CreatePayload()
{
int i;
for ( i=0;i<1024;i++)
{
payload_buffer[i]='a';
}

return payload_buffer;

}
//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
char* CreateHeader()
{
//byte0 contiene varios bits. los dos primeros son la version RTP=2
header_buffer[0]=0b10000000;

//tipo de carga util (dynamic =96..127)
header_buffer[1]=100;

//numero de secuencia
sequence=(sequence+1) % 65000;
header_buffer[2]=sequence/255;
header_buffer[3]=sequence % 255;

//marca tiempo
header_buffer[4]=0;
header_buffer[5]=0;
header_buffer[6]=0;
header_buffer[7]=0;

//SSRC fuente de sincronizacion
header_buffer[8]=0;
header_buffer[9]=0;
header_buffer[10]=0;
header_buffer[11]=0;

//CSRC fuente del contenido
header_buffer[12]=0;
header_buffer[13]=0;
header_buffer[14]=0;
header_buffer[15]=0;

return header_buffer;



}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%

void Rtp_packetizer()
{

char *h;
char *p;

while (true)
{

h=CreateHeader();
p=CreatePayload();

fwrite(h, sizeof(char), 16,stdout);
fwrite(p, sizeof(char), 1024,stdout);
fflush(stdout);
//fclose (stdout);
sleep(1);

}
}

//%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%%
int main(int argc, char* argv[]) {

if (argc==2)
{
 if (!strcmp(argv[1], "-test"))
 {

 }
 }
Rtp_packetizer();

}
