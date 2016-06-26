#include <stdio.h>
#include "time_counter.h"

clock_t g_start_time;
clock_t g_end_time;

void startTimeCounter(void)
{
    g_start_time = clock();
}

void endTimeCounter(void)
{
    g_end_time = clock();
}

void calculateTimeDuration(void)
{
    double duration = (double)(g_end_time - g_start_time) / CLOCKS_PER_SEC;
    printf("Run Time: %lf seconds\n", duration);
}
