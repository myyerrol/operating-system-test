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
#include "time_counter/src/time_counter.h"
#include "input_output/src/input_output.h"

int main(int argc, char **argv)
{
    int file_id;
    int pipe_fd;
    int read_bytes;
    int open_mode = O_RDONLY;
    int read_bytes_sum = 0;
    char *file_name = argv[1];
    u_int8_t *buffer = (u_int8_t *)malloc(BUFFER_SIZE);

    if (argc < 2) {
        printf("Please use format [fifo_send filename]\n");
        exit(EXIT_FAILURE);
    }

    printf("--------------------------\n");
    printf("Process Communication Test\n");
    printf("--------------------------\n");
    printf("Waiting fifo sender.......\n");
    pipe_fd = open(FIFO_NAME, open_mode);

    // Open the file, and type is write-read.
    if (!openWriteFile(&file_id, file_name)) {
        printf("Error, can not open file!\n");
        exit(EXIT_FAILURE);
    }

    if (pipe_fd == -1) {
        printf("Error, can not open fifo!\n");
        exit(EXIT_FAILURE);
    }

    printf("Receiving data......\n");
    startTimeCounter();

    do {
        // Read data from fifo.
        read_bytes = read(pipe_fd, buffer, BUFFER_SIZE);
        // Write data to file.
        if (writeFileData(buffer, file_id, read_bytes) < 0) {
            printf("Error, can not write!\n");
        }
        memset(buffer, '\0', read_bytes);
        read_bytes_sum += read_bytes;
       printData(read_bytes_sum / CONVERSION_UNIT);
    } while (read_bytes > 0);

    endTimeCounter();
    printf("\nReceiving data: %d bytes\n", read_bytes_sum);
    calculateTimeDuration();
    closeFile(file_id, file_name);
    return 0;
}
