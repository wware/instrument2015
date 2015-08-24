#!/bin/sh

mkdir -p teensy/
(cd teensy; ln -s ../teensy.ino)
(cd teensy; ln -s ../voice.h)
git clone git@github.com:wware/Synth.cpp.git
cp Synth.cpp/teensy/synth.h teensy/
cp Synth.cpp/teensy/synth.cpp teensy/
