#include <stdio.h>
#include <pthread.h>
#include <semaphore.h>
#include <stdlib.h>
#include <fcntl.h>
#include <unistd.h>

#define SEM_NAME "/3_Thread_Sem"

static int counter = 0;
static sem_t *sem;

void *thread1(void *arg)
{
    for (int i = 0; i < 60000; i++)
    {
        sem_wait(sem);
        counter += 1;
        printf("A :%d\n", counter);
        sem_post(sem);
    }
}
void *thread2(void *arg)
{
    for (int i = 0; i < 60000; i++)
    {
        sem_wait(sem);
        counter += 1;
        printf("B :%d\n", counter);
        sem_post(sem);
    }
}
void *thread3(void *arg)
{
    for (int i = 0; i < 60000; i++)
    {
        sem_wait(sem);
        counter += 1;
        printf("C :%d\n", counter);
        sem_post(sem);
    }
}
int main()
{
    pthread_t t1, t2, t3;
    sem = sem_open(SEM_NAME, O_CREAT | O_RDWR, 0777, 1);

    if (pthread_create(&t1, NULL, thread1, NULL) != 0)
    {
        return 1;
    }
    if (pthread_create(&t2, NULL, thread2, NULL) != 0)
    {
        return 1;
    }
    if (pthread_create(&t3, NULL, thread3, NULL) != 0)
    {
        return 1;
    }
    int res = 0;
    if (pthread_join(t1, (void **)&res) != 0)
    {
        fprintf(stderr, "PThread 1 Join Error \n");
        exit(0);
    }
    if (pthread_join(t2, (void **)&res) != 0)
    {
        fprintf(stderr, "PThread 2 Join Error \n");
        exit(0);
    }
    if (pthread_join(t3, (void **)&res) != 0)
    {
        fprintf(stderr, "PThread 3 Join Error \n");
        exit(0);
    }
}