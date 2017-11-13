
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
	if (WSAStartup(MAKEWORD(2, 2), &wsaData) != 0)
	{
		printf("Server: WSAStartup failed with error %ld\n", WSAGetLastError());
		return -1;
	}
	else
		printf("Server: The Winsock DLL status is %s.\n", wsaData.szSystemStatus);

	// Create a new socket to receive datagrams on.
	ReceivingSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

	if (ReceivingSocket == INVALID_SOCKET)
	{
		printf("Server: Error at socket(): %ld\n", WSAGetLastError());
		// Clean up
		WSACleanup();
		// Exit with error
		return -1;
	}
	else
		printf("Server: socket() is OK!\n");

	// Set up a SOCKADDR_IN structure that will tell bind that we
	// want to receive datagrams from all interfaces using port 5150.

	ReceiverAddr.sin_family = AF_INET;
	ReceiverAddr.sin_port = htons(3000);
	ReceiverAddr.sin_addr.s_addr = INADDR_ANY;

	SenderAddr.sin_family = AF_INET;
	SenderAddr.sin_port = htons(3000);
	//SenderAddr.sin_addr.s_addr = inet_addr("192.168.43.19");
	inet_pton(AF_INET, "192.168.43.19", &(SenderAddr.sin_addr));

	// Associate the address information with the socket using bind.
	// At this point you can receive datagrams on your bound socket.
	if (bind(ReceivingSocket, (SOCKADDR *)&ReceiverAddr, sizeof(ReceiverAddr)) == SOCKET_ERROR)
	{
		printf("Server: bind() failed! Error: %ld.\n", WSAGetLastError());
		// Close the socket
		closesocket(ReceivingSocket);
		// Do the clean up
		WSACleanup();
		// and exit with error
		return -1;
	}
	else
		printf("Server: bind() is OK!\n");

	// Some info on the receiver side...
	//getsockname(ReceivingSocket, (SOCKADDR *)&ReceiverAddr, (int *)sizeof(ReceiverAddr));

	//printf("Server: Receiving IP(s) used: %s\n", inet_ntoa(ReceiverAddr.sin_addr));
	printf("Server: Receiving port used: %d\n", htons(ReceiverAddr.sin_port));
	printf("Server: I\'m ready to receive a datagram...\n");


	//SelectTiming = recvfromTimeOutUDP(ReceivingSocket, 10, 0);
	//La siguientes lineas ahorran la funcion recvfromTimeOutUDP 
	//permitiendo que el select con el null al final sea bloqueante
	//y espere lo que haga falta,
	struct fd_set fds;
	FD_ZERO(&fds);
	FD_SET(ReceivingSocket, &fds);
	SelectTiming = select(0, &fds, 0, 0, NULL);

	switch (SelectTiming)
	{
	case 0:
		// Timed out, do whatever you want to handle this situation
		printf("Server: Timeout lor while waiting you bastard client!...\n");
		break;
	case -1:
		// Error occurred, maybe we should display an error message?
		// Need more tweaking here and the recvfromTimeOutUDP()...
		printf("Server: Some error encountered with code number: %ld\n", WSAGetLastError());
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
			if (ByteReceived > 0)
			{
				printf("Server: Total Bytes received: %d\n", ByteReceived);
				printf("Server: The data is \"%s\"\n", ReceiveBuf);
			}
			else if (ByteReceived <= 0)
				printf("Server: Connection closed with error code: %ld\n",
					WSAGetLastError());
			else
				printf("Server: recvfrom() failed with error code: %d\n",
					WSAGetLastError());
		}

		//clock_t end = clock();
		//float seconds = (float)(end - start) / CLOCKS_PER_SEC;
		//printf("Receving time is %.16g\n", seconds);

		// Some info on the sender side
		//getpeername(ReceivingSocket, (SOCKADDR *)&SenderAddr, &SenderAddrSize);
		//printf("Server: Sending IP used: %s\n", inet_ntoa(SenderAddr.sin_addr));
		printf("Server: Sending port used: %d\n", htons(SenderAddr.sin_port));
	}
	}

	// When your application is finished receiving datagrams close the socket.
	printf("Server: Finished receiving. Closing the listening socket...\n");
	if (closesocket(ReceivingSocket) != 0)
		printf("Server: closesocket() failed! Error code: %ld\n", WSAGetLastError());
	else
		printf("Server: closesocket() is OK...\n");

	// When your application is finished call WSACleanup.
	printf("Server: Cleaning up...\n");
	if (WSACleanup() != 0)
		printf("Server: WSACleanup() failed! Error code: %ld\n", WSAGetLastError());
	else
		printf("Server: WSACleanup() is OK\n");
	// Back to the system
	getchar();
	return 0;
}
