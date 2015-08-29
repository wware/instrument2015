#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "teensy/voice.h"


FILE *outf, *gp_outf;
int t;
Synth s, s2, s3;
ISynth *synth_ary[3];


int main(void)
{
    uint8_t i;
    gp_outf = fopen("foo.gp", "w");

    outf = fopen("foo.py", "w");
    fprintf(outf, "sampfreq = %lf\n", (double) SAMPLING_RATE);
    fprintf(outf, "samples = [\n");

#define NUM_NOISY_VOICES  4
#define NUM_SIMPLE_VOICES  14
#define NUM_SQUARE_VOICES  8
    synth_ary[0] = &s;
    synth_ary[1] = &s2;
    synth_ary[2] = &s3;
    for (i = 0; i < NUM_NOISY_VOICES; i++)
        s.add(new NoisyVoice());
    for (i = 0; i < NUM_SIMPLE_VOICES; i++)
        s2.add(new SimpleVoice());
    for (i = 0; i < NUM_SQUARE_VOICES; i++)
        s3.add(new TwoSquaresVoice());
    s.quiet();
    use_synth_array(synth_ary, 3);
    use_synth(0);

    for (t = 0; t < 3 * SAMPLING_RATE; t++) {
        int32_t y;
        get_synth()->compute_sample();
        ASSERT(get_synth()->get_sample((uint32_t *) &y) == 0);

        // why the left shift? why 5 and not 4 or 6?
        fprintf(outf, "%ld,\n", (long int) y << 5);

        if (t == SAMPLING_RATE / 2) {
            s.keydown(0);
            s.keydown(7);
        }
        if (t == SAMPLING_RATE) {
            s.keydown(4);
            s.keydown(16);
        }
        if (t == SAMPLING_RATE * 3 / 2) {
            s.keydown(12);
        }
        if (t == SAMPLING_RATE * 2) {
            s.keyup(0);
            s.keyup(4);
            s.keyup(7);
        }
    }

    fprintf(outf, "]\n");
    fclose(outf);
    fclose(gp_outf);
}
