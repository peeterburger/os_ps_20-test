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

/** handle pthread errors */
#define handle_error_en(en, msg) \
        do { errno = en; perror(msg); exit(EXIT_FAILURE); } while (0)

static int total_knobs = 0;
static int total_doors = 0;
static int seconds_elapsed = 0;

static sem_t sem_to_consume;

static pthread_mutex_t mutexes[2] = { PTHREAD_MUTEX_INITIALIZER, PTHREAD_MUTEX_INITIALIZER };

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
    float knob_rate;
    float door_rate;
    int total_door_workers = 0;
    int total_knob_workers = 0;

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

    // initialize consumption semaphore
    if (sem_init(&sem_to_consume, 1, 0) < 0) {
        perror("sem_init()");
        return EXIT_FAILURE;
    }

    // assign work to each worker
    for (int i = 0; i < NUM_WORKERS; i++) {
        if (i % 2) {
            workers->work_type = WORK_DOORS;
            total_door_workers++;
        } else {
            workers->work_type = WORK_KNOBS;
            total_knob_workers++;
        }
    }

    // start workers
    for (int i = 0; i < NUM_WORKERS; i++) {
        if ((en = pthread_create(&workers[i].thread, NULL, worker_routine, &workers[i])) != 0) {
            handle_error_en(en, "pthread_create()");
        }
    }

    // perform factory management
    while (1) {
        sleep(1);

        // requested doors have been produced
        if (total_doors >= threshhold_doors)
            break;

        seconds_elapsed++;
        knob_rate = total_knobs / (double) seconds_elapsed;
        door_rate = total_doors / (double) seconds_elapsed;

        printf("Producing %.2f knobs/s, %.2f doors/s\n", 
                knob_rate, door_rate);

        // no need to perform worker update
        if (!load_balancing || door_rate / knob_rate >= 0.9)
            continue;

        for (int i = 0; i < NUM_WORKERS; i++) {
            if (workers[i].work_type != WORK_KNOBS)
                continue;

            // to the current worker, assign door work
            workers[i].work_type = WORK_DOORS;
            total_door_workers++;
            total_knob_workers--;

            printf("\tWorkers reassigned: %d making knobs, %d making doors\n",
                    total_knob_workers, total_door_workers);

            break;
        }
    }

    // print final stats
    printf("%d doors produced in %.2fs (%.2f doors/s)\n",
            total_doors, (float) seconds_elapsed, door_rate);
    printf("%d knobs produced\n", total_knobs);

    // cleanup semaphores
    if (sem_destroy(&sem_to_consume) < 0) {
        perror("sem_destroy()");
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}

static void *worker_routine(void *arg) {
    struct worker *worker = arg;
    unsigned seed = (unsigned) pthread_self();

    //compute lazyness
    worker->lazyness = rand_r(&seed) / (double) RAND_MAX;

    while (1) {
        switch(worker->work_type) {
        case WORK_DOORS:
            // simulate production
            usleep((10 + worker->lazyness * 90) * 1000);

            sem_wait(&sem_to_consume);
            
            pthread_mutex_lock(&mutexes[0]);
            total_doors++;
            pthread_mutex_unlock(&mutexes[0]);

            break;

        case WORK_KNOBS:
            // simulate production
            usleep((10 + worker->lazyness * 30) * 1000);

            pthread_mutex_lock(&mutexes[1]);
            total_knobs++;
            pthread_mutex_unlock(&mutexes[1]);

            sem_post(&sem_to_consume);

            break;
        }
    }

    return NULL;
}