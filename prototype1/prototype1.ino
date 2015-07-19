const int piano = 1;
const int organ = 20;
const int glockenspiel = 10;
const int trumpet = 57;
const int channel = 1;

static int measurePeriod(void)
{
  // Step 1, set up registers
  int count = 0, x = 0, mask = 1 << 17, addr = 0x400ff050;
# define READ_INPUT  "ldr %1, [%3]\n"     "ands %1, %1, %2\n"
# define INC_COUNT   "add %0, %0, #1\n"
  asm volatile(
    // Step 2, if input is low go to step 7
    READ_INPUT
    "beq step7"                             "\n"
    // Step 3, wait for falling edge
    "step3:"                                "\n"
    READ_INPUT
    "bne step3"                             "\n"
    // Step 4, wait for rising edge while incrementing counter
    "step4:"                                "\n"
    INC_COUNT
    READ_INPUT
    "beq step4"                             "\n"
    // Step 5, wait for falling edge while incrementing counter
    "step5:"                                "\n"
    INC_COUNT
    READ_INPUT
    "bne step5"                             "\n"
    // Step 6, go to step 10
    "b step10"                              "\n"
    // Step 7, wait for rising edge
    "step7:"                                "\n"
    READ_INPUT
    "beq step7"                             "\n"
    // Step 8, wait for falling edge while incrementing counter
    "step8:"                                "\n"
    INC_COUNT
    READ_INPUT
    "bne step8"                             "\n"
    // Step 9, wait for rising edge while incrementing counter
    "step9:"                                "\n"
    INC_COUNT
    READ_INPUT
    "beq step9"                             "\n"
    // Step 10, done
    "step10:"                               "\n"
    : "+r" (count), "+r" (x), "+r" (mask), "+r" (addr)
  );

  return count;
}

struct _Contact {
  int pitch_offset, state, total_measure, working_pitch;
} contacts[] = {
  { 14, 0, 0, 0 },
  { 12, 0, 0, 0 },
  { 11, 0, 0, 0 },
  { 9, 0, 0, 0 },
  { 7, 0, 0, 0 },
  { 6, 0, 0, 0 },
  { 5, 0, 0, 0 },
  { 4, 0, 0, 0 },
  { 3, 0, 0, 0 },
  { 2, 0, 0, 0 },
  { 1, 0, 0, 0 },
  { 0, 0, 0, 0 },
  { 0, 0, 0, 0 },  // Left pinky
  { 0, 0, 0, 0 },  // Left ring
  { 0, 0, 0, 0 },  // Left middle
  { 0, 0, 0, 0 },  // Left index, closest to right hand
};
const int num_contacts = (sizeof(contacts) / sizeof(contacts[0]));

#define NUM_PITCHES 64
int pitch_states[NUM_PITCHES];

void setup() {
  int i;
  for (i = 0; i < NUM_PITCHES - 1; i++)
    pitch_states[i] = 0;
  //start serial connection
  Serial.begin(9600);
  //configure pin2 as an input and enable the internal pull-up resistor
  pinMode(1, INPUT_PULLUP);
  pinMode(0, OUTPUT);
  pinMode(2, OUTPUT);
  pinMode(3, OUTPUT);
  pinMode(4, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  digitalWrite(2, HIGH);
  digitalWrite(3, LOW);
  digitalWrite(4, HIGH);
  digitalWrite(5, HIGH);
  digitalWrite(6, HIGH);
  digitalWrite(0, HIGH);
}

void loop() {
  int i, n;
  int pitch_base = 60;  // middle C
  struct _Contact *c;
  delayMicroseconds(3000);
  for (i = 0; i < num_contacts - 4; i++) {
    c = &contacts[i];
    digitalWrite(6, (i & 4) ? HIGH : LOW);
    digitalWrite(5, (i & 2) ? HIGH : LOW);
    digitalWrite(4, (i & 1) ? HIGH : LOW);
    digitalWrite(3, (!(i & 8)) ? HIGH : LOW);
    digitalWrite(2, (i & 8) ? HIGH : LOW);
    n = measurePeriod() - 18;
    if (n <= 0) {
      // if note was on before, NOTE_OFF now
      if (c->state != 0) {
        pitch_states[c->working_pitch] = 0;
        c->state = 0;
        c->total_measure = 0;
        usbMIDI.sendNoteOff(c->working_pitch, 0, channel);
      }
    }
    else {
      /*
       * TODO - Don't start a note if there is another note of the same
       * working pitch already in progress. Use the pitch_states array
       * to keep track of which pitches are active at any given time.
       */
      if (c->state < 5) {
        c->total_measure += n;
        c->state++;
      } else {
        c->working_pitch = c->pitch_offset + pitch_base;
        n = 5 * c->total_measure;
        c->total_measure = 0;
        n = (n > 127) ? 127 : n;
        usbMIDI.sendProgramChange(glockenspiel, channel);
        usbMIDI.sendNoteOn(c->working_pitch, n, channel);
      }
    }
    n = (n < 0) ? 0 : n;
  }
}

