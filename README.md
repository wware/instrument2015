<!--
pip install markdown
python -m markdown README.md > README.html
-->

Programming the Teensy
====

I want to read the GPIO in assembly to accurately measure the period of the oscillator.

* [The Freescale Semiconductor web page](http://www.freescale.com/webapp/sps/site/prod_summary.jsp?code=K20_50) for the MK20DX256VLH7 processor used in the Teensy 3.1.
* [The datasheet](https://www.pjrc.com/teensy/K20P64M72SF1RM.pdf) that talks about GPIO programming starting on page 1331.
* [The Teensy 3.1 schematic](https://www.pjrc.com/teensy/schematic.html). PTB17 is the GPIO that I want to read. The Port B input register is at 0x400FF050. There is no MMU so no need for any mmap craziness.
* [Here](https://forum.pjrc.com/threads/25317-Assembly-coding-for-Teensy3-1) is a great discussion of embedding assembly in C code. Also see http://www.ethernut.de/en/documents/arm-inline-asm.html.
* [A C header file](http://www.keil.com/dd/docs/arm/freescale/kinetis/mk20d7.h) for registers in the MK20DX.
* [Some nice info](http://www.peter-cockerell.net/aalp/html/frames.html) on ARM assembly language.

I think all I need is something like this.

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

So that piece is assembly, and then in C you choose a touch contact and call that function. That should ensure that you read the keyboard about as fast as it can be read.