#include <netdb.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>

#define PORT 6123    // the port client will be connecting to
#define HOSTNAME "localhost"
#define ERROR -1

#define BUFFERSIZE 65536 // max size of buffer


/*- This function provides integration with external tools
 * Currently uses system call use GNU tools echo and tr to change "a" into "e"
 * Which writes output in file, which is read back into passed buffer
 *
 * This file can be compiled separately from espeak-ng with command:
 * gcc -DSTANDALONE -o preprocess.o preprocess.c
 *
 * and then executed as stand alone executable with
 * ./preprocess.o "passed arguments"
 */
void preprocessText(char **data) {
	int sockfd;
/*        
int i;
	for (i=0;i<strlen(*data);i++)
	{
		if(((*data)[i] & 127 ) == 0){
			continue;
		}
		else if(((*data)[i] & 0xE0) == 0xE0){
       			 FILE *before;
       			 count = fopen("/home/omar/debug/count.txt","a");
        		fprintf(count, "+1 \n");

			memmove(&(*data)[i], &(*data)[i+3], strlen(*data) - i);
			i--;
		}
		else if(((*data)[i] & 0xC0) == 0xC0){
			i++;
			continue;
		}
	}*/
        FILE *before;
        before = fopen("/home/omar/debug/before.txt","a");
        fprintf(before, (*data));
	fprintf(before, "\n");

	struct hostent *he;
	struct sockaddr_in sockedAddr; // connector's address information
	char hostname[50] = HOSTNAME;

	if ((he = gethostbyname(hostname)) == NULL) { // get the host info
		herror("gethostbyname");
		return;
	}

	if ((sockfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
		perror("socket");
		return;
	}

	sockedAddr.sin_family = AF_INET; // host byte order
	sockedAddr.sin_port = htons(PORT); // short, network byte order
	sockedAddr.sin_addr = *((struct in_addr *) he->h_addr);
	bzero(&(sockedAddr.sin_zero), 8); // zero the rest of the struct

	if (connect(sockfd, (struct sockaddr *) &sockedAddr,
			sizeof(struct sockaddr)) == -1) {
		perror("connect");
		return;
	}

	char bufSend[BUFFERSIZE + 1] = { 0 }; // reserve last byte for null terminator (for debugging purposes)
	int sentPacketBytes;
	int sentBytes = 0;
	do {
		strncpy(bufSend, (*data) + sentBytes, BUFFERSIZE); // copy part of data into send buffer
		//FILE *fp1;
                //fp1 = fopen('/home/omar/debug/fp1.txt','w');
                //fprintf(fp1,bufSend);
                //fprintf(fp1,'\n \n \n');

                int packetSize = strlen(bufSend) + 1;
		if (packetSize > BUFFERSIZE)
			packetSize = BUFFERSIZE;
		sentPacketBytes = send(sockfd, bufSend, packetSize, 0);
		if (sentPacketBytes == ERROR) {
			perror("send() error");
			return;
		}
		sentBytes += sentPacketBytes;
	} while (strlen(bufSend) == BUFFERSIZE); // send data while string is not ended inside buffer

	char bufRecieve[BUFFERSIZE + 1] = { 0 };
	int recvPacketBytes;
	int recvBytes = 0;
	do {
		recvPacketBytes = recv(sockfd, bufRecieve, BUFFERSIZE, 0);
		if (recvPacketBytes == ERROR) {
			perror("receive() error");
			return;
		}
		strncpy((*data) + recvBytes, &bufRecieve[0], recvPacketBytes); // copy data from received buffer to data buffer
		recvBytes += recvPacketBytes;
	} while (strlen(bufRecieve) == BUFFERSIZE); // receive data while string is not ended inside buffer
	close(sockfd);
        
	FILE *after;
        after = fopen("/home/omar/debug/after.txt","a");
        fprintf(after, (*data));
	fprintf(after, "\n");


}

/*
 * This function is used for testing as stand-alone executable
 */
#ifdef STANDALONE
int main(int argc, char **argv) {
	char data[1024] = { 0 };
	char *dataref = &data[0];
	if (argc > 1)
		strcpy(data, argv[1]);
	printf("data before:%s\n",data);
	preprocessText(&dataref);
	printf("data  after:%s\n", data);
}
#endif

