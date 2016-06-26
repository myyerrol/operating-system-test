#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <limits.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <sys/unistd.h>
#include "fifo.h"
#include "file_manager/src/file_manager.h"
#include "input_output/src/input_output.h"

int main(int argc, char **argv)
{
    int file_id;
    int pipe_fd;
    int read_bytes;
    int open_mode = O_WRONLY;
    int read_bytes_sum = 0;
    char *file_name = argv[1];
    u_int8_t *buffer = (u_int8_t *)malloc(BUFFER_SIZE);

    if (argc < 2) {
        printf("Please use format [fifo_send filename]\n");
        exit(EXIT_FAILURE);
    }

    if (access(FIFO_NAME, F_OK) == -1) {
        if (mkfifo(FIFO_NAME, 0777) != 0) {
            printf("Error, can not create fifo!\n");
            exit(EXIT_FAILURE);
        }
    }

    printf("--------------------------\n");
    printf("Process Communication Test\n");
    printf("--------------------------\n");
    printf("Waiting fifo receiver.....\n");
    pipe_fd = open(FIFO_NAME, open_mode);

    if (!openReadFile(&file_id, file_name)) {
        printf("Error, can not open file!\n");
        exit(EXIT_FAILURE);
    }

    if (pipe_fd == -1) {
        printf("Error, can not open fifo!\n");
        exit(EXIT_FAILURE);
    }

    printf("Sending data......\n");

    do {
        read_bytes = readFileData(buffer, file_id, BUFFER_SIZE);
        if (write(pipe_fd, (char *)buffer, read_bytes) < 0) {
            printf("Error, can not write!\n");
        }
        read_bytes_sum += read_bytes;
        printData(read_bytes_sum / CONVERSION_UNIT);
    } while (read_bytes > 0);

    printf("\nSend bytes: %d\n", read_bytes_sum);
    printf("File size: %lld MB\n", getFileSize(file_name) / CONVERSION_UNIT);
    closeFile(file_id, file_name);
    return 0;
}
