#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

int main()
{
    pid_t current_pid;
    int count = 1;

    current_pid = fork();

    if (current_pid < 0) {
        printf("Error, some errors happened when creating process!\n");
        exit(EXIT_FAILURE);
    }
    else if (current_pid == 0) {
        printf("Child  process is created!\n");
        printf("Child's  count is %d\n", ++count);
    }
    else {
        printf("Father process is created!\n");
        printf("Father's count is %d\n", ++count);
    }

    printf("The current pid = %d\n", current_pid);
}
