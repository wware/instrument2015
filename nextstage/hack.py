import math
import os
import aifc

assert os.system("g++ -Wall -g -o foo instr.cpp") == 0
assert os.system("./foo > foo.py") == 0

import foo
S = foo.samples

try:
    os.unlink("quux.aiff")
except:
    pass

q = aifc.open("quux.aiff", "wb")
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

os.system("afplay quux.aiff")