#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/shm.h>
#include "shared_memory.h"
#include "file_manager/src/file_manager.h"
#include "input_output/src/input_output.h"

int main(int argc, char **argv)
{
    int file_id;
    int bytes_num = 0;
    int shared_memory_id;
    void *memory_to_shared = NULL;
    char *file_name = argv[1];
    long long bytes_has_read = 0;
    u_int8_t *buffer = (u_int8_t *)malloc(BUFFER_SIZE);
    MemoryType *shared_memory;

    if (argc < 2) {
        printf("Please use format [shared_send filename]\n");
        exit(EXIT_FAILURE);
    }

    // Open the file, and type is read-only.
    if (!openReadFile(&file_id, file_name)) {
        printf("Error, can not open file!\n");
        exit(EXIT_FAILURE);
    }

    // Create the shared space.
    if ((shared_memory_id = shmget((key_t)11111, sizeof(MemoryType),
                                   0666 | IPC_CREAT)) == -1) {
        printf("Error, can not initial the shared memory!\n");
        exit(EXIT_FAILURE);
    }

    // Get the first address of shared memory.
    if ((memory_to_shared = shmat(shared_memory_id, (void *)0, 0)) ==
        (void *)-1) {
        printf("Error, can not get the shared memory!\n");
        exit(EXIT_FAILURE);
    }

    shared_memory = (MemoryType *)memory_to_shared;
    shared_memory->has_client = FALSE;
    shared_memory->has_data   = TRUE;

    printf("--------------------------\n");
    printf("Process Communication Test\n");
    printf("--------------------------\n");
    printf("Waiting shared receiver...\n");
    printf("Sending data......\n");

    while (TRUE) {
        if (shared_memory->has_client == FALSE) {
            continue;
        }
        if (!shared_memory->has_data) {
            memset(buffer, '\0', BUFFER_SIZE);
            // Read data from file.
            bytes_num = readFileData(buffer, file_id, BUFFER_SIZE);
            bytes_has_read += bytes_num;
            printData(bytes_has_read / CONVERSION_UNIT);
            // Empty the buffer.
            memset(shared_memory->buffer, '\0', BUFFER_SIZE);
            // Write data to shared memory.
            memcpy(shared_memory->buffer, buffer, bytes_num);
            shared_memory->write_byte_num = (bytes_num <= 0) ? -1 : bytes_num;
            shared_memory->has_data = TRUE;
            if (bytes_num <= 0) {
                break;
            }
        }
    }

    printf("\nSending data: %lld bytes\n", bytes_has_read);

    // Separate shared memory from the process.
    if (shmdt((void *)shared_memory) == -1) {
        printf("Error, can not separate shared memory from the process!\n");
        exit(EXIT_FAILURE);
    }

    // Delete shared memory.
    if (shmctl(shared_memory_id, IPC_RMID, 0) == -1) {
        printf("Error, can not delete shared memory!\n");
        exit(EXIT_FAILURE);
    }

    return 0;
}
