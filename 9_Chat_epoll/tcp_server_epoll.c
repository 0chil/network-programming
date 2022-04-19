#include <stdio.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <unistd.h>
#include <sys/time.h>
#include <sys/epoll.h>
#include <errno.h>
#include <netdb.h>
#include <arpa/inet.h>

#define MAX_EVENTS 10

void errProc(const char *str)
{
    fprintf(stderr, "%s: %s", str, strerror(errno));
    exit(1);
}

int main(int argc, char **argv)
{
    if (argc != 2)
    {
        printf("Usage: %s [port] \n", argv[0]);
        return 1;
    }

    printf("get local interface...\n");

    struct addrinfo hints, *local;
    memset(&hints, 0, sizeof(hints));
    hints.ai_socktype = SOCK_STREAM;
    hints.ai_flags = AI_PASSIVE;
    if (getaddrinfo(NULL, argv[1], &hints, &local) != 0)
        errProc("getaddrinfo");

    int epfd, ready, readFd;
    struct epoll_event ev;
    struct epoll_event events[MAX_EVENTS];

    printf("server start...\n");

    epfd = epoll_create(1);
    if (epfd == -1)
        errProc("epoll_create");

    int listenSd = socket(local->ai_family, local->ai_socktype, local->ai_protocol);
    if (listenSd == -1)
        errProc("socket");

    if (bind(listenSd, local->ai_addr, local->ai_addrlen) != 0)
        errProc("bind");

    if (listen(listenSd, 5) < 0)
        errProc("listen");

    // 모니터링 할 이벤트 종류, FD
    ev.events = EPOLLIN;
    ev.data.fd = listenSd;
    if (epoll_ctl(epfd, EPOLL_CTL_ADD, listenSd, &ev) == -1)
        errProc("epoll_ctl");

    struct sockaddr_in clntAddr;
    int clntAddrLen = sizeof(clntAddr);
    char rBuff[BUFSIZ];

    while (1)
    {
        printf("Monitoring EPOLLIN...\n");
        ready = epoll_wait(epfd, events, MAX_EVENTS, -1); // Blocking wait
        if (ready == -1 && errno != EINTR)
            errProc("epoll_wait");
        for (int i = 0; i < ready; i++)
        {
            if (events[i].data.fd == listenSd)
            {
                // accept requested
                int connectSd = accept(listenSd, (struct sockaddr *)&clntAddr, &clntAddrLen);
                if (connectSd == -1)
                {
                    fprintf(stderr, "accept error\n");
                    continue;
                }
                printf("%s :%d connected...\n", inet_ntoa(clntAddr.sin_addr), ntohs(clntAddr.sin_port));
                ev.data.fd = connectSd;
                if (epoll_ctl(epfd, EPOLL_CTL_ADD, connectSd, &ev) == -1)
                    errProc("epoll_ctl");
            }
            else
            {
                // IO
                readFd = events[i].data.fd;
                int readLen = read(readFd, rBuff, BUFSIZ - 1);
                if (readLen == 0)
                {
                    // disconnected , FIN received.
                    fprintf(stderr, "Client(%d) disconnected\n", readFd);
                    if (epoll_ctl(epfd, EPOLL_CTL_DEL, readFd, &ev) == -1) // 관심대상에서 빼기
                        errProc("epoll_ctl(delete)");
                    close(readFd); //소켓 닫기
                    continue;
                }
                rBuff[readLen] = '\0';
                printf("Client(%d): %s\n", readFd, rBuff);
                write(readFd, rBuff, strlen(rBuff));
            }
        }
    }
    close(listenSd);
    close(epfd);
    return 0;
}
 