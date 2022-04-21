
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

void sigchld_handler(int sig)
{
	int status;
	wait(&status);
	if (WIFEXITED(status))
	{
		printf("%d \n", sig);
		exit(1);
	}
}

int main(int argc, char **argv)
{
	int count = 0;
	int status;
	pid_t pid = fork();

	if (pid == 0) // child process
	{
		printf("A child process is generated.\n");
		count = count + 1;
		printf("Child Process Count: %d \n", count);
		sleep(5);
		return 3;
	}
	else if (pid < 0) // fork error
	{
		printf("Fork Error\n");
		exit(5);
	}
	else // parent process
	{
		signal(SIGCHLD, sigchld_handler);
		while (1)
			;
		// wait(&status);
		// if (WIFEXITED(status))
		// {
		// 	printf("Child process returns %d \n", WEXITSTATUS(status));
		// }
		// printf("This is a parent process.\n");
		// printf("Parent Process Count %d \n", count);
	}

	return 0;
}
