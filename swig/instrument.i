%module instrument
%include "typemaps.i"

%{
#define SWIG_FILE_WITH_INIT
#include "../teensy/key.h"
#include "../teensy/synth.h"
#include "../teensy/voice.h"

extern int32_t mult_signed(int32_t x, int32_t y);
extern int32_t mult_unsigned(uint32_t x, uint32_t y);
extern int32_t mult_unsigned_signed(uint32_t x, int32_t y);
%}

%apply char { int8_t }
%apply int { int32_t }
%apply long long { int64_t }

%apply unsigned char { uint8_t }
%apply unsigned int { uint32_t }
%apply unsigned long long { uint64_t }

%apply unsigned int *OUTPUT { uint32_t * }

extern int32_t mult_signed(int32_t x, int32_t y);
extern int32_t mult_unsigned(uint32_t x, uint32_t y);
extern int32_t mult_unsigned_signed(uint32_t x, int32_t y);

%include "../teensy/key.h"
%include "../teensy/synth.h"
%include "../teensy/voice.h"
