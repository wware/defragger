#include <stdio.h>
#include <time.h>
#include <stdlib.h>
#include <string.h>
#define COMMENTS_ENABLED
#include "common.h"
#include "mem.h"

#define MAX_SIZE   (8 * 1024)
#define N   (1024 * 1024)
void *things[N];

#define MALLOC(size)   malloc(size)
#define FREE(ptr)      free(ptr)


void report_memory()
{
    printf("%.3lf\n", physical_memory());
}

int main(void)
{
    unsigned long i, size;
    void *p = NULL, *q;
    srand(time(NULL));

    COMMENT("Baseline memory usage.");
    report_memory();

    COMMENT("Allocate a lot of small chunks of memory. Python loves to do this.");
    for (i = 0; i < N; i++) {
        size = 113 * N + rand();
        size %= MAX_SIZE;
        size &= (unsigned long) -8;
        if (size == 0)
            size = 8;
        q = MALLOC(size);
        if (q == NULL) {
            printf("Out of memory\n");
            return 1;
        }
        memset(q, (char) i, size);
        *((void **) q) = p;
        p = q;
    }

    COMMENT("Free all those small chunks of memory. Ideally we get all our memory back.");
    while (p != NULL) {
        q = *((void **) p);
        FREE(p);
        p = q;
    }

    COMMENT("Weep for the bytes that sacrificed themselves to run such useless code.");
    report_memory();
    return 0;
}
