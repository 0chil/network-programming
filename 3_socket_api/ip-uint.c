#include <netinet/in.h>
#include <stdio.h>
#include <arpa/inet.h>

int main(int argc, char** argv){
	uint32_t ipInInt;
	char* ipInStr = "127.0.0.1";
	ipInInt = inet_network(ipInStr);

	printf("String: %s, Decimal: %u , Hex: %X \n", ipInStr, ipInInt, ipInInt);

	ipInInt = htonl(ipInInt);
	printf("After htonl - Decimal :%u, Hex :%X \n", ipInInt, ipInInt);
	return 0;
}
