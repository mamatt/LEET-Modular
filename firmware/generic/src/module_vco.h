//-----10|-------20|-------30|-------40|-------50|-------60|-------70|-------80|
#ifndef __module_vco_h
#define __module_vco_h
extern "C"
{
#include <gd32vf103.h>
}

class module_vco
{
public:
    module_vco(){};
    uint8_t phaseTrig;
    float sine(float xTmp);
    // float sine3x(float xTmp, uint16_t dist);
    uint16_t square(float xTmp, uint16_t pw);
    uint16_t square2x(float xTmp, uint16_t dist);
    uint16_t square3x(float xTmp, uint16_t dist);
    uint16_t triangle(float xTmp, uint16_t shape);
    uint16_t triangle2x(float xTmp, uint16_t dist);
    uint16_t triangle3x(float xTmp, uint16_t dist);
    //    uint16_t int_saw(uint16_t x);
    //    uint16_t int_r_saw(uint16_t x);

private:
    float phase, phase1, phase2, phase3;
    float oldPhase;
    float sin(float x);
};
#endif
//-----10|-------20|-------30|-------40|-------50|-------60|-------70|-------80|