#ifndef EFFECTS_H
#define EFFECTS_H

#include "math.h"

float ez_distort(float input_);
float advanced_distort(float x);
float crusher(float x, float normfreq, int bits );
float overdrive(float x);
float tremolo(float x);
float delay_line(float input);
float fuzz_exp(float x);


#endif
