#!/bin/bash

if false
then
    DEBUG="-debug-classes \
        -debug-symtabs \
        -debug-symbols \
        -debug-csymbols \
        -debug-lsymbols \
        -debug-tags \
        -debug-template \
        -debug-typedef \
        -debug-typemap \
        -debug-tmsearch \
        -debug-tmused"
fi

swig ${DEBUG} -c++ -python instrument.i || exit 1
g++ -fPIC -c stub.cpp ../teensy/key.cpp instrument_wrap.cxx -I/usr/include/python2.7 \
    -I/System/Library/Frameworks/Python.framework/Versions/2.7/include/python2.7/Python.h \
    -I../teensy || exit 1

if [ "$(uname -a | cut -c -6)" == "Darwin" ]
then
    # Mac
    OPTS="-lpython -dynamiclib"
else
    # Linux
    OPTS="-shared"
fi

g++ ${OPTS} stub.o key.o instrument_wrap.o -o _instrument.so || exit 1

python testit.py || exit 1

echo "Everything worked"
