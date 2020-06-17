#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <errno.h>
#include <semaphore.h>

#define NUM_WORKERS 20
#define LOAD_BALANCING_DFLT 0
#define THRESHHOLD_DOORS_DFLT 2000

#define WORK_KNOBS 0
#define WORK_DOORS 1

#define MAX_KNOBS_IN_BUFFER 100000

/** handle pthread errors */
#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

static int total_knobs = 0;
static int total_doors = 0;
static int seconds_elapsed = 0;

static sem_t sem_to_produce;
static sem_t sem_to_consume;

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

    // initialize production semaphore
    if (sem_init(&sem_to_produce, 1, MAX_KNOBS_IN_BUFFER) < 0) {
        perror("sem_init()");
        return EXIT_FAILURE;
    }

    // initialize consumption semaphore
    if (sem_init(&sem_to_consume, 1, 0) < 0) {
        perror("sem_init()");
        return EXIT_FAILURE;
    }

    // assign work to each worker
    for (int i = 0; i < NUM_WORKERS; i++) {
        if (i % 2) {
            workers->work_type = WORK_DOORS;
        } else {
            workers->work_type = WORK_KNOBS;
        }
    }

    // start workers
    for (int i = 0; i < NUM_WORKERS; i++) {
        if ((en = pthread_create(&workers[i].thread, NULL, worker_routine, &workers[i])) != 0) {
            handle_error_en(en, "pthread_create()");
        }
    }

    while (1) {
        sleep(1);
        seconds_elapsed++;

        printf("load: %d\n", load_balancing);
        printf("doors: %d\n", threshhold_doors);
        
        printf("workers: %p\n", workers);
        printf("seconds_elapsed: %d\n", seconds_elapsed);
        printf("total_doors: %d\n", total_doors);
        printf("total_knobs: %d\n", total_knobs);

        if (total_doors >= threshhold_doors) break;

        printf("Producing %.2f knobs/s, %.2f doors/s\n", 
                total_knobs / (double) seconds_elapsed, total_doors / (double) seconds_elapsed);

        if (load_balancing) {
            // perform load balancing
        }

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
    unsigned seed = (unsigned) pthread_self();
    int en;

    //compute lazyness
    worker->lazyness = rand_r(&seed) / (double) RAND_MAX;

    printf("%f\n", worker->lazyness);

    // make the thread cancellable
    if ((en = pthread_setcancelstate(PTHREAD_CANCEL_ENABLE, NULL)) != 0)
        handle_error_en(en, "pthread_setcancelstate()");

    while (1) {
        switch(worker->work_type) {
        case WORK_DOORS:
            // simulate production
            usleep((10 + worker->lazyness * 90) * 1000);

            sem_wait(&sem_to_consume);
            total_doors++;
            break;

        case WORK_KNOBS:
            // simulate production
            usleep((10 + worker->lazyness * 30) * 1000);

            total_knobs++;
            sem_post(&sem_to_consume);

            break;
        }
    }

}