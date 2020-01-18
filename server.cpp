#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <stdlib.h>
#include <string.h>
#include <sys/select.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>

#define PACK_SIZE 516
#define Data_SIZE 512
#define TIMEOUT 3
#define MAX_FAIL 7

using namespace std;

int nextNum = 0;

uint16_t WRQ_OP = 2;
uint16_t DATA_OP = 3;
uint16_t ACK_OP = 4;
const char* MODE = "octet";

struct st_ack
{
	uint16_t op;
	uint16_t num;
} __attribute__((packed));
struct st_wrq
{
	uint16_t op;
	char data[Data_SIZE+2];
} __attribute__((packed));
struct st_data
{
	uint16_t op;
	uint16_t num;
	char data[Data_SIZE];
} __attribute__((packed));


void error(const char *msg) {
	perror(msg);
	exit(1);
}

struct st_ack proccess_wrq(struct st_wrq* wrq, FILE** fd){
	// check opcode
	if(ntohs(wrq->op) != WRQ_OP)
		error("FLOWERROR: Wrong opcode for wrq packet");

	//split filename and mode!
	char* ptr = wrq->data;
	char filename[255];
	char mode[255];
	strncpy(filename, ptr, 255);
	ptr += strlen(ptr)+1;
	strncpy(mode, ptr, 255); 

	// open file for writing
	*fd = fopen(filename,"w");

	printf("IN:WRQ,%s,%s\n", filename, mode);

	// check filename and mode
	if(strlen(filename) == 0){
		printf("FLOWERROR: file name is not correct");
		exit(1);
	}
	if(strcmp(mode,MODE)){
		printf("FLOWERROR: file type not supported");
		exit(1);
	}

	// make ack message
	struct st_ack ack;
	memset(&ack, 0, sizeof(ack));
	ack.op = htons(ACK_OP);
	ack.num = htons(0);

	nextNum += 1;
	return ack;
}
struct st_ack proccess_data(struct st_data* data, int datalen, FILE** fd){
	// check opcode
	if(ntohs(data->op) != DATA_OP){
		printf("FLOWERROR: Wrong opcode for data packet");
		exit(1);
	}

	// check packete num
	uint16_t num = ntohs(data->num);
	if(num != nextNum){
		printf("FLOWERROR: wrong packet num");
		exit(1);
	}

	// write data to file
	int dataLen = strlen(data->data);
	printf("WRITING: %d\n", dataLen);
	fwrite(data->data, sizeof(char), datalen, *fd);

	// check if end of file
	if(dataLen < Data_SIZE){
		printf("RECVOK\n");
		fclose(*fd);
	}

	printf("IN:DATA, %d, %d\n", num, dataLen);

	// make ack message
	struct st_ack ack;
	memset(&ack, 0, sizeof(ack));
	ack.op = htons(ACK_OP);
	ack.num = htons(num);

	nextNum += 1;
	return ack;
}


