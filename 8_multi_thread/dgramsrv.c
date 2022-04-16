#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <errno.h>
#include <arpa/inet.h>
#include <netdb.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() errno

void toupper(char *str, int len)
{
    for (int i = 0; i < len; i++)
    {
        str[i] = str[i] - 'a' + 'A';
    }
}

int main()
{
    printf("Configuring local address...\n");
    struct addrinfo hints;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_DGRAM;
    hints.ai_flags = AI_PASSIVE;
    struct addrinfo *bind_address;
    getaddrinfo(0, "8888", &hints, &bind_address);
    printf("Creating socket...\n");
    SOCKET socket_listen;
    socket_listen = socket(bind_address->ai_family,
                           bind_address->ai_socktype, bind_address->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen))
    {
        fprintf(stderr, "socket() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    printf("Binding socket to local address...\n");
    if (bind(socket_listen,
             bind_address->ai_addr, bind_address->ai_addrlen))
    {
        fprintf(stderr, "bind() failed. (%d)\n", GETSOCKETERRNO());
        return 1;
    }
    freeaddrinfo(bind_address);

    fd_set dFds, rFds;
    int maxFd = socket_listen;
    FD_ZERO(&dFds);
    FD_SET(socket_listen, &dFds);

    printf("Waiting for connections...\n");
    while (1)
    {
        rFds = dFds;
        if (select(maxFd + 1, &rFds, 0, 0, NULL) == -1)
            break;
        for (int i = 0; i <= maxFd; i++)
        {
            if (FD_ISSET(i, &rFds) && i == socket_listen)
            {
                char rBuff[BUFSIZ];
                struct sockaddr_in client;
                int clientSz = sizeof(client);
                int readLen = recvfrom(i, rBuff, BUFSIZ - 1, 0, (struct sockaddr *)&client, &clientSz);
                if (readLen < 0)
                {
                    break;
                }
                rBuff[readLen] = '\0';
                printf("Received : %s\n", rBuff);
                toupper(rBuff, readLen);
                sendto(i, rBuff, readLen - 1, 0, (struct sockaddr *)&client, clientSz);
            }
        }
    }
    printf("Closing listening socket...\n");
    CLOSESOCKET(socket_listen);
    printf("Finished.\n");
    return 0;
}