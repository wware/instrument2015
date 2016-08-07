#!/usr/bin/env python

import os
import re
import time

assert os.system("fluidsynth --server --no-shell --audio-driver=alsa /usr/share/sounds/sf2/FluidR3_GM.sf2 &") == 0
time.sleep(2)

R = os.popen('ps ax | grep fluidsynth | grep -v grep').read()
if not R:
    os.system("ps ax | grep fluidsynth | cut -c -6 | xargs kill -9")
    assert R, R

connected = False
R = os.popen('aconnect -l').read()
r2 = re.compile(r"client (\d+): 'FLUID Synth").search(R)
if not r2:
    os.system("ps ax | grep fluidsynth | cut -c -6 | xargs kill -9")
    assert r2, R

fluid_num = r2.group(1)

try:
    while True:
        time.sleep(1)
        R = os.popen('aconnect -l').read()
        r1 = re.compile(r"client (\d+): 'Teensy MIDI'").search(R)
        if not r1:
            connected = False
            continue
        if connected:
            continue
        os.system("aconnect %s %s" % (r1.group(1), r2.group(1)))
        connected = True
except KeyboardInterrupt:
    os.system("ps ax | grep fluidsynth | cut -c -6 | xargs kill -9")
