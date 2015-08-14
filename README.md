<!--
pip install markdown
python -m markdown README.md > README.html
-->

The Tooba is an electronic musical instrument built into a piece of PVC tubing (hence the name). It borrows principles from the Moog-style analog music synthesizers of past decades.

[![Video demo of the instrument](http://img.youtube.com/vi/QGhZ0tecp60/0.jpg)](https://www.youtube.com/watch?v=QGhZ0tecp60)

Way back when, one of my friends in high school was a guy named [Dave Wilson](http://www.matrixsynth.com/2010/08/rip-david-hillel-wilson-curator-of-new.html) whose father got him into 1970s-era Moog-style analog electronic music synthesizers. Dave created a museum of historical synthesizers in his home in Nashua, New Hampshire. Throughout our high school and college years, we exchanged ideas and circuits and bits of lore for various synthesizer hacks.

Music synthesizers of that era were actually special-purpose [analog computers](https://en.wikipedia.org/wiki/Analog_computer), performing arithmetic operations with [integrators, summers, and other such circuits](https://courses.engr.illinois.edu/ece486/labs/lab1/analog_computer_manual.pdf). These computations can be performed digitally by a microprocessor or special-purpose digital circuit (e.g. FPGA). So Dave and I both at various points and in various contexts wrote code to do that.

Sound generation in the Tooba (and keyboard scanning and voice assignment) is done in [C++](https://github.com/wware/instrument2015/blob/master/nextstage/teensy/teensy.ino) running on a 32-bit ARM microcontroller.

Status, plans, etc
----

The first PVC prototype, the one shown at Providence on Aug 8th 2015, did all sound generation in the interrupt handler. More recently, the interrupt handler only transfers audio samples from a queue to the microcontroller's on-chip 12-bit DAC, and sound generation occurs in the loop() function.

* Doxygen stuff is now [online](http://wware.github.io/instrument2015/).
* ADSR and other slow-moving things should perform real calculations at only around 50 Hz, not 40 or 50 kHz. In between calculations they should use linear interpolation. Four billion divided by 50K is 80K, so it’s fine to use 16 fraction bits to interpolate. This will significantly speed things up.
* Never copy or move blocks of data. Move pointers instead. The persistence data for a voice should live in one place and never move.
* The next step right now is to take the code in the Voice class and split it into VCO, VCA and ADSR classes. Then add a Noise class, a Summer class, and a VCF class. The audio outputs of all these should be 24-bit signed integers in the range from -0x80_0000 to 0x7F_FFFF. Use asserts to make sure they never go outside that range.
* Set up a [monkey test framework](https://en.wikipedia.org/wiki/Monkey_test) that virtually pounds on the keyboard, and detects overflows, underruns, etc.
  * A [SWIG wrapper](http://swig.org/) will be very helpful with this.

Teensy 3.1 info
----

* [The Freescale Semiconductor web page](http://www.freescale.com/webapp/sps/site/prod_summary.jsp?code=K20_50) for the MK20DX256VLH7 processor used in the Teensy 3.1.
* [The datasheet](https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf) that talks about GPIO programming starting on page 1331.
* [The Teensy 3.1 schematic](https://www.pjrc.com/teensy/schematic.html). PTB17 is the GPIO that I want to read. The Port B input register is at 0x400FF050. There is no MMU so no need for any mmap craziness.
* [Here](https://forum.pjrc.com/threads/25317-Assembly-coding-for-Teensy3-1) is a great discussion of embedding assembly in C code. Also see http://www.ethernut.de/en/documents/arm-inline-asm.html.
* [A C header file](http://www.keil.com/dd/docs/arm/freescale/kinetis/mk20d7.h) for registers in the MK20DX.
* [Some nice info](http://www.peter-cockerell.net/aalp/html/frames.html) on ARM assembly language.

Earlier prototype
----

I want to read the GPIO in assembly to accurately measure the period of the oscillator. This code ran on the June prototype and measured the capacitance of the touch sensitive keyboard. The capacitance was used in the RC time constant of a free-running oscillator and the idea here is to get a measurement roughly proportional to the period.

1. Set counter to zero.
2. If input is LOW, go to step 7.
3. Wait for a falling edge. Proceed when input is LOW.
4. Increment counter while waiting for rising edge. Proceed when input is HIGH.
5. Increment counter while waiting for falling edge. Proceed when input is LOW.
6. Go to step 10.
7. Wait for a rising edge. Proceed when input is HIGH.
8. Increment counter while waiting for falling edge. Proceed when input is LOW.
9. Increment counter while waiting for rising edge. Proceed when input is HIGH.
10. Return counter.

So that piece is assembly, and then in C you choose a touch contact and call that function. That should ensure that you read the keyboard about as fast as it can be read. The code for this is buried in [the sandbox directory](https://github.com/wware/instrument2015/blob/master/sandbox/prototype1/prototype1.ino#L7).

~~~
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
~~~

This works well in both the [Arduino/Teensy IDE](https://www.pjrc.com/teensy/teensyduino.html) and in the [online ARM C++ compiler](http://assembly.ynh.io/).
