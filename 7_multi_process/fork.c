#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main()
{
    int count = 0;
    int status;
    pid_t pid = fork();

    if (pid == 0)
    {
        // child process
        printf("A child process is generated\n");
        count = count + 1;
        printf("Child process count:%d\n", count);
        sleep(15);
        return 3;
    }
    else if (pid < 0)
    {
        printf("Fork Error\n");
        exit(5);
    }
    else
    {
        printf("child PID %d\n", pid);
        wait(&status);
        if (WIFEXITED(status))
        {
            printf("Child process is terminated and returned %d\n", WEXITSTATUS(status));
        }
        else if(WIFSIGNALED(status)){
            printf("child process terminated by signal\n");
        }
    }
    return 0;
}