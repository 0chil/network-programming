#include <stdio.h>
#include <pthread.h>
#include <stdlib.h>
#include <unistd.h>

void *init_thread(void *parm)
{
    int i;
    for (i = 0; i < 10; i++)
    {
        printf("#%d, Counter :%d \n", (int)(parm), i);
        sleep(1);
    }
    printf("Thread is terminated\n");
}

int main(int argc, char** argv){
    pthread_t thread_id, tid2;

    if(pthread_create(&thread_id, NULL, init_thread, 1) != 0){
        fprintf(stderr, "PThread Creation Error \n");
        exit(0);
    }
    if(pthread_create(&tid2, NULL, init_thread, 2) != 0){
        fprintf(stderr, "PThread Creation Error \n");
        exit(0);
    }
    printf("thread id : %d and %d\n", thread_id, tid2);

    sleep(5);

    printf("Main function is terminated\n");
    return 0;

}