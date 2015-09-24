#!/bin/sh

if [ -n "$1" ]
then
    SYNTHREV="$1"
else
    SYNTHREV=master
fi

rm -rf Synth.cpp
git clone git@github.com:wware/Synth.cpp
(cd Synth.cpp; git checkout $SYNTHREV)

cp Synth.cpp/teensy/synth.h teensy/
cp Synth.cpp/teensy/synth.cpp teensy/
cp Synth.cpp/teensy/voice.h teensy/