int main(int argc, char *argv[]) {
	if(argc != 2)
		error("TTFTP_ERROR: wrong number of parameters");
	unsigned short echoServPort = atoi(argv[1]);

	int sock_fd;
	FILE* data_fd;
	struct sockaddr_in serverSocketAddr; /* Local address */
	struct sockaddr_in clientSocketAddr; /* Client address */
	unsigned int clientAddrLen; /* Length of incoming message */
	char echoBuffer[PACK_SIZE]; /* Buffer for echo string */
	int recvMsgSize; /* Size of received message */

	fd_set fdSet;
    FD_ZERO(&fdSet);
    struct timeval timeInt;

    struct st_wrq wrq;
    struct st_data data;
    struct st_ack ack;
    memset(&wrq, 0, sizeof(wrq));
    memset(&data, 0, sizeof(data));
    memset(&ack, 0, sizeof(ack));

	/* Create socket for sending/receiving datagrams */
	if ((sock_fd = socket(PF_INET, SOCK_DGRAM, IPPROTO_UDP)) < 0)
		error("TTFTP_ERROR: socket creation failed");
    FD_SET(sock_fd, &fdSet);

	/* Construct local address structure */
	memset(&serverSocketAddr, 0, sizeof(serverSocketAddr)); /* Zero out structure */
	serverSocketAddr.sin_family = AF_INET; /* Internet address family */
	serverSocketAddr.sin_addr.s_addr = htonl(INADDR_ANY); /* Any incoming interface */
	serverSocketAddr.sin_port = htons(echoServPort); /* Local port */

	/* Bind to the local address */
	if (bind(sock_fd, (struct sockaddr *) &serverSocketAddr, sizeof(serverSocketAddr)) < 0)
		error("TTFTP_ERROR: socket bind failed");

	while(true) {
		/* Set the size of the in-out parameter */
		clientAddrLen = sizeof(clientSocketAddr);

		/* Block until receive message from a client */
		recvMsgSize = recvfrom(sock_fd, &wrq, PACK_SIZE, 0, (struct sockaddr *) &clientSocketAddr, &clientAddrLen);
		if (recvMsgSize < 0)
			error("TTFTP_ERROR: socket recvfrom failed");
        ack = proccess_wrq(&wrq, &data_fd);

        if (sendto(sock_fd, &ack, sizeof(ack), 0, (struct sockaddr*) &clientSocketAddr,  clientAddrLen)  != sizeof(ack))
            error("TTFTP_ERROR: faild to send response");
        printf("OUT:ACK, %d\n", ack.num);

		int timeoutExpiredCount = 0;
		int packetReady = 0;
		do
		{
			do
			{
				recvMsgSize = -1;
				do
				{
					// TODO: Wait WAIT_FOR_PACKET_TIMEOUT to see if something appears                  
					//       for us at the socket (we are waiting for DATA)
					timeInt.tv_sec = TIMEOUT;
					timeInt.tv_usec = 0;
					packetReady = select(sock_fd+1, &fdSet, NULL, NULL, &timeInt);
					if (packetReady > 0)// TODO: if there was something at the socket and we are here not because of a timeout
					{
						// TODO: Read the DATA packet from the socket (at least we hope this is a DATA packet)
						recvMsgSize = recvfrom(sock_fd, &data, PACK_SIZE, 0, (struct sockaddr *) &clientSocketAddr, &clientAddrLen);
						if (recvMsgSize < 0)
							error("TTFTP_ERROR: socket recvfrom failed");
						ack = proccess_data(&data, recvMsgSize, &data_fd);

					}
					if   (packetReady == 0) // TODO: Time out expired while waiting for data to appear at the socket
				   	{  
				   		printf("FLOWERROR: timeout while waiting for packet");
				   		//TODO: Send another ACK for the last packet  
				   		if (sendto(sock_fd, &ack, sizeof(ack), 0, (struct sockaddr*) &clientSocketAddr,  clientAddrLen)  != sizeof(ack))
				            error("TTFTP_ERROR: faild to send response");
				        printf("OUT:ACK, %d\n", ack.num); 
				   		timeoutExpiredCount++;  
				   	}
				   	if   (packetReady < 0)
				   	{
				   	    error("TTFTP_ERROR: error while waiting for packet");
				   	}
				   	if   (timeoutExpiredCount>= MAX_FAIL)  
				   	{                 
				   		printf("FLOWERROR: failed %d times\n", MAX_FAIL);
				   		exit(1);
				   	} 
				} while(recvMsgSize <= 0); // TODO: Continue while some socket was ready  
						     //       but recvfrom somehow failed to read the data
			} while(false);
			timeoutExpiredCount = 0;

			// TODO: send ACK packet to the client
	        if (sendto(sock_fd, &ack, sizeof(ack), 0, (struct sockaddr*) &clientSocketAddr,  clientAddrLen)  != sizeof(ack))
	            error("TTFTP_ERROR: faild to send response");
	        printf("OUT:ACK, %d\n", ack.num);

		} while(recvMsgSize >= PACK_SIZE); // Have blocks left to be read from client (not end of transmission)
	}
	/* NOT REACHED */
}	