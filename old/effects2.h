#ifndef _ZE_EFFECTS_H
#define _ZE_EFFECTS_H

#include "math.h"
#include "mbed.h"

float ez_distort(float input_);
float advanced_distort(float x);
float crusher(float x, float normfreq, int bits );
float overdrive(float x);
float tremolo(float x);
float delay_line(float input);


#endif
