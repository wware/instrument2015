#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED 1

#include "common.h"

// make sure this is a power of 2
#define BUFSIZE 1024

class Queue
{
    int wpointer, rpointer;
    uint32_t buffer[BUFSIZE];
    inline int size(void) {
        return (wpointer + BUFSIZE - rpointer) & (BUFSIZE - 1);
    }
    inline int empty(void) {
        return size() == 0;
    }
    inline int full(void) {
        return size() == BUFSIZE - 1;
    }
public:
    Queue() {
        wpointer = rpointer = 0;
    }
    /* If the queue is empty, this method returns 1. Otherwise it reads a sample
     * and stores it in x and returns zero.
     */
    uint8_t read(uint32_t *x) {
        if (empty()) return 1;
        *x = buffer[rpointer];
        rpointer = (rpointer + 1) & (BUFSIZE - 1);
        return 0;
    }
    /* If the queue is full, this method returns 1. Otherwise it writes a sample
     * and returns zero.
     */
    uint8_t write(uint32_t x) {
        if (full()) return 1;
        buffer[wpointer] = x;
        wpointer = (wpointer + 1) & (BUFSIZE - 1);
        return 0;
    }
};

#endif
