#include <stdio.h>
#include <string.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netdb.h> // addrinfo, hostent, getbyhostname, getaddrinfo
#include <arpa/inet.h>
#include <netinet/in.h>

int main(int argc, char **argv)
{
	struct addrinfo hints, *res, *p; // hints, 결과 받을 변수, 반복문용 변수
	int status;
	char ipstr[INET6_ADDRSTRLEN];

	if (argc != 2)
	{
		fprintf(stderr, "Usage : %s [hostname] \n", argv[0]);
		return 1;
	}

	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((status = getaddrinfo(argv[1], NULL, &hints, &res)) != 0)
	{
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(status));
		return 2;
	}

	printf("IP Addresses for %s:\n", argv[1]);
	p = res;
	while (p != NULL)
	{
		void *addr;
		char *ipver;

		if (p->ai_family == AF_INET)
		{
			addr = &((struct sockaddr_in *)p->ai_addr)->sin_addr;
			ipver = "IPv4";
		}
		else
		{
			addr = &((struct sockaddr_in6 *)p->ai_addr)->sin6_addr;
			ipver = "IPv6";
		}

		inet_ntop(p->ai_family, addr, ipstr, sizeof(ipstr));
		printf("	%s: %s\n", ipver, ipstr);

		p = p->ai_next;
	}

	freeaddrinfo(res);

	return 0;
}
