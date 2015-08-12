import math
import os
import sys

if sys.platform == 'linux2':
    # Ubuntu
    import wave
    fname = "quux.wav"
    player = "mplayer"
else:
    # Mac
    import aifc as wave
    fname = "quux.aiff"
    player = "afplay"

CMD = "g++ -Wall -g -D__ARDUINO=0 -Iteensy -o foo instr.cpp teensy/key.cpp teensy/voice.cpp"

assert os.system(CMD) == 0
assert os.system("./foo") == 0

if 'gnuplot' in sys.argv[1:]:
    os.system("echo \"plot 'foo.gp' using 1:3 with lines; pause 10\" | gnuplot")
    sys.exit(0)

import foo
S = foo.samples

try:
    os.unlink(fname)
except:
    pass

q = wave.open(fname, "wb")
q.setnchannels(1)
q.setsampwidth(2)
q.setframerate(foo.sampfreq)
q.setnframes(len(S))

def f(x):
    xhi = (x >> 8) & 0xff
    xlo = x & 0xff
    return chr(xhi) + chr(xlo)

q.writeframes("".join(map(f, S)))
q.close()

os.system(player + " " + fname)