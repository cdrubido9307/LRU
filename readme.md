 In this lab we develop a multi-threaded simulation of a Least Recently Used (LRU) cache of keys.

Our job is to create several parallel verions of the code, with increasing sophistication.

LRU Model Overview

In this assignment, we will assume that our cache needs to track the reference count of a set of keys (here, integers from 0..MAX_KEY). When a key is referenced, by calling reference(key), the key should be found in the cache, and either added and the reference count set to 1, or the reference count incremented by one (if already present). This cache is organized as a simple, singly-linked list.

Periodically, a thread will need to clean the cache (similar to the clock algorithm we saw in class). The cleaner thread iterates through the list and decrements the reference count by 1. For any node that reaches a reference count of zero, the node should be removed from the list and freed. 

Mutex-LRU

In mutex-lru.c we implemented a thread-safe list, using coarse-grained locking. In other words, it is sufficient to have one lock for the entire list. 
To test the code, run lru-mutex -c XXX (where XXX is an integer greater than 1).

Ideally, we want to maintain that the cache has a certain number of elements --- i.e., that the count variable stays between LOW_WATER_MARK and HIGH_WATER_MARK. Therefore, we use condition variables to ensure two things:

    That a call to reference() blocks until count is below HIGH_WATER_MARK, as reference may add an element to the list.
    That a call to clean() blocks until count is above LOW_WATER_MARK, ensuring the list doesn't get too small.
    That any blocked threads will exit from their current routine when shutdown_threads() is called.

In particular, we would like you to use condition variables, as well as the existing mutex synchronization, to allow the delete thread to sleep until there is work to do, and then to wake up once there is.

For this we add condition variable support to the mutex lru list, using a condition variable to block reference and clean, as described above.

Fine-LRU

We can improve concurrency by supporting a single lock per node, allowing threads to execute concurrently in different parts of the trie.

For this, we implemented a list that uses fine-grained locking in fine-lru.c. In other words, every node in the list has its own lock.
What makes fine-grained locking tricky is ensuring that you cannot deadlock while acquiring locks, and that a thread doesn't hold unnecessary locks.

Protocol

We try to follow hand-over-hand locking protocol. For this, we hold the lock of two nodes at a time to provide safety when multiple threads traverse the list. We do not hold more locks than the ones needed to avoid deadlocks. Also, the global mutex is acquired at the beginning of the functions to check the conditional variables and it is released right afterwards. To avoid deadlocks and a complicated implementation, we have a window where count is out of sync with the list. For example, in the reference method, the global lock is dropped and then re-acquired to increment count and possibly signal other threads. We only re-acquire the global lock at the end of each function and once the previous locks are released. The list head lock is kept only while we are inserting elements to the beginning of the list or deleting the first nodes on the list. Otherwise, we release the list head lock, but always maintaining two nodes locked at a given time (if possible).

Tests
We provide three test cases, one for referencing, another for cleaning, and one that combines the two. You must uncomment them in order for them to be run. The ones inside DEBUG are for single thread tests, and the ones inside main are for multiple ones.

