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

// 소프트웨어학부 20180288 박성철 과제 4

void errProc(char *ctxt, int errorSd);

fd_set defaultFds, rFds; // 멀티 플렉싱을 위한 파일 디스크립터 셋, 전역변수 이므로 초기화 불필요(0)
int maxFd = 0;           // 최대 파일 디스크립터 번호
int listenSd;            // 리스닝 소켓

void openServer(char *port) // 입력된 포트로 listen 소켓을 여는 함수
{
    struct sockaddr_in srvAddr;

    printf("서버 시작\n");
    listenSd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
    if (listenSd == -1)
        printf("socket error");

    memset(&srvAddr, 0, sizeof(srvAddr));
    srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
    srvAddr.sin_family = AF_INET;
    srvAddr.sin_port = htons(atoi(port));

    if (bind(listenSd, (struct sockaddr *)&srvAddr, sizeof(srvAddr)) == -1)
        printf("bind error\n");
    if (listen(listenSd, 5) < 0)
        printf("listen error\n");

    FD_SET(listenSd, &defaultFds); // 리스닝 소켓을 파일 디스크립터 셋에 등록
    maxFd = listenSd;              // 최대 파일 디스크립터 번호 갱신
}

void sendToAll(char *buff, int from, int len) // 수신 내용을 송신자 외 전체에 발송하는 함수
{
    buff[len - 1] = from - 3;           // 버퍼의 마지막 글자(NULL)을 송신자의 번호로 설정 (범위 -2^8 ~ 2^8)
    for (int i = 0; i < maxFd + 1; i++) // 현재까지의 소켓 중
    {
        if (FD_ISSET(i, &defaultFds) && i != listenSd && i != from) // 송신자와 리스닝 소켓을 제외한 소켓
        {
            write(i, buff, len); // 메시지 송신
        }
    }
}

void sendNoticeToAll(int state, int from) // 접속, 접속 종료, 접속자 식별 등 공지 전송하는 함수
{
    // state
    // 0: 접속종료, 1: 접속, 2: 접속자에게 접속자 번호 알림

    char buff[BUFSIZ + 1]; // 메시지 문자열
    switch (state)         // 상태에 맞게 메시지 문자열에 내용 복사
    {
    case 0:
        strcpy(buff, "가 종료했습니다");
        break;
    case 1:
        strcpy(buff, "가 접속했습니다");
        break;
    case 2:
        strcpy(buff, "가 당신입니다");
        break;
    }

    int length = strlen(buff);
    buff[length] = from - 3; // 버퍼의 마지막 글자(NULL)을 송신자의 번호로 설정 (범위 -2^8 ~ 2^8)

    for (int sd = 0; sd < maxFd + 1; sd++) // 현재까지의 소켓 중
    {
        if (FD_ISSET(sd, &defaultFds) && sd != listenSd) // 리스닝 소켓을 제외한 소켓
        {
            if (state == 2 && sd != from) // 접속자 식별 메시지의 경우 접속자가 아니라면 건너 뜀
                continue;
            else if (state != 2 && sd == from) // 그외의 경우에는 접속자인 경우 건너 뜀
                continue;
            write(sd, buff, length + 1); // 공지 메시지 송신
        }
    }
}

void loop()
{
    int readLen;
    char rBuff[BUFSIZ];
    struct sockaddr_in clntAddr;
    int clntAddrLen;
    while (1)
    {
        rFds = defaultFds;
        if (select(maxFd + 1, &rFds, 0, 0, NULL) == -1) // 관심 대상 파일 디스크립터 중 읽을 내용이 있는 파일 디스크립터 rFds에 저장
            break;
        for (int i = 0; i < maxFd + 1; i++)
        {
            if (FD_ISSET(i, &rFds)) // 읽을 내용이 있는 경우
            {
                if (i == listenSd) // 리스닝 소켓인 경우
                {
                    int connectSd = accept(listenSd, (struct sockaddr *)&clntAddr, &clntAddrLen); // accept
                    printf(">> 사용자(%d) 접속\n", connectSd - 3);
                    if (connectSd == -1)
                    {
                        printf("Accept Error");
                        continue;
                    }
                    FD_SET(connectSd, &defaultFds); // 관심 대상에 추가
                    if (maxFd < connectSd)          // 최대 파일 디스크립터 번호 갱신
                        maxFd = connectSd;
                    sendNoticeToAll(2, connectSd); // 접속자에게 사용자 번호 알림 메시지 발송
                    sendNoticeToAll(1, connectSd); // 접속자 외의 사람들에게 사용자 접속 메시지 발송
                }
                else // 리스닝 소켓이 외의 어떤 소켓인 경우
                {
                    readLen = read(i, rBuff, sizeof(rBuff) - 1);
                    if (readLen <= 0) // 접속 종료
                    {
                        printf(">> 사용자(%d) 접속종료\n", i - 3);
                        FD_CLR(i, &defaultFds); // 관심 대상 제거
                        close(i);               // 소켓 파일 디스크립터 닫기
                        sendNoticeToAll(0, i);  // 본인을 제외한 전체에게 접속 종료 메세지 발송
                        continue;
                    }
                    rBuff[readLen] = '\0';

                    printf("사용자(%d): %s\n", i - 3, rBuff);
                    sendToAll(rBuff, i, readLen + 1); // 수신한 내용을 송신자 외 전체에 발송
                }
            }
        }
    }
}

int main(int argc, char **argv)
{

    if (argc != 2)
    {
        printf("Usage: %s [port]\n", argv[0]);
        return 1;
    }

    openServer(argv[1]); // 포트 번호를 프로그램 Argument 를 통해 받아 리스닝 소켓을 염
    loop();              // 파일 디스크립터 셋 읽기 반복

    return 0;
}

void errProc(char *ctxt, int errorSd)
{
    fprintf(stderr, "%s error on: %s\n", ctxt, strerror(errno));
    if (errorSd >= 0)
        close(errorSd);
    exit(1);
}