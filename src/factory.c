#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>

#define NUM_WORKERS 20
#define LOAD_BALANCING_DFLT 0
#define THRESHHOLD_DOORS_DFLT 2000

#define WORK_KNOBS 0
#define WORK_DOORS 1

/** handle pthread errors */
#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

static int total_knobs = 0;
static int total_doors = 0;
static int seconds_elapsed = 0;

struct worker {
    pthread_t thread;
    float lazyness;
    int work_type;
};

static void *worker_routine(void *arg);

int main(int argc, char **argv) {
    char *strtoll_check;
    int load_balancing = LOAD_BALANCING_DFLT;
    int threshhold_doors = THRESHHOLD_DOORS_DFLT;
    int en;
    struct worker workers[NUM_WORKERS];

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

    // start workers
    for (int i = 0; i < NUM_WORKERS; i++) {
        if ((en = pthread_create(&workers[i].thread, NULL, worker_routine, &workers[i])) != 0) {
            handle_error_en(en, "pthread_create()");
        }
    }

    while (1) {
        sleep(1);

        if (total_doors >= threshhold_doors) break;

        printf("Producing %f knobs/s, %f doors/s\n", 
                total_knobs / (double) seconds_elapsed, total_doors / (double) seconds_elapsed);

        if (load_balancing) {
            // perform load balancing
        }

        seconds_elapsed++;
    }

    // cancel all workers
    for (int i = 0; i < NUM_WORKERS; i++) {
        if ((en = pthread_cancel(workers[i].thread)) != 0) {
            handle_error_en(en, "pthread_cancel()");
        }
    }

    // cleanup

    return EXIT_SUCCESS;
}

static void *worker_routine(void *arg) {
    struct worker *worker = arg;
    int en;

    // make the thread cancellable
    if ((en = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)) != 0)
        handle_error_en(en, "pthread_setcancelstate()");

    while (1) {
        switch(worker->work_type) {
        case WORK_DOORS:
            // produce door
            break;

        case WORK_KNOBS:
            // produce knob
            break;

        }
    }

}