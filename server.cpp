#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PACK_SIZE = 516
#define Data_SIZE = 512
#define TIMEOUT = 3
#define MAX_FAIL = 7
using namespace std;

bool state;

struct st_ack
{
	uing16_t op;
	uing16_t num;
} __attribute__((packed));
struct st_wrq
{
	uing16_t op;
	string data[Data_SIZE+2];
} __attribute__((packed));
struct st_data
{
	uing16_t op;
	uing16_t num;
	char data[Data_SIZE];
} __attribute__((packed));

void error(char *msg) {
	perror(msg);
	exit(1);
}


struct st_ack proccess_wrq(struct st_wrq &wrq){
}
struct st_ack proccess_data(struct st_data &data){
}


int main(int argc, char *argv[]) {
	if(argc != 2)
		error("wrong number of parameters");
	unsigned short echoServPort = atoi(argv[1]);

	int sock_fd;
	/* Socket */
	struct sockaddr_in serverSocketAddr; /* Local address */
	struct sockaddr_in clientSocketAddr; /* Client address */
	unsigned int clientAddrLen; /* Length of incoming message */
	char echoBuffer[PACK_SIZE]; /* Buffer for echo string */
	int recvMsgSize; /* Size of received message */
	fd_set fdSet;
    FD_ZERO(&fdSet);
    FD_SET(sock_fd, &fdSet);
    struct timeval timeInt;
    timeInt.tv_sec = TIMEOUT;
    timeInt.tv_usec = 0;
    struct st_ack wrq;
    struct st_data data;
    struct st_ack ack;

	/* Create socket for sending/receiving datagrams */
	if ((sock_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		error("socket creation failed");

	/* Construct local address structure */
	memset(&serverSocketAddr, 0, sizeof(serverSocketAddr)); /* Zero out structure */
	serverSocketAddr.sin_family = AF_INET; /* Internet address family */
	serverSocketAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	serverSocketAddr.sin_port = htons(echoServPort); /* Local port */

	/* Bind to the local address */
	if (bind(sock_fd, (struct sockaddr *) &serverSocketAddr, sizeof(serverSocketAddr)) < 0)
		error("socket bind failed");

	while(true) {
		/* Set the size of the in-out parameter */
		clientAddrLen = sizeof(clientSocketAddr);

		/* Block until receive message from a client */
		recvMsgSize = recvfrom(sock_fd, &wrq, PACK_SIZE, 0, (struct sockaddr *) &clientSocketAddr, &clientAddrLen);
		if (recvMsgSize < 0)
			error("socket recvfrom failed");
		printf("Handling client %s\n", inet_ntoa(clientSocketAddr.sin_addr));

        ack = proccess_wrq(wrq);

        if (sendto(sock_fd, &ack, sizeof(ack), 0, (struct sockaddr*)clientSocketAddr,  sizeof(*clientSocketAddr))  != sizeof(clientSocketAddr))
            error("faild to send response");

		int timeoutExpiredCount = 0;
		int packetReady = 0;
		string sendData;
		do
		{
			do
			{
				do
				{
					// TODO: Wait WAIT_FOR_PACKET_TIMEOUT to see if something appears                  
					//       for us at the socket (we are waiting for DATA)
					packetReady = select(sock_fd+1, &fdSet, NULL, NULL, timeInt);

					if (packetReady > 0)// TODO: if there was something at the socket and we are here not because of a timeout
					              
					{
						// TODO: Read the DATA packet from the socket (at least we hope this is a DATA packet)
						recvMsgSize = recvfrom(sock_fd, &data, PACK_SIZE, 0, (struct sockaddr *) &clientSocketAddr, &clientAddrLen);
					}
					if   (packetReady == 0) // TODO: Time out expired while waiting for data to appear at the socket
				   	{  
				   		//TODO: Send another ACK for the last packet   
				   		timeoutExpiredCount++;  
				   	}
				   	if   (packetReady < 0)
				   	{
				   	    error("Error while waiting for packet");
				   	}
				   	if   (timeoutExpiredCount>= MAX_FAIL)  
				   	{                 
				   		error("Timeout while waiting for packet");
				   	} 
				} while(recvMsgSize < 0) // TODO: Continue while some socket was ready  
						     //       but recvfrom somehow failed to read the data
			} while(false);
			timeoutExpiredCount = 0;

			ack = proccess_data(data);
            if (sendto(sock_fd, &ack, sizeof(ack), 0, (struct sockaddr*)clientSocketAddr,  sizeof(*clientSocketAddr))  != sizeof(clientSocketAddr))
                error("faild to send response");

			// TODO: send ACK packet to the client
		} while(recvMsgSize == PACK_SIZE); // Have blocks left to be read from client (not end of transmission)
	}
	/* NOT REACHED */
}	