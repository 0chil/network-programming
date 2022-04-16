#include <stdio.h>
#include <sys/socket.h>
#include <stdlib.h>
#include <errno.h>
#include <netinet/in.h>
#include <string.h>
#include <arpa/inet.h>
#include <sys/types.h>

void errProc(char *ctxt, int clientSd);

int main(int argc, char **argv)
{
	int clientSd;				   // 소켓 파일 디스크립터
	struct sockaddr_in clientAddr; // 클라이언트가 접속할 주소 저장용 변수
	int readLen, recvByte, maxBuff;
	// 입력한 글자 길이, 읽어온 바이트 길이, 버퍼 최대 길이 저장용 변수
	char rBuff[BUFSIZ], wBuff[BUFSIZ]; // 버퍼 read, write용 변수
	const char endWord[] = "!end";	   // 종료 문자열

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

	while (1)
	{
		// 전송하기
		fgets(wBuff, BUFSIZ - 1, stdin);  // 사용자 입력 받기
		readLen = strlen(wBuff);		  // 사용자 입력 길이 저장
		wBuff[readLen - 1] = '\0';        // 개행문자 널로 대체
		readLen--;						  // 널로 대체되었으므로 길이를 줄임
		if (readLen < 2)
			continue;					  // 개행문자 외 아무것도 입력되지 않았으면 다시 입력받음
		write(clientSd, wBuff, readLen);  // 전송

		// 수신하기
		maxBuff = BUFSIZ - 1; // 버퍼 최대 길이 지정
		recvByte = 0;		  // 현재 읽어온 길이 지정
		do
		{
			recvByte += read(clientSd, rBuff, maxBuff);
			maxBuff -= recvByte;
		} while (recvByte < (readLen - 1)); // 중간에 널 문자 있을 경우 대비해 읽어올 내용의 길이가 0일때까지 버퍼를 읽어옴
		rBuff[recvByte] = '\0';             // 마지막 글자 널 문자 (출력용)
		printf("server: %s\n", rBuff);      // 읽어온 문자열 출력
		if (strcmp(wBuff, endWord) == 0)    // 읽어온 문자열이 종료 문자열이면 종료 단계로 들어감
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
