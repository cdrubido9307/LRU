/* -*- mode:c; c-file-style:"k&r"; c-basic-offset: 4; tab-width:4; indent-tabs-mode:nil; mode:auto-fill; fill-column:78; -*- */
/* vim: set ts=4 sw=4 et tw=78 fo=cqt wm=0: */

#ifndef __LRU_H__
#define __LRU_H__

/* A simple LRU tracking interface */

// Maximum key value
#define MAX_KEY 128
#define LOW_WATER_MARK 32
#define HIGH_WATER_MARK 96
// Uncomment this line for debug printing
//#define DEBUG 1

/* Optional init routine.  May not be required.
 * Returns 0 on success, -errno on failure.
 */
int init (int numthreads);

/* Return 1 on success, 0 on failure.
 * Should set the reference count up by one if found; add if not.*/
int reference (int key);

/* Do a pass through all elements, either decrement the reference count,
 * or remove if it hasn't been referenced since last cleaning pass.
 *
 * check_water_mark: If 1, block until there are more elements in the cache
 * than the LOW_WATER_MARK.  This should only be 0 during self-testing or in
 * single-threaded mode.
 */
void clean(int check_water_mark);

/* Optional shut-down routine to wake up blocked threads.
   May not be required. */
void shutdown_threads (void);

/* Print the contents of the list.  Mostly useful for debugging. */
void print (void);


#endif /* __LRU_H__ */
