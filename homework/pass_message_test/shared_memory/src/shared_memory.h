#ifndef SHARED_MEMORY_H
#define SHARED_MEMORY_H

#include <sys/types.h>

#define CONVERSION_UNIT 1000000
#define BUFFER_SIZE     1024 * 4
#define TRUE            1
#define FALSE           0

typedef struct MemoryType {
    long long write_byte_num;
    int       has_data;
    int       has_client;
    u_int8_t  buffer[BUFFER_SIZE];
} MemoryType;

#endif // SHARED_MEMORY_H
