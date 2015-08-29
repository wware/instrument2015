#!/bin/sh

SYNTHREV=f4eb2a860be9205f98a6084feafbd36d505bb1ff

git clone git@github.com:wware/Synth.cpp
(cd Synth.cpp; git checkout $SYNTHREV)

cp Synth.cpp/teensy/synth.h teensy/
cp Synth.cpp/teensy/synth.cpp teensy/
cp Synth.cpp/teensy/voice.h teensy/
