#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <pthread.h>
#include <sys/types.h>

int g_count = 0;

void *thread_a(void *arg)
{
    printf("This is thread a\n");
    printf("The count is %d\n", ++g_count);
}

void *thread_b(void *arg)
{
    printf("This is thread b\n");
    printf("The count is %d\n", ++g_count);
}

int main(void)
{
    pthread_t pthread_a, pthread_b;

    if(pthread_create(&pthread_a, NULL, thread_a, NULL) != 0) {
        printf("Error, some errors happened when creating thread a!\n");
    }
    if(pthread_create(&pthread_b, NULL, thread_b, NULL) != 0) {
        printf("Error, some errors happened when creating thread a!\n");
    }

    sleep(1);
    return 0;
}
