#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void *init_thread(void *param)
{
    int i;
    for (i = 0; i < 5; i++)
    {
        printf("Thread #%d, Counter :%d\n", param, i);
        sleep(1);
    }
    printf("Thread #%d terminated\n", param);
    return NULL;
}

void *init_thread2(void *param)
{
    int i;
    for (i = 0; i < 10; i++)
    {
        printf("Thread #%d, Counter :%d\n", param, i);
        sleep(1);
    }
    printf("Thread #%d terminated\n", param);
    return NULL;
}

int main()
{
    pthread_t tid1, tid2;
    int res;
    if (pthread_create(&tid1, NULL, init_thread, 1) != 0)
    {
        fprintf(stderr, "PThread Creation error\n");
        exit(0);
    }
    if (pthread_create(&tid2, NULL, init_thread2, 2) != 0)
    {
        fprintf(stderr, "PThread Creation error\n");
        exit(0);
    }
    if (pthread_join(tid1, (void **)&res) != 0)
    {
        fprintf(stderr, "Pthread join error\n");
        exit(0);
    }
    if (pthread_join(tid2, (void **)&res) != 0)
    {
        fprintf(stderr, "Pthread2 join error\n");
        exit(0);
    }

    printf("Main terminated\n");
}