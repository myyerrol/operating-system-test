#include <stdio.h>
#include "input_output.h"

void printData(int data)
{
    int i;
    int count = 1;
    int temp  = data;

    printf("\b\b\b");

    while (temp) {
        temp = temp / 10;
        count++;
    }

    for (i = 0; i < count; i++) {
        printf("\b");
    }

    printf("%d MB", data);
}
