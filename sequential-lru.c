/* -*- mode:c; c-file-style:"k&r"; c-basic-offset: 4; tab-width:4; indent-tabs-mode:nil; mode:auto-fill; fill-column:78; -*- */
/* vim: set ts=4 sw=4 et tw=78 fo=cqt wm=0: */

/* @* Place your name here, and any other comments *@
 * @* that deanonymize your work inside this syntax *@
 */

#include <stddef.h>
#include <stdlib.h>
#include <stdio.h>

/* Define the simple, singly-linked list we are going to use for tracking lru */
struct list_node {
    struct list_node* next;
    int key;
    int refcount;
};

static struct list_node* list_head = NULL;
static int count = 0;

/* No init work needed for the sequential version. */
int init (int numthreads) {
    // This code is only safe with one thread
    return 0;
}

/* Return 1 on success, 0 on failure.
 * Should set the reference count up by one if found; add if not.*/
int reference (int key) {
    int found = 0;
    struct list_node* cursor = list_head;
    struct list_node* last = NULL;

    while(cursor) {
        if (cursor->key < key) {
            last = cursor;
            cursor = cursor->next;
        } else {
            if (cursor->key == key) {
                cursor->refcount++;
                found++;
            }
            break;
        }
    }

    if (!found) {
        // Handle 2 cases: the list is empty/we are trying to put this at the
        // front, and we want to insert somewhere in the middle or end of the
        // list.
        struct list_node* new_node = malloc(sizeof(struct list_node));
        if (!new_node) return 0;
        count++;
        new_node->key = key;
        new_node->refcount = 1;
        new_node->next = cursor;

        if (last == NULL)
            list_head = new_node;
        else
            last->next = new_node;
    }

    return 1;
}

/* Do a pass through all elements, either decrement the reference count,
 * or remove if it hasn't been referenced since last cleaning pass.
 *
 * check_water_mark: If 1, block until there are more elements in the cache
 * than the LOW_WATER_MARK.  This should only be 0 during self-testing or in
 * single-threaded mode.
 */
void clean(int check_water_mark) {
    struct list_node* cursor = list_head;
    struct list_node* last = NULL;

    while(cursor) {
        cursor->refcount--;
        if (cursor->refcount == 0) {
            struct list_node* tmp = cursor;
            if (last) {
                 last->next = cursor->next;
            } else {
                 list_head = cursor->next;
            }
            tmp = cursor->next;
            free(cursor);
            cursor = tmp;
            count--;
        } else {
            last = cursor;
            cursor = cursor->next;
        }
    }
}

/* Optional shut-down routine to wake up blocked threads.
   May not be required. */
void shutdown_threads () { return; }

/* Print the contents of the list.  Mostly useful for debugging. */
void print () {
    printf("=== Starting list print ===\n");
    printf("=== Total count is %d ===\n", count);
    struct list_node* cursor = list_head;
    while(cursor) {
        printf ("Key %d, Ref Count %d\n", cursor->key, cursor->refcount);
        cursor = cursor->next;
    }
    printf("=== Ending list print ===\n");
}
