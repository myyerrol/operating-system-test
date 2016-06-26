#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include "file_manager.h"

void closeFile(int file_id, char *file_name)
{
    close(file_id);
}

int openReadFile(int *file_id, char *file_name)
{
    *file_id = open(file_name, O_RDONLY);

    if (*file_id < 0) {
        return FALSE;
    }

    return TRUE;
}

int openWriteFile(int *file_id, char *file_name)
{
    int flag = FALSE;

    if (access(file_name, F_OK) < 0) {
        flag = TRUE;
    }

    *file_id = open(file_name, O_WRONLY|O_CREAT);

    if (*file_id < 0) {
        return FALSE;
    }
    if (flag) {
        chmod(file_name, 0666);
    }

    return TRUE;
}

int readFileData(u_int8_t *buffer, int file_id, int read_len)
{
    int byte_num = 0;

    byte_num = read(file_id, buffer, read_len);
    return byte_num;
}

int writeFileData(u_int8_t *buffer, int file_id, int write_len)
{
    int byte_num = 0;

    byte_num = write(file_id, buffer, write_len);
    return byte_num;
}

long long getFileSize(char *file_name)
{
    struct stat file_stat;

    if (stat(file_name, &file_stat) < 0) {
        return ERROR;
    }

    return file_stat.st_size;
}
