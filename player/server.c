
#include <winsock2.h>
#include <stdio.h>
#include <Ws2tcpip.h>
#include <time.h>
#pragma comment(lib,"ws2_32.lib")


// A sample of the select() return value
int recvfromTimeOutUDP(SOCKET socket, long sec, long usec)
{
	// Setup timeval variable
	struct timeval timeout;
	struct fd_set fds;

	timeout.tv_sec = sec;
	timeout.tv_usec = usec;
	// Setup fd_set structure
	FD_ZERO(&fds);
	FD_SET(socket, &fds);
	// Return value:
	// -1: error occurred
	// 0: timed out
	// > 0: data ready to be read
	return select(0, &fds, 0, 0, &timeout);
}

int main(int argc, char **argv)
{
	WSADATA            wsaData;
	SOCKET             ReceivingSocket;
	SOCKADDR_IN        ReceiverAddr;
	int                Port = 5150;
	char			ReceiveBuf[1100];
	int                BufLength = 1100;
	SOCKADDR_IN        SenderAddr;
	int                SenderAddrSize = sizeof(SenderAddr);
	int                ByteReceived = 5, SelectTiming, ErrorCode;
	char			ch = 'Y';

	// Initialize Winsock version 2.2
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	ReceivingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	ReceiverAddr.sin_family = AF_INET;
	ReceiverAddr.sin_port = htons(3000);
	ReceiverAddr.sin_addr.s_addr = INADDR_ANY;

	SenderAddr.sin_family = AF_INET;
	SenderAddr.sin_port = htons(3000);
	//SenderAddr.sin_addr.s_addr = inet_addr("192.168.43.19");
	inet_pton(AF_INET, "192.168.43.19", &(SenderAddr.sin_addr));
	bind(ReceivingSocket, (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr));

	struct fd_set fds;
	FD_ZERO(&fds);
	FD_SET(ReceivingSocket, &fds);
	SelectTiming = select(0, &fds, 0, 0, NULL);

	switch (SelectTiming)
	{
	case 0:
		// Timed out, do whatever you want to handle this situation
		printf("Server: Timeout ...\n");
		break;
	case -1:
		// Error occurred, maybe we should display an error message?
		// Need more tweaking here and the recvfromTimeOutUDP()...
		printf("Server: Codigo error: %ld\n", WSAGetLastError());
		break;
	default:
	{
		//clock_t start = clock();
		while (1)
		{
			// Call recvfrom() to get it then display the received data...
			ByteReceived = recvfrom(ReceivingSocket, ReceiveBuf, BufLength,
					0, (SOCKADDR *)&SenderAddr, &SenderAddrSize);
			//Medidas de tiempo
			//clock_t start = clock();
			//for (int i = 0; i < 10;i++) {
				//ByteReceived = recvfrom(ReceivingSocket, ReceiveBuf, BufLength, 0, (SOCKADDR *)&SenderAddr, &SenderAddrSize);
			//}
				printf("Bytes Recibidos: %d\n", ByteReceived);
				printf("Vamos a ver el dato en char \"%s\"\n", ReceiveBuf);

		}
	}
	}

	// When your application is finished receiving datagrams close the socket.
	closesocket(ReceivingSocket);
	WSACleanup();
	getchar();
	return 0;
}
