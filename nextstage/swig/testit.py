import instrument

assert instrument.Key is not None
assert instrument.Queue is not None
assert instrument.Voice is not None
assert instrument.Oscillator is not None
assert instrument.ADSR is not None
assert instrument.mult_signed is not None

q = instrument.Queue()
assert q.write(4) == 0

# this should succeed (0) with result 4
x = q.read()
assert x == [0, 4]

# this should fail (1)
x = q.read()
assert x[0] == 1

# fill up the queue, remember it holds only 1023 samples
for i in range(1023):
    assert q.write(i) == 0, i

# queue is now full, expect failure
assert q.write(1023) == 1

# verify queue contents
for i in range(1023):
    r, j = q.read()
    assert r == 0, r
    assert j == i, (j, i)

osc = instrument.Oscillator()
osc.setfreq(2000.0)
assert osc.get_phase() == 0
osc.setwaveform(0)
assert osc.output() == 0x0, hex(osc.output())  # ramp
osc.setwaveform(1)
assert osc.output() == 0x0, hex(osc.output())  # triangle
osc.setwaveform(2)
assert osc.output() == 0x0, hex(osc.output())  # square
for i in range(4):
    osc.step()
assert osc.get_phase() != 0
osc.setwaveform(0)
assert osc.output() == 0x33333340, hex(osc.output())
osc.setwaveform(1)
assert osc.output() == 0x66666680, hex(osc.output())
osc.setwaveform(2)
assert osc.output() == 0x7fffffff, hex(osc.output())
for i in range(6):
    osc.step()
osc.setwaveform(0)
assert osc.output() == -0x7fffffe0, hex(osc.output())
osc.setwaveform(1)
assert osc.output() == -0x41, hex(osc.output())
osc.setwaveform(2)
assert osc.output() == -0x80000000, hex(osc.output())
for i in range(9):
    osc.step()
osc.setwaveform(0)
assert osc.output() == -0xccccc90, hex(osc.output())
osc.setwaveform(1)
assert osc.output() == -0x19999920, hex(osc.output())
osc.setwaveform(2)
assert osc.output() == -0x80000000, hex(osc.output())
osc.step()
osc.setwaveform(0)
assert osc.output() == 0x40, hex(osc.output())
osc.setwaveform(1)
assert osc.output() == 0x80, hex(osc.output())
osc.setwaveform(2)
assert osc.output() == 0x7fffffff, hex(osc.output())

adsr = instrument.ADSR()
adsr.setA(0.01)
adsr.setD(0.1)
adsr.setS(0.1)
adsr.setR(0.1)
assert adsr.output() == 0, hex(adsr.output())
adsr.keydown(1)
adsr.step()
assert adsr.output() == 0xa3d70a, hex(adsr.output())
adsr.step()
assert adsr.output() == 0x147ae14, hex(adsr.output())
adsr.step()
assert adsr.output() == 0x1eb851e, hex(adsr.output())
adsr.step()
assert adsr.output() == 0x28f5c28, hex(adsr.output())
for i in range(400 - 4):
    adsr.step()
assert adsr.state() == 1, adsr.state()
assert adsr.output() == 0xffffffa0, hex(adsr.output())
adsr.step()
assert adsr.state() == 2, adsr.state()
assert adsr.output() == 0xffffffff, hex(adsr.output())
adsr.step()
assert adsr.output() == 0xffffffff, hex(adsr.output())
adsr.step()
assert adsr.output() == 0xfff14197, hex(adsr.output())
for i in range(4000):
    adsr.step()
assert adsr.output() == 0x6e568fc8, hex(adsr.output())
for i in range(4000):
    adsr.step()
assert adsr.output() == 0x38c5f8f4, hex(adsr.output())
adsr.keydown(0)
adsr.step()
assert adsr.output() == 0x38c256e5, hex(adsr.output())
adsr.step()
assert adsr.output() == 0x38beb511, hex(adsr.output())
for i in range(25000):
    adsr.step()
assert adsr.output() == 0x1c033c, hex(adsr.output())
for i in range(25000):
    adsr.step()
assert adsr.output() == 0, hex(adsr.output())
