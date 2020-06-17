#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>

#define NUM_WORKERS 20
#define LOAD_BALANCING_DFLT 0
#define THRESHHOLD_DOORS_DFLT 2000

static int total_knobs = 0;
static int total_doors = 0;
static int seconds_elapsed = 0;

int main(int argc, char **argv) {
    char *strtoll_check;
    int load_balancing = LOAD_BALANCING_DFLT;
    int threshhold_doors = THRESHHOLD_DOORS_DFLT;
    pthread_t workers[NUM_WORKERS];

    if (argc < 3) {
        fprintf(stderr, "Usage: ./factory <enable load balancing> <number of doors to produce>\n");
        return EXIT_FAILURE;
    }

    if (argv[1][0] == '1') load_balancing = 1;

    // convert parameter 'threshhold_doors' to an integer
    threshhold_doors = strtoll(argv[2], &strtoll_check, 10);
    if (*strtoll_check != '\0') {
        fprintf(stderr, "strtoll(): Could not convert to integer\n");
        return EXIT_FAILURE;
    }

    printf("load: %d\n", load_balancing);
    printf("doors: %d\n", threshhold_doors);
    
    printf("workers: %p\n", workers);
    printf("seconds_elapsed: %d\n", seconds_elapsed);
    printf("total_doors: %d\n", total_doors);
    printf("total_knobs: %d\n", total_knobs);

    return EXIT_SUCCESS;
}