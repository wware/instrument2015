%module instrument
%include "typemaps.i"

%{
#define SWIG_FILE_WITH_INIT
#include "../teensy/key.h"
#include "../teensy/queue.h"
%}

%apply unsigned char { uint8_t }
%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }

%apply unsigned int *OUTPUT { uint32_t * }

%include "../teensy/key.h"
%include "../teensy/queue.h"
