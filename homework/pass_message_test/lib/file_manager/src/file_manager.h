#ifndef FILE_MANAGER_H
#define FILE_MANAGER_H

#include <sys/types.h>

#define TRUE   1
#define FALSE  0
#define ERROR -1

void closeFile(int file_id, char *file_name);
int openReadFile(int *file_id, char *file_name);
int openWriteFile(int *file_id, char *file_name);
int readFileData(u_int8_t *buffer, int file_id, int read_len);
int writeFileData(u_int8_t *buffer, int file_id, int write_len);
long long getFileSize(char *file_name);

#endif // FILE_MANAGER_H
