//-----10|-------20|-------30|-------40|-------50|-------60|-------70|-------80|
#ifndef __module_noise_h
#define __module_noise_h
#include "drivers\display.h"

class module_noise
{
public:
    module_noise(){};
    uint16_t prng(uint16_t volume);
    void init(display &disp);

private:
    uint32_t Rnd;   // random seed
    uint8_t LowBit; // noise out
};
#endif
//-----10|-------20|-------30|-------40|-------50|-------60|-------70|-------80|