/* -*- mode:c; c-file-style:"k&r"; c-basic-offset: 4; tab-width:4; indent-tabs-mode:nil; mode:auto-fill; fill-column:78; -*- */
/* vim: set ts=4 sw=4 et tw=78 fo=cqt wm=0: */

/* Multi-threaded LRU Simulation.
 *
 * Don Porter - porter@cs.unc.edu
 *
 * COMP 530 - University of North Carolina, Chapel Hill
 *
 */

#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <assert.h>
#include <ctype.h>
#include "lru.h"


int separate_cleaner_thread = 0;
int simulation_length = 30; // default to 30 seconds
volatile int finished = 0;

#ifdef DEBUG
#define DEBUG_PRINT(...) printf(__VA_ARGS__)
#else
#define DEBUG_PRINT(...)
#endif


static void *
delete_thread(void *arg) {
    while (!finished) {
        clean(1);
    }
    return NULL;
}

int32_t global_salt = 0;
int use_global_salt = 0;

// For the single-threaded case, we set a ratio
// of references versus cleaning passes.
#define CLEAN_RATIO 64

static void *
client(void *arg)
{
    struct random_data rd;
    char rand_state[256];
    int32_t salt = time(0);
    int counter = 0;

    if (use_global_salt)
        salt = global_salt;

    DEBUG_PRINT("Salt is %d\n", salt);

    // See http://lists.debian.org/debian-glibc/2006/01/msg00037.html
    rd.state = (int32_t*)rand_state;

    // Initialize the prng.  For testing, it may be helpful to reduce noise by
    // temporarily setting this to a fixed value at the command line.
    initstate_r(salt, rand_state, sizeof(rand_state), &rd);

    while (!finished) {
        /* Pick a random operation, string, and ip */
        int32_t key;
        int rv = random_r(&rd, &key);
        counter++;

        if (rv) {
            printf("Failed to get random number - %d\n", rv);
            return NULL;
        }

        // Mod it down from 0..MAX_KEY
        key %= MAX_KEY;
        reference(key);

        /* If we don't have a separate delete thread, the client needs to
         * periodically clear the LRU refernce count
         */
        if (!separate_cleaner_thread && ((counter % CLEAN_RATIO) == 0))
            clean(0);
    }

    return NULL;
}


#define die(msg, val) do {                      \
        print();                                \
        printf(msg, val);                       \
        exit(1);                                \
    } while (0)

void self_tests() {
    int rv;

    for (int i = 0; i < 64; i++) {
        rv = reference(i);
        if (!rv) die ("Failed to reference key %d\n", i);
    }

    // Should see 0..64 in the list
    print();

    for (int i = 0; i < 64; i+=2) {
        rv = reference(i);
        if (!rv) die ("Failed to reference key %d\n", i);
    }

    // Should see 0..64 in the list, with evens having a ref of 2
    print();

    clean(0);

    // Should see only evens
    print();
    clean(0);

    // Should see an empty list
    print();
}

void help() {
    printf ("LRU Simulator.  Usage: ./lru-[variant] [options]\n\n");
    printf ("Options:\n");
    printf ("\t-c numclients - Use numclients threads.\n");
    printf ("\t-h - Print this help.\n");
    printf ("\t-l length - Run clients for length seconds.\n");
    printf ("\n\n");
}

int main(int argc, char ** argv) {
    int numthreads = 1; // default to 1
    int c, i, rv;
    pthread_t *tinfo;

    // Read options from command line:
    //   # clients from command line, as well as seed file
    //   Simulation length
    while ((c = getopt (argc, argv, "c:hl:s:")) != -1) {
        switch (c) {
        case 'c':
            numthreads = atoi(optarg);
            if (numthreads > 1)
                separate_cleaner_thread = 1;
            break;
        case 'h':
            help();
            return 0;
        case 'l':
            simulation_length = atoi(optarg);
            break;
        case 's':
            use_global_salt = 1;
            global_salt = atoi(optarg);
            break;
        default:
            printf ("Unknown option\n");
            help();
            return 1;
        }
    }

    // Create initial data structure, populate with initial entries
    // Note: Each variant of the tree has a different init function, statically compiled in
    init(numthreads);
    srandom(time(0));

    tinfo = calloc(numthreads + separate_cleaner_thread, sizeof(pthread_t));

    // Run the self-tests if we are in debug mode
#ifdef DEBUG
    self_tests();
#endif

    // Create the delete thread
    if (separate_cleaner_thread) {
        rv = pthread_create(&tinfo[numthreads], NULL,
                            &delete_thread, NULL);
        if (rv != 0) {
            printf ("Delete thread creation failed %d\n", rv);
            return rv;
        }
    }

    // Launch client threads
    for (i = 0; i < numthreads; i++) {

        rv = pthread_create(&tinfo[i], NULL,
                            &client, NULL);
        if (rv != 0) {
            printf ("Thread creation failed %d\n", rv);
            return rv;
        }
    }

    // After the simulation is done, shut it down
    sleep (simulation_length);
    finished = 1;

    // Wait for all clients to exit.  If we are allowing blocking on a cond var,
    // notify the threads to give up, since they may hang forever otherwise
    shutdown_threads();

    for (i = 0; i < numthreads; i++) {
        int rv = pthread_join(tinfo[i], NULL);
        if (rv != 0)
            printf ("Uh oh.  pthread_join failed %d\n", rv);
    }

#ifdef DEBUG
    /* Print the final tree for fun */
    print();
#endif

    return 0;
}
