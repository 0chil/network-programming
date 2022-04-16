#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <string.h>
#include <errno.h>
#include <stdlib.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>

void errProc();
void errPrint();

void sigchld_handler(int sig)
{
	// SIGCHLD가 발생한 상황
	int status;
	wait(&status); // child 프로세스의 변화를 기다린다.
	if( WIFEXITED(status) ){ // child 프로세스가 정상 종료한 경우
		printf("child process terminated with %d!\n", WEXITSTATUS(status)); // 메시지 출력
	}
}

int main(int argc, char **argv)
{
	int srvSd, clntSd;
	struct sockaddr_in srvAddr, clntAddr;
	int clntAddrLen, readLen, strLen;
	char rBuff[BUFSIZ];
	pid_t pid;

	if (argc != 2)
	{
		printf("Usage: %s [port] \n", argv[0]);
		exit(1);
	}
	printf("Server start...\n");

	// Registering SIGCHLD handler
	struct sigaction new_action; // 새로운 핸들러를 지정할 sigaction
	new_action.sa_handler = sigchld_handler; // 핸들러로 sigchld_handler를 지정한다.
	sigemptyset(&new_action.sa_mask); // 마스킹 없음
	new_action.sa_flags = 0; // 플래그 없음

	sigaction(SIGCHLD, &new_action, NULL); // SIGCHLD에 대해 새로운 핸들러를 등록한다, 기존 sigaction은 저장하지 않는다.

	srvSd = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	if (srvSd == -1)
		errProc("socket");

	memset(&srvAddr, 0, sizeof(srvAddr));
	srvAddr.sin_addr.s_addr = htonl(INADDR_ANY);
	srvAddr.sin_family = AF_INET;
	srvAddr.sin_port = htons(atoi(argv[1]));

	if (bind(srvSd, (struct sockaddr *)&srvAddr, sizeof(srvAddr)) == -1)
		errProc("bind");
	if (listen(srvSd, 5) < 0)
		errProc("listen");
	clntAddrLen = sizeof(clntAddr);
	while (1)
	{
		clntSd = accept(srvSd, (struct sockaddr *)&clntAddr, &clntAddrLen);
		if (clntSd == -1)
		{
			errPrint("accept");
			continue;
		}
		printf("client %s:%d is connected...\n",
			   inet_ntoa(clntAddr.sin_addr),
			   ntohs(clntAddr.sin_port));
		pid = fork();
		if (pid == 0)
		{ /* child process */
			close(srvSd);
			while (1)
			{
				readLen = read(clntSd, rBuff, sizeof(rBuff) - 1);
				if (readLen == 0)
					break;
				rBuff[readLen] = '\0';
				printf("Client(%d): %s\n",
					   ntohs(clntAddr.sin_port), rBuff);
				write(clntSd, rBuff, strlen(rBuff));
			}
			printf("Client(%d): is disconnected\n",
				   ntohs(clntAddr.sin_port));
			close(clntSd);
			return 0;
		}
		else if (pid == -1)
			errProc("fork");
		else
		{ /*Parent Process*/
			close(clntSd);
		}
	}
	close(srvSd);
	return 0;
}

void errProc(const char *str)
{
	fprintf(stderr, "%s: %s \n", str, strerror(errno));
	exit(1);
}

void errPrint(const char *str)
{
	// if(strcmp(str, "accept") == 0 && errno == EINTR) return;
	fprintf(stderr, "%s: %s \n", str, strerror(errno));
}