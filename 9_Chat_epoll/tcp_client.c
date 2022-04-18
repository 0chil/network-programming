#include <stdio.h>
#include <netdb.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <string.h>
#include <netinet/in.h>
#include <unistd.h>
#include <errno.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

int main(int argc, char **argv)
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage : %s hostname port", argv[1]);
        return 1;
    }

    printf("Retrieve remote address...\n");
    struct addrinfo hints, *peer;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    if (getaddrinfo(argv[1], argv[2], &hints, &peer) != 0)
    {
        fprintf(stderr, "getaddrinfo() failed\n");
        return 1;
    }

    printf("Remote address is: ");
    char address_buff[100], service_buff[100];
    getnameinfo(peer->ai_addr, peer->ai_addrlen, address_buff, sizeof(address_buff), service_buff, sizeof(service_buff), NI_NUMERICHOST);
    printf("%s, %s\n", address_buff, service_buff);

    printf("create socket\n");
    SOCKET socket_peer;
    socket_peer = socket(peer->ai_family, peer->ai_socktype, peer->ai_protocol);
    if (!ISVALIDSOCKET(socket_peer))
    {
        fprintf(stderr, "socket() failed\n");
        return 1;
    }

    printf("connect to host\n");
    if (connect(socket_peer, peer->ai_addr, peer->ai_addrlen) != 0)
    {
        fprintf(stderr, "connect() failed\n");
        return 1;
    }
    freeaddrinfo(peer);

    printf("connected to %s, %s\n", address_buff, service_buff);
    while (1)
    {
        fd_set reads;
        FD_ZERO(&reads);
        FD_SET(socket_peer, &reads);
        FD_SET(0, &reads);

        struct timeval timeout;
        timeout.tv_sec = 0;
        timeout.tv_usec = 100000;

        if (select(socket_peer + 1, &reads, 0, 0, &timeout) < 0)
        {
            fprintf(stderr, "select() failed\n");
            return 1;
        }

        if (FD_ISSET(socket_peer, &reads))
        {
            char read[BUFSIZ];
            int readLen = recv(socket_peer, read, BUFSIZ - 1, 0);
            if (readLen == 0)
            {
                printf("connection closed\n");
                break;
            }
            printf("Received %d bytes: %.*s", readLen, readLen, read);
        }

        if (FD_ISSET(0, &reads))
        {
            char read[BUFSIZ];
            if (!fgets(read, BUFSIZ, stdin))
                break;
            printf("Send : %s", read);
            int sentLen = send(socket_peer, read, strlen(read), 0);
            printf("sent %d bytes\n", sentLen);
        }
    }

    printf("Close socket\n");
    CLOSESOCKET(socket_peer);

    printf("finished\n");
}