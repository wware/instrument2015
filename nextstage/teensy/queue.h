#ifndef QUEUE_H_INCLUDED
#define QUEUE_H_INCLUDED 1

#include "common.h"

/** This MUST be a power of 2 */
#define BUFSIZE 1024

/**
 * A queue containing unsigned 32-bit samples. WARNING: This class
 * provides NO protection against interrupts. That must be done by
 * the user:
 *
 * ~~~
 * void example_usage(void) {
 *     uint8_t r;
 *     uint32_t x;
 *     x = next_audio_sample;
 *     cli();
 *     r = queue.write(x);
 *     sei();
 *     // handle case where r != 0
 * }
 * ~~~
 *
 * Internal implementation is a fixed-size circular buffer.
 */
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
    /**
     * Read a sample from the queue.
     * @param x a pointer to the variable to store the sample in
     * @return 0 if read is successful, 1 if queue is empty.
     */
    uint8_t read(uint32_t *x) {
        if (empty()) return 1;
        *x = buffer[rpointer];
        rpointer = (rpointer + 1) & (BUFSIZE - 1);
        return 0;
    }
    /**
     * Write a sample to the queue.
     * @param x the sample to be written
     * @return 0 if write is successful, 1 if queue is full.
     */
    uint8_t write(uint32_t x) {
        if (full()) return 1;
        buffer[wpointer] = x;
        wpointer = (wpointer + 1) & (BUFSIZE - 1);
        return 0;
    }
};

#endif    // QUEUE_H_INCLUDED
