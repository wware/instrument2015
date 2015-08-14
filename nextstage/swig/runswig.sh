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
g++ -fPIC -c stub.cpp ../teensy/key.cpp instrument_wrap.cxx -I/usr/include/python2.7 -I../teensy || exit 1
g++ -shared stub.o key.o instrument_wrap.o -o _instrument.so || exit 1

python testit.py || exit 1

echo "Everything worked"
