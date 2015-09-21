#!/bin/sh

SYNTHREV=76cca934c0e39a44600465eebf73122083094a53

git clone git@github.com:wware/Synth.cpp
(cd Synth.cpp; git checkout $SYNTHREV)

cp Synth.cpp/teensy/synth.h teensy/
cp Synth.cpp/teensy/synth.cpp teensy/
cp Synth.cpp/teensy/voice.h teensy/
