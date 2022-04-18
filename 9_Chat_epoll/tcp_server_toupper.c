#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <unistd.h>
#include <errno.h>
#include <stdio.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

#define ISVALIDSOCKET(s) ((s) >= 0)
#define CLOSESOCKET(s) close(s)
#define SOCKET int
#define GETSOCKETERRNO() (errno)

int main()
{
    struct addrinfo hints, *local;
    memset(&hints, 0, sizeof(hints));
    hints.ai_family = AF_INET;
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, "8080", &hints, &local) != 0)
    {
        fprintf(stderr, "getaddrinfo() error\n");
        return 1;
    }

    SOCKET socket_listen = socket(local->ai_family, local->ai_socktype, local->ai_protocol);
    if (!ISVALIDSOCKET(socket_listen))
    {
        fprintf(stderr, "socket() error\n");
        return 1;
    }

    if (bind(socket_listen, local->ai_addr, local->ai_addrlen) != 0)
    {
        fprintf(stderr, "bind() error\n");
        return 1;
    }

    if (listen(socket_listen, 10) < 0)
    {
        fprintf(stderr, "listen() error\n");
        return 1;
    }

    fd_set dFds, rFds;
    FD_ZERO(&dFds);
    FD_SET(socket_listen, &dFds);
    int maxFds = socket_listen;

    while (1)
    {
        rFds = dFds;
        if (select(maxFds + 1, &rFds, 0, 0, NULL) < 0)
        {
            fprintf(stderr, "select() %d error\n", GETSOCKETERRNO());
            return 1;
        }

        for (int i = 0; i < maxFds + 1; i++)
        {
            if (FD_ISSET(i, &rFds))
            {
                if (i == socket_listen)
                {
                    // accept
                    struct sockaddr_storage clntAddr;
                    int clntAddrLen = sizeof(clntAddr);
                    int newFd = accept(i, (struct sockaddr *)&clntAddr, &clntAddrLen);
                    if (newFd < 0)
                    {
                        fprintf(stderr, "accept() error\n");
                        continue;
                    }
                    FD_SET(newFd, &dFds);
                    maxFds = maxFds < newFd ? newFd : maxFds;

                    char address[100], service[100];
                    getnameinfo((struct sockaddr *)&clntAddr, clntAddrLen, address, sizeof(address), service, sizeof(service), NI_NUMERICHOST | NI_NUMERICSERV);
                    printf("%s:%s connected\n", address, service);
                }
                else
                {
                    // read
                    char rBuff[BUFSIZ];
                    int readLen = recv(i, rBuff, BUFSIZ - 1, 0);
                    if (readLen == 0)
                    {
                        printf("Disconnected(%d) \n", i);
                        FD_CLR(i, &dFds);
                        CLOSESOCKET(i);
                        continue;
                    }
                    rBuff[readLen] = '\0';
                    for (int j = 0; j < readLen; j++)
                    {
                        rBuff[j] = toupper(rBuff[j]);
                    }
                    send(i, rBuff, readLen, 0);
                }
            }
        }
    }
    CLOSESOCKET(socket_listen);
    return 0;
}