#include "effects.h"



float ez_distort(float input_)
{
    int adjust = 0.4f;
    float out_ = input_;
    if(out_ > 1 - adjust)
    {
        out_ = 1 - adjust;
    }
    else if(out_ < adjust){
        out_ = adjust;
    }
    return out_;
}

float advanced_distort(float x)
{
    return x;
}

float phasor = 0;
float crusher(float x, float normfreq, int bits )
{
    //wpm http://www.musicdsp.org/archive.php?classid=4
    float step = 1.0f/(1<<bits);
    float last = 0;

    phasor = phasor + normfreq;
    if (phasor >= 1.0f)
    {
      phasor = phasor - 1.0f;
      last = step * floor( x/step + 0.5f );
    }
    return last;
}

float overdrive(float x)
{
     return (3 - ((2-3*x)*(2-3*x)))/3.0f;
    //x = x*2.0f;
    if(x < 0.3333f)
        return 2*x;
    else if(x < 0.6666f)
        return (3 - ((2-3*x)*(2-3*x)))/3.0f;
    else
        return 1;
}

unsigned int vindex = 0;
float tremolo(float x)
{
    float pi=3.1415;
    float Fs = 5000.0f;
    float Fc = 1.0f;
    float alpha = 1.0f;
    float trem=(1+ alpha*sin(2*pi*vindex*(Fc/Fs)));
    float y = trem * x;
    vindex += 1;
    return y;
}

int is_init = 0;
float delay_line(float input)
{
    //if(is_init == 0)
    //{Delay_Init(5, 2, 1, 2); is_init = 1;}
    //return Delay_task(input);
}
