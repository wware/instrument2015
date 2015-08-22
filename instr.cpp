#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

#include "teensy/voice.h"

FILE *outf, *gp_outf;
int t;
Synth s;

int main(void)
{
    gp_outf = fopen("foo.gp", "w");

    outf = fopen("foo.py", "w");
    fprintf(outf, "sampfreq = %lf\n", (double) SAMPLING_RATE);
    fprintf(outf, "samples = [\n");

    for (int i = 0; i < 5; i++)
        s.add(new NoisyVoice());

    for (t = 0; t < 3 * SAMPLING_RATE; t++) {
        int32_t y;
        s.compute_sample();
        ASSERT(s.get_sample((uint32_t *) &y) == 0);

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
