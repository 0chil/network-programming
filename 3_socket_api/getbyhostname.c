#include <stdio.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <errno.h>
int main(int argc, char** argv)
{
	struct hostent *ent;
	struct in_addr **res;

	int i = 0;
	if(argc != 2)
	{
		fprintf(stderr, "Usage :%s <hostname> \n", argv[0]);
		return -1;
	}

	ent = gethostbyname(argv[1]);
	if(ent == NULL) return 1;
	res = ent->h_addr_list;
	printf("hostname : %s\n", ent->h_name);
	while(res[i] != NULL)
	{
		printf("%s ", inet_ntoa(*res[i]));
		i++;
	}
	printf("\n");

}
