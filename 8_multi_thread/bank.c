#include <stdio.h>
#include <pthread.h>
#include <fcntl.h>
#include <semaphore.h>

#define SEM_NAME "/HEESU"

static sem_t *sem;

typedef struct
{
    int balance;
} Bank;
static Bank the_bank;
void deposit(int n)
{
    the_bank.balance += n;
}
void withdraw(int n)
{
    if (the_bank.balance >= n)
    {
        the_bank.balance -= n;
    }
}
void *employee(void *args)
{
    for (int i = 0; i < 100; ++i)
    {
        sem_wait(sem);
        deposit(2);
        sem_post(sem);
    }
    for (int i = 0; i < 100; ++i)
    {
        sem_wait(sem);
        withdraw(2);
        sem_post(sem);
    }
    return NULL;
}
int main()
{
    the_bank.balance = 0;
    // Create a thread for 32 employees.
    sem = sem_open(SEM_NAME, O_RDWR | O_CREAT, 0777, 1);

    int N = 100000;
    pthread_t tids[N];
    for (int i = 0; i < N; ++i)
    {
        pthread_create(&tids[i], NULL, &employee, NULL);
    }
    // Wait til each on is done.
    for (int i = 0; i < N; ++i)
    {
        pthread_join(tids[i], NULL);
    }
    printf("Total: %d\n", the_bank.balance);
    sem_unlink(SEM_NAME);
    return 0;
}