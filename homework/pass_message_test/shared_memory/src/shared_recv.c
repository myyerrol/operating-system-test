#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include "shared_memory.h"
#include "file_manager/src/file_manager.h"
#include "time_counter/src/time_counter.h"
#include "input_output/src/input_output.h"

int main(int argc, char **argv)
{
    int file_id;
    int flag = FALSE;
    int shared_memory_id;
    void *memory_to_shared = NULL;
    char *file_name = argv[1];
    long long bytes_has_read = 0;
    MemoryType *shared_memory;

    if (argc < 2) {
        printf("Please use format [shared_send filename]\n");
        exit(EXIT_FAILURE);
    }

    // Open the file, and type is write-read.
    if (!openWriteFile(&file_id, file_name)) {
        printf("Error, can not open file!\n");
        exit(EXIT_FAILURE);
    }

    // Create the shared space.
    if ((shared_memory_id = shmget((key_t)11111, sizeof(MemoryType),
                                   0666 | IPC_CREAT)) == -1) {
        printf("Error, can not get the shared memory id!\n");
        exit(EXIT_FAILURE);
    }

    // Get the first address of shared memory.
    if ((memory_to_shared = shmat(shared_memory_id, (void *)0, 0)) ==
        (void *)-1) {
        printf("Error, can not get the shared memory!\n");
        exit(EXIT_FAILURE);
    }

    shared_memory = (MemoryType *)memory_to_shared;
    shared_memory->has_client = TRUE;

    printf("--------------------------\n");
    printf("Process Communication Test\n");
    printf("--------------------------\n");
    printf("Waiting shared sender.....\n");
    printf("Receiving data......\n");

    while (TRUE) {
        if (shared_memory->has_data) {
            // Start the timer.
            if (!flag) {
                startTimeCounter();
                flag = TRUE;
            }
            // Receive over.
            if (shared_memory->write_byte_num == -1) {
                printf("\nReceiving data: %lld bytes\n", bytes_has_read);
                break;
            }
            bytes_has_read += shared_memory->write_byte_num;
            printData(bytes_has_read / CONVERSION_UNIT);
            // Write data to shared memory.
            writeFileData(shared_memory->buffer, file_id,
                          shared_memory->write_byte_num);
            shared_memory->has_data = FALSE;
        }
    }

    endTimeCounter();
    calculateTimeDuration();

    // Separate shared memory from the process.
    if (shmdt((void *)shared_memory) == -1) {
        printf("Error, can not separate shared memory from the process!\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
