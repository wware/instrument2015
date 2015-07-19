/*
Adapted from https://www.pjrc.com/teensy/td_libs_TimerOne.html
My purpose here is to determine that my command of ARM assembly is good
enough that I can use it in a timer interrupt handler without breaking
the world.
*/

#include <TimerOne.h>

// This example uses the timer interrupt to blink an LED
// and also demonstrates how to share a variable between
// the interrupt and the main program.

const int led = LED_BUILTIN;  // the pin with a LED

void setup(void)
{
  pinMode(led, OUTPUT);
  Timer1.initialize(150000);
  Timer1.attachInterrupt(blinkLED); // blinkLED to run every 0.15 seconds
  Serial.begin(9600);
}


// The interrupt will blink the LED, and keep
// track of how many times it has blinked.
int ledState = LOW;
volatile unsigned long blinkCount = 0; // use volatile for shared variables

void blinkLED(void)
{
  /*
  Both variables are 32 bits, the same size as a register.
  LOW = 0, HIGH = 1.
  */
  asm volatile(
    "cmp %0, #0"                    "\n"
    "bne step1"                     "\n"
    "add %1, %1, #1"                "\n"
    "ldr %0, =1"                    "\n"
    "b step2"                       "\n"
    "step1:"                        "\n"
    "ldr %0, =0"                    "\n"
    "step2:"                        "\n"
    : "+r" (ledState), "+r" (blinkCount)
  );
  digitalWrite(led, ledState);
}


// The main program will print the blink count
// to the Arduino Serial Monitor
void loop(void)
{
  unsigned long blinkCopy;  // holds a copy of the blinkCount

  // to read a variable which the interrupt code writes, we
  // must temporarily disable interrupts, to be sure it will
  // not change while we are reading.  To minimize the time
  // with interrupts off, just quickly make a copy, and then
  // use the copy while allowing the interrupt to keep working.
  noInterrupts();
  blinkCopy = blinkCount;
  interrupts();

  Serial.print("blinkCount = ");
  Serial.println(blinkCopy);
  delay(100);
}
