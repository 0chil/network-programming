#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

int printAddr(struct sockaddr_in *);

int main(int argc, char** argv){
	char *sampleIp = "127.0.0.1";
	int port = 9002;
	// set IP address and port for example

	struct sockaddr_in sockAddr1, sockAddr2, sockAddr3;

	// get host address inet_addr
	sockAddr1.sin_family = AF_INET;
	sockAddr1.sin_addr.s_addr = inet_addr(sampleIp);
	// addr as binary (nbo), access s_addr as the return is char *
	sockAddr1.sin_port = htons(port); // trying to send..

	sockAddr2.sin_family = AF_INET;
	inet_aton(sampleIp, &(sockAddr2.sin_addr));
	sockAddr2.sin_port = htons(port);

	sockAddr3.sin_family = AF_INET;
	inet_pton(AF_INET, sampleIp, &(sockAddr3.sin_addr));
	sockAddr3.sin_port = htons(port);
	
	printAddr(&sockAddr1);
	printAddr(&sockAddr2);
	printAddr(&sockAddr3);

	printf("ntoa============\n");
	printf("IP : %s \n", inet_ntoa(sockAddr1.sin_addr));
	printf("IP : %s \n", inet_ntoa(sockAddr2.sin_addr));
	printf("IP : %s \n", inet_ntoa(sockAddr3.sin_addr));
	
	return 0;
	
}

int printAddr(struct sockaddr_in *mySockAddr){
	int port = ntohs(mySockAddr->sin_port);
	char* txt = inet_ntoa(mySockAddr->sin_addr);
	printf("IP: %s, Port: %d \n", txt, port);
	return 0;
}
