#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>

void errProc(char *ctxt, int serverSd, int clientSd);

int main(int argc, char **argv)
{
	int serverSd, clientSd;					   //서버 소켓 파일 디스크립터, 클라이언트 소켓 파일 디스크립터
	struct sockaddr_in serverAddr, clientAddr; // 서버 주소, 클라이언트 주소 저장용 변수
	int clientAddrLen, readLen;				   // 클라이언트 주소 정보 길이, 읽어온 바이트 길이 저장용 변수
	char rBuff[BUFSIZ];						   // 버퍼 read용 변수
	const char endWord[] = "!end";			   // 종료 문자열
	pid_t pid;

	if (argc != 2)
	{
		printf("Usage: %s [port]\n", argv[0]);
		return 1;
	}

	serverSd = socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓 생성(서버용)
	if (serverSd == -1)
		errProc("socket", serverSd, clientSd);

	clientAddrLen = sizeof(clientAddr);				// 클라이언트 주소 정보 길이 지정
	memset(&serverAddr, 0, sizeof(serverAddr));		// 서버 주소 정보 초기화
	serverAddr.sin_family = AF_INET;				// IPv4
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY); // Wilcard Address, network byte order로 변환
	serverAddr.sin_port = htons(atoi(argv[1]));		// argument 를 포트 번호로 지정, network byte order로 변환

	if (bind(serverSd, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1) // 서버 소켓에 주소 정보 바인딩
		errProc("bind", serverSd, clientSd);

	if (listen(serverSd, 3) == -1) // listen 시작, 백로그 크기는 3
		errProc("listen", serverSd, clientSd);
	printf("echo server listening\n");

	while (1)
	{
		clientSd = accept(serverSd, (struct sockaddr *)&clientAddr, &clientAddrLen); // 접속 시도하는 클라이언트를 accept, 클라이언트 접속 정보 저장
		if (clientSd == -1)
		{
			fprintf(stderr, "%s error: %s\n", "accept", strerror(errno));
			continue;
		}
		printf("client %s is connected from %d\n", inet_ntoa(clientAddr.sin_addr), ntohs(clientAddr.sin_port)); // 접속된 클라이언트 접속 정보 출력

		pid = fork();
		if (pid == 0)
		{
			// child
			close(serverSd); // 듣기 소켓 필요없고, 서비스 소켓이 주고받기만 잘 하면 됨
			while (1)
			{
				// rBuff[0] = '\0';
				readLen = read(clientSd, rBuff, sizeof(rBuff) - 1);
				if (readLen == 0)
					break;
				rBuff[readLen] = '\0';
				printf("client(%d): %s\n", ntohs(clientAddr.sin_port), rBuff);
				write(clientSd, rBuff, readLen);
			}

			printf("Client(%d): is disconnected\n", ntohs(clientAddr.sin_port));
			close(clientSd);
			return 0;
		}
		else if (pid == -1)
			errProc("fork()", serverSd, clientSd);
		else
		{
			// parent
			close(clientSd); //서비스 소켓은 필요없음
		}
	}
	close(serverSd);
	return 0;
}

void errProc(char *ctxt, int serverSd, int clientSd)
{
	fprintf(stderr, "%s error: %s\n", ctxt, strerror(errno));
	// close(clientSd); // 오류 발생시 적극적으로 connection close
	// close(serverSd);
	exit(1);
}
