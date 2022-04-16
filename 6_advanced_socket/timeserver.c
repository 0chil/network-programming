#include <stdio.h>
#include <time.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <netdb.h>
#include <errno.h>

#define ISVALIDSOCKET(s) s >= 0
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() errno

int main(int argc, char **argv)
{
	printf("Configuring local address...\n");
	struct addrinfo hints, *res;
	memset(&hints, 0, sizeof(hints));
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE;
	getaddrinfo(NULL, "8080", &hints, &res);

	printf("Creating Socket....\n");
	SOCKET socket_listen = socket(res->ai_family, res->ai_socktype, res->ai_protocol);

	if (!ISVALIDSOCKET(socket_listen))
	{
		fprintf(stderr, "socket() failed (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	printf("Binding socket to local address...\n");
	if (bind(socket_listen, res->ai_addr, res->ai_addrlen) != 0)
	{
		fprintf(stderr, "bind() failed (%d)\n", GETSOCKETERRNO());
		return 1;
	}
	freeaddrinfo(res);

	printf("Listening...\n");
	if (listen(socket_listen, 10) != 0)
	{
		fprintf(stderr, "listen() failed (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	printf("Waiting for connection...\n");
	struct sockaddr_storage client_address;
	socklen_t client_len;
	SOCKET socket_client = accept(socket_listen, (struct sockaddr *)&client_address, &client_len);
	if (!ISVALIDSOCKET(socket_client))
	{
		fprintf(stderr, "accept() failed (%d)\n", GETSOCKETERRNO());
		return 1;
	}

	printf("Client is connected...\n");
	char address_buffer[100];
	getnameinfo((struct sockaddr *)&socket_client, client_len, address_buffer, sizeof(address_buffer), 0, 0, NI_NUMERICHOST);
	printf("%s\n", address_buffer);

	printf("Reading request...\n");
	char request[1024];
	int bytes_received = recv(socket_client, request, sizeof(request), 0);
	printf("Received %d bytes\n", bytes_received);

	printf("Sending response...\n");
	const char *response =
		"HTTP/1.1 200 OK\r\n"
		"Connection: close\r\n"
		"Content-Type: text/plain\r\n\r\n"
		"Local time is: ";
	int bytes_sent = send(socket_client, response, strlen(response), 0);
	printf("Sent %d of %d bytes\n", bytes_sent, strlen(response));

	time_t timer;
	time(&timer);
	char *time_msg = ctime(&timer);
	bytes_sent = send(socket_client, time_msg, strlen(time_msg), 0);
	printf("Sent %d of %d bytes\n", bytes_sent, strlen(time_msg));

	printf("Closing connection...\n");
	CLOSESOCKET(socket_client);

	printf("Closing listening socket...\n");
	CLOSESOCKET(socket_listen);

	return 0;
}
