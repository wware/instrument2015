import math
import os
import sys


CMD = ("g++ -Wall -g -D__ARDUINO=0 -Iteensy -o foo instr.cpp teensy/common.cpp teensy/key.cpp")
assert os.system(CMD) == 0, CMD

if 'valgrind' in sys.argv[1:]:
    CMD = "valgrind --tool=callgrind --dump-instr=yes --collect-jumps=yes ./foo"
else:
    CMD = "./foo"
assert os.system(CMD) == 0, CMD

if 'gnuplot' in sys.argv[1:]:
    os.system("echo \"set term png; set output 'output.png';"
        " plot 'foo.gp' using 1:3 with lines, 'foo.gp' using 1:2 with lines\" | gnuplot")
    sys.exit(0)
# else:
#     assert sys.platform == 'darwin', 'Sound only works on the Mac'
elif sys.platform == 'darwin':
    import aifc
    player = "afplay"
    fname = "quux.aiff"

    def f(x):
        xhi = (x >> 8) & 0xff
        xlo = x & 0xff
        return chr(xhi) + chr(xlo)
else:
    import wave as aifc
    player = "audacious"
    fname = "quux.wav"

    def f(x):
        xhi = (x >> 8) & 0xff
        xlo = x & 0xff
        return chr(xlo) + chr(xhi)

import foo
S = foo.samples

try:
    os.unlink(fname)
except:
    pass

q = aifc.open(fname, "wb")
q.setnchannels(1)
q.setsampwidth(2)
q.setframerate(foo.sampfreq)
q.setnframes(len(S))

q.writeframes("".join(map(f, S)))
q.close()

os.system(player + " " + fname)
