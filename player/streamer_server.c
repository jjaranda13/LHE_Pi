
#include <winsock.h>
#include <stdio.h>
#pragma comment(lib,"ws2_32.lib")

void main() 
{
	printf("Hola mundo!\n");
	
	//int BufLen = 1024;
	int cont;//altura
	unsigned long int **bits_Y;//Para almacenerla igual que me llega
	int height_down_Y;
	int *tam_linea = malloc(256 * sizeof(int));
	WSADATA wsaData;
	SOCKET RecvSocket;
	struct sockaddr_in RecvAddr;
	int Puerto = 3000;
	//int RecvBuf;
	struct sockaddr_in SenderAddr;
	int SenderAddrSize = sizeof(SenderAddr);

	WSAStartup(MAKEWORD(2, 2), &wsaData);
	RecvSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
	RecvAddr.sin_family = AF_INET;
	RecvAddr.sin_port = htons(Puerto);
	RecvAddr.sin_addr.s_addr = INADDR_ANY;
	bind(RecvSocket, (SOCKADDR *)&RecvAddr, sizeof(RecvAddr));
	//printf("Recibo %d bytes\n", recvfrom(RecvSocket, (int *)&RecvBuf, 32, 0, (SOCKADDR *)&SenderAddr, &SenderAddrSize));
	//printf("%d\n", RecvBuf);
	recvfrom(RecvSocket, (int *)&cont, sizeof(cont), 0, (struct sockaddr *)&SenderAddr, &SenderAddrSize);
	height_down_Y = cont;
	bits_Y = malloc(height_down_Y * sizeof(unsigned char*));
	printf("La altura es %d\n", height_down_Y);
	for (int i = 0; i<height_down_Y; i++){ //= i + 4) {
		//recibo el tamaño de linea
		recvfrom(RecvSocket, (int *)&cont, sizeof(cont), 0, (struct sockaddr *)&SenderAddr, &SenderAddrSize);
		tam_linea[i] = cont;
		bits_Y[i] = malloc(tam_linea[i] * sizeof(unsigned char));
		printf("Linea %d de %d bytes\n", i, tam_linea[i]);
		//Recibo los bytes de la linea
		//recvfrom(RecvSocket, &bits_Y[i][0], tam_linea[i], 0, (struct sockaddr *)&SenderAddr, &SenderAddrSize);
	}

	closesocket(RecvSocket);
	WSACleanup();

	getchar();
}
