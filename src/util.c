#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <sys/time.h>
#include <sys/resource.h>

#include "util.h"

void alloc_failure() {
    fprintf(stderr, "memory allocation failure\n");
    exit(1);
}

double get_time() {
    struct timeval t;
    gettimeofday(&t, NULL);
    return ((double)t.tv_sec) + ((double)t.tv_usec)*1e-6;
}

int rand_range(int n) {
    uint32_t x;

    uint32_t max_val = (0xffffffff / n) * n;

    FILE *f = fopen("/dev/urandom", "r");

    while (1) {
        if (fread(&x, sizeof(uint32_t), 1, f) != 1) {
            perror("could not read from /dev/urandom. aborting.");
            exit(1);
        }
        if (x > max_val) continue;

        fclose(f);
        return x % n;
    }
}

int *shuffled_range(int n) {
    int i,j;

    int *shuffled = calloc(sizeof(int), n);
    for (i=0; i < n; i++) {
        j = rand_range(i+1);

        shuffled[i] = shuffled[j];
        shuffled[j] = i;
    }

    return shuffled;
}
