#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>
#include <unistd.h>
#include <pthread.h>

// 소프트웨어학부 20180288 박성철 과제 4

void errProc(char *ctxt, int clientSd);

int clientSd; // 소켓 파일 디스크립터

void *reader(void *arg) // 별도의 스레드가 실행할 함수
{
	char rBuff[BUFSIZ];
	int readLen;
	while (1)
	{
		readLen = read(clientSd, rBuff, sizeof(rBuff) - 1);
		if (readLen <= 0) // 서버와의 연결 종료 시 클라이언트 종료
		{
			fprintf(stderr, "server down\n");
			exit(0);
		}
		int userNumber = (int)rBuff[readLen - 1];	   // 버퍼의 마지막 글자는 송신자의 번호 (server.c 참고) (범위 -2^8 ~ 2^8)
		rBuff[readLen - 1] = '\0';					   // 사용자 번호를 저장했으므로 출력을 위해 NULL로 변경
		printf("사용자(%d): %s\n", userNumber, rBuff); // 읽어온 문자열 출력
	}
}

int main(int argc, char **argv)
{
	struct sockaddr_in clientAddr; // 클라이언트가 접속할 주소 저장용 변수
	int readLen;				   // 입력한 글자 길이
	char wBuff[BUFSIZ];			   // 사용자 입력 저장용 변수
	const char endWord[] = "!end"; // 종료 문자열

	if (argc != 3)
	{
		printf("Usage: %s [IPv4 Address] [port]\n", argv[0]);
		return 1;
	}

	clientSd = socket(AF_INET, SOCK_STREAM, 0); // TCP 소켓 생성
	if (clientSd == -1)
		errProc("socket", clientSd);

	memset(&clientAddr, 0, sizeof(clientAddr));		 // 주소 담을 변수 초기화
	clientAddr.sin_family = AF_INET;				 // IPv4
	clientAddr.sin_addr.s_addr = inet_addr(argv[1]); // argument를 IP주소로 사용, network byte order로 변환
	clientAddr.sin_port = htons(atoi(argv[2]));		 // argument를 Port 로 사용, network byte order로 변환

	if (connect(clientSd, (struct sockaddr *)&clientAddr, sizeof(clientAddr)) == -1) // 해당 주소로 접속 시도
		errProc("connect", clientSd);
	printf("connected to %s:%d\n", inet_ntoa(clientAddr.sin_addr), htons(clientAddr.sin_port)); // 접속 정보 출력

	pthread_t thread;
	pthread_create(&thread, NULL, reader, NULL); // 수신 버퍼 읽기를 별도의 스레드로 실행

	while (1)
	{
		// 전송하기
		fgets(wBuff, BUFSIZ - 1, stdin); // 사용자 입력 받기
		readLen = strlen(wBuff);		 // 사용자 입력 길이 저장
		wBuff[readLen] = '\0';
		if (readLen < 2)
			continue;						 // 개행문자 외 아무것도 입력되지 않았으면 다시 입력받음
		write(clientSd, wBuff, readLen - 1); // 전송

		if (strcmp(wBuff, endWord) == 0) // 읽어온 문자열이 종료 문자열이면 종료 단계로 들어감
			break;
	}
	close(clientSd); // 커넥션 종료
	printf("connection closed\n");
	return 0;
}

void errProc(char *ctxt, int clientSd)
{
	fprintf(stderr, "%s error: %s\n", ctxt, strerror(errno));
	close(clientSd); // 오류가 있을 경우 적극적으로 소켓 close
	exit(1);
}
