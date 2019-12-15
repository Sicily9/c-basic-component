#include <sys/time.h>
#include <stdio.h>

long long timeInMilliseconds(void) {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (((long long) tv.tv_sec) * 1000) + (tv.tv_usec / 1000);
}

long long timeInMicroseconds(void) {
    struct timeval tv;

    gettimeofday(&tv, NULL);
    return (((long long) tv.tv_sec) * 1000000) + tv.tv_usec;
}

#define start_benchmark() long long start = timeInMilliseconds();
#define end_benchmark(msg) do { \
    long long elapsed = timeInMilliseconds()-start; \
    printf(msg ": %lld ms\n", elapsed); \
} while(0);
