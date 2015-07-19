/*
We have a 40 kHz or 50 kHz timer interrupt that handles sound
samples. First it writes the previously computed sample to the serial DAC.
Then it computes the sample for the next time.
*/

#define __SERIAL 0

#include <stdio.h>
#include <stdint.h>
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

int main(void)
{
    int t;

    setup();

    printf("sampfreq = %lf\n", (double) SAMPLING_RATE);
    printf("samples = [\n");

    v.setfreq(440);
    v.keydown(1);
    for (t = 0; t < 3 * SAMPLING_RATE; t++) {
        loop();
        printf("%d,\n", v.output() >> 16);
        if (t == SAMPLING_RATE)
            v.keydown(0);
    }

    printf("]\n");
}
