/*
We have a 40 kHz or 50 kHz timer interrupt that handles sound
samples. First it writes the previously computed sample to the serial DAC.
Then it computes the sample for the next time.
*/

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#define __ARDUINO 0

#define HIGH 1
#define LOW 0

#define ASSERT(cond)    assertion(cond, #cond)

#include "teensy/teensy.ino"

#define _STEP  1.0594630943592953
#define _STEP2  (_STEP * _STEP)
#define _STEP4  (_STEP2 * _STEP2)

#define _A  (440.0 * 2 * VCO_MAX_VALUE / (4 * SAMPLING_RATE))

#define F(oct)       ((int) ((1 << oct) * _A / _STEP4))
#define Fsharp(oct)  ((int) ((1 << oct) * _A / _STEP2 / _STEP))
#define G(oct)       ((int) ((1 << oct) * _A / _STEP2))
#define Gsharp(oct)  ((int) ((1 << oct) * _A / _STEP))
#define A(oct)       ((int) ((1 << oct) * _A))
#define Bflat(oct)   ((int) ((1 << oct) * _A * _STEP))
#define B(oct)       ((int) ((1 << oct) * _A * _STEP2))
#define C(oct)       ((int) ((1 << oct) * _A * _STEP2 * _STEP))

void assertion(int cond, const char *strcond)
{
    if (!cond) {
        fprintf(stderr, "ASSERTION FAILED: %s\n", strcond);
        exit(1);
    }
}

int main(void)
{
    FILE *outf, *gp_outf;
    int i, t;
    int64_t x, max = (1L << 31);
    int32_t y;

    setup();

    gp_outf = fopen("foo.gp", "w");

    outf = fopen("foo.py", "w");
    fprintf(outf, "sampfreq = %lf\n", (double) SAMPLING_RATE);
    fprintf(outf, "samples = [\n");

    for (i = 0; i < NUM_VOICES; i++)
        v[i].setfreq(200 + 0.2 * i);

    for (t = 0; t < 4 * SAMPLING_RATE; t++) {
        x = 0;
        for (i = 0; i < NUM_VOICES; i++)
            x += v[i].output();
        ASSERT(x > -max);
        ASSERT(x < max);
        compute_sample();
        ASSERT(samples.read((uint32_t *) &y) == 0);

        /* Numbers for Gnuplot */
        fprintf(gp_outf, "%d %d %d\n", t, v[0].adsr_level() >> 22, (int) (y - 2048));

        /* Numbers for AIFF file */
        fprintf(outf, "%ld,\n", (long int) (y << 3));   // DO NOT GO TO 4

        if (t == SAMPLING_RATE / 2) {
            for (i = 0; i < NUM_VOICES; i++)
                v[i].keydown(1);
        }
        if (t == SAMPLING_RATE) {
            for (i = 0; i < NUM_VOICES; i++)
                v[i].keydown(0);
        }
    }

    fprintf(outf, "]\n");
    fclose(outf);
    fclose(gp_outf);
}
