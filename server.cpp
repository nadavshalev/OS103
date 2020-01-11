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

string tftp_prtotocol(char* dataIn){
	if(state){ //conn is active

	}
	else{
		struct st_wrq wrqObj = (struct st_wrq)dataIn;
		string data = string(wrqObj.data);
		size_t found = data.find('0');
		if (found == string::npos)
			error("illegal filename");
		sting filename = data.substr(0,found-1);
		string restData = data.substr(found+1, string::npos);
	}
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
		recvMsgSize = recvfrom(sock_fd, echoBuffer, PACK_SIZE, 0, (struct sockaddr *) &clientSocketAddr, &clientAddrLen);
		if (recvMsgSize < 0)
			error("socket recvfrom failed");
		printf("Handling client %s\n", inet_ntoa(clientSocketAddr.sin_addr));


		int timeoutExpiredCount = 0;
		do
		{
			do
			{
				do
				{
					// TODO: Wait WAIT_FOR_PACKET_TIMEOUT to see if something appears                  
					//       for us at the socket (we are waiting for DATA)

					if ()// TODO: if there was something at the socket and                       
						 //       we are here not because of a timeout   
						recvMsgSize = recvfrom(sock_fd, echoBuffer, PACK_SIZE, 0, (struct sockaddr *) &clientSocketAddr, &clientAddrLen);
					              
					{                 	   
						// TODO: Read the DATA packet from the socket (at                      
						//       least we hope this is a DATA packet)                
					}
					if   (...) // TODO: Time out expired while waiting for data    
							   //       to appear at the socket  
				   	{  
				   		//TODO: Send another ACK for the last packet   
				   		timeoutExpiredCount++;  
				   	}
				   	if   (timeoutExpiredCount>= MAX_FAIL)  
				   	{                 
				   		// FATAL ERROR BAIL OUT 
				   	} 
				} while(recvMsgSize < 0) // TODO: Continue while some socket was ready  
						     //       but recvfrom somehow failed to read the data
			} while(false);
			timeoutExpiredCount = 0;         
			lastWriteSize = fwrite(...); 
			// write next bulk of data 
			// TODO: send ACK packet to the client
		} while(...); // Have blocks left to be read from client (not end of transmission)


		/* Send received datagram back to the client */
		if (sendto(sock_fd, echoBuffer, recvMsgSize, 0, (struct sockaddr *) &clientSocketAddr, sizeof(clientSocketAddr)) != recvMsgSize)
			error("sendto() sent a different number of bytes than expected");
	}
	/* NOT REACHED */
}	