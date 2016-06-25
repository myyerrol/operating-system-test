#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    int i;

    printf("Current pid = %d\n", getpid());

    for(i = 0; i < 2; i++) {
        pid_t current_pid = fork();
        if (current_pid < 0) {
            printf("Error, some errors happened when creating process!\n");
            exit(EXIT_FAILURE);
        }
        else if (current_pid == 0) {
            printf("i = %d Child's  process: pid = %d father's pid = %d ", i,
                getpid(), getppid());
            printf("current pid = %d\n",  current_pid);
        }
        else {
            printf("i = %d Father's process: pid = %d father's pid = %d ", i,
                getpid(), getppid());
            printf("current pid = %d\n",  current_pid);
        }
    }

    printf("End pid = %d\n", getpid());
    return 0;
}
