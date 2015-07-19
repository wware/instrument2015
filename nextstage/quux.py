import aifc
import math
import sys

filename = sys.argv[1]

F_SAMPLE = 40000
DT = 1. / F_SAMPLE

class Aiff:
    def __init__(self):
        self.f = aifc.open(filename, "w")
        self.f.setnchannels(1)
        self.f.setsampwidth(2)   # Sound samples are 16-bit signed integers
        self.f.setframerate(F_SAMPLE)
        self.z = ""

    def silent_sample(self):
        self.sample(0)

    def sample(self, x):
        # x ranges from -1.0 to 1.0
        x = int(32767 * x)
        xh = (x >> 8) & 0xff
        xl = x & 0xff
        self.z += chr(xh) + chr(xl)

    def close(self):
        self.f.writeframes(self.z)
        self.f.close()


class Oscillator:
    def __init__(self):
        self.phase = 0
        self.dphase = 0
        self.normalized_phase = 0.

    def setfreq(self, f):
        self.dphase = int((2.0 ** 32) * f / F_SAMPLE)

    def step(self):
        self.phase = (self.phase + self.dphase) % (1 << 32)
        # normalized phase ranges from 0.0 to 1.0
        self.normalized_phase = (2 ** -32) * self.phase

    """
    These outputs produce real numbers in the range
    -1 to 1.
    """
    def sine(self):
        return math.sin(2 * math.pi * self.normalized_phase)

    def ramp(self):
        return -1 + 2 * self.normalized_phase

    def triangle(self):
        if self.normalized_phase > 0.5:
            return 3 - 4 * self.normalized_phase
        else:
            return -1 + 4 * self.normalized_phase


class ADSR:
    def __init__(self):
        self._value = 0      # 32-bit unsigned integer
        self.state = 0
        self.threshold = 0x80000000
        self.A = self.D = self.R = 1
        self.S = 0x40000000

    def value(self):
        return  0.999 * self._value / self.threshold

    def setattack(self, a):
        a = max(a, 0.01)
        self.A = int(self.threshold * DT / a)

    def setdecay(self, d):
        d = max(d, 0.01)
        self.D = int(self.threshold * DT / d)

    def setsustain(self, s):
        self.S = int(self.threshold * s)

    def setrelease(self, r):
        r = max(r, 0.01)
        self.R = int(self.threshold * math.exp(-DT / r))

    def step(self):
        if self.state == 1:
            # attack
            self._value += self.A
            if self._value >= self.threshold:
                self.state = 2
        elif self.state == 2:
            # decay
            self._value -= self.D
            if self._value <= self.S:
                # sustain
                self._value = self.S
                self.state = 3
        elif self.state == 0:
            # release
            self._value = (self._value * self.R) >> 31

    def keydown(self):
        self.state = 1

    def keyup(self):
        self.state = 0


adsr = ADSR()
adsr.setattack(0)
adsr.setdecay(0.1)
adsr.setsustain(0.5)
adsr.setrelease(0.4)

osc = Oscillator()
osc.setfreq(440)

aiff = Aiff()

for i in range(1 * F_SAMPLE):
    osc.step()
    aiff.sample(osc.sine())

for i in range(int(0.5 * F_SAMPLE)):
    aiff.silent_sample()

for i in range(1 * F_SAMPLE):
    osc.step()
    aiff.sample(osc.triangle())

for i in range(int(0.5 * F_SAMPLE)):
    aiff.silent_sample()

for i in range(1 * F_SAMPLE):
    osc.step()
    aiff.sample(osc.ramp())

for i in range(int(0.5 * F_SAMPLE)):
    aiff.silent_sample()

adsr.keydown()
for i in range(2 * F_SAMPLE):
    if i > 0.8 * F_SAMPLE:
        adsr.keyup()
    osc.step()
    adsr.step()
    aiff.sample(osc.ramp() * adsr.value())

aiff.close()