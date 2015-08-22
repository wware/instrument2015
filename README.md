<!--
pip install markdown
python -m markdown README.md > README.html
-->

Modular Music Synthesis in C++
====

Historical background
----

Way back when, one of my friends in high school was a guy named [Dave Wilson](http://www.matrixsynth.com/2010/08/rip-david-hillel-wilson-curator-of-new.html) whose father got him into 1970s-era Moog-style analog electronic music synthesizers. Dave created a museum of historical synthesizers in his home in Nashua, New Hampshire. Throughout our high school and college years, we exchanged ideas and circuits and bits of lore for various synthesizer hacks.

Music synthesizers of that era were composed of [modules](https://en.wikipedia.org/wiki/Modular_synthesizer) and were actually special-purpose [analog computers](https://en.wikipedia.org/wiki/Analog_computer), performing arithmetic operations with [integrators, summers, and other such circuits](https://courses.engr.illinois.edu/ece486/labs/lab1/analog_computer_manual.pdf). These computations can be performed digitally by a microprocessor or special-purpose digital circuit (e.g. FPGA). So Dave and I both at various points and in various contexts wrote code to do that.

Sound generation in this code is done in C++, and can run on the [Teensy 3.1 board](https://www.pjrc.com/teensy/teensy31.html) which has a 32-bit ARM microcontroller capable of running at 96 MHz.

Embodiments
----

This code has two readily available embodiments. One is the test.py script which will generate an audio file called "quux.aiff". It will attempt to play the file.

The other is an easy-to-build piece of electronics using a Teensy board, some batteries, some pushbuttons, a few components, and a pair of earbuds. I call this thing a "trivisynth" because it is the most trivial piece of hardware that one could justifiably call a synthesizer.

Performance considerations
----

Fixed-point arithmetic because the ARM has no floating-point hardware. Interrupt handler should be limited in scope. Some things could be coded in assembly language, and that may be done in the future. If you get buffer underruns (the LED turns on) then you can either decrease the SAMPLING_RATE or the number of voices.

test.py
====

TL;DR

trivisynth
====

You'll need [Teensyduino](https://www.pjrc.com/teensy/teensyduino.html) set up in order to load the code onto your Teensy board.

Provide instructions to build the damn thing and a little info about how to play it.

Teensy 3.1 info
----

* [The Freescale Semiconductor web page](http://www.freescale.com/webapp/sps/site/prod_summary.jsp?code=K20_50) for the MK20DX256VLH7 processor used in the Teensy 3.1.
* [The datasheet](https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf) that talks about GPIO programming starting on page 1331.
* [The Teensy 3.1 schematic](https://www.pjrc.com/teensy/schematic.html). PTB17 is the GPIO that I want to read. The Port B input register is at 0x400FF050. There is no MMU so no need for any mmap craziness.
* [Here](https://forum.pjrc.com/threads/25317-Assembly-coding-for-Teensy3-1) is a great discussion of embedding assembly in C code. Also see http://www.ethernut.de/en/documents/arm-inline-asm.html.
* [A C header file](http://www.keil.com/dd/docs/arm/freescale/kinetis/mk20d7.h) for registers in the MK20DX.
* [Some nice info](http://www.peter-cockerell.net/aalp/html/frames.html) on ARM assembly language.
* There is an [online ARM C++ compiler](http://assembly.ynh.io/).

Here is what assembly language looks like.

```c++
static int measurePeriod(void)
{
    // Step 1, set up registers
    int count = 0, x = 0, mask = 1 << 17, addr = 0x400ff050;
#   define READ_INPUT  "ldr %1, [%3]\n"     "ands %1, %1, %2\n"
#   define INC_COUNT   "add %0, %0, #1\n"
    asm volatile(
        // Step 2, if input is low go to step 7
        READ_INPUT
        "beq step7"                             "\n"
        // Step 3, wait for falling edge
        "step3:"                                "\n"

        ... lots more code ...

        "step10:"                               "\n"
        : "+r" (count), "+r" (x), "+r" (mask), "+r" (addr)
    );
    return count;
}
```
