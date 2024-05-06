//-----10|-------20|-------30|-------40|-------50|-------60|-------70|-------80|
#include "module_vco.h"
extern "C"
{
#include "gd32vf103.h"
}

float module_vco::sine(float xTmp)
{
    if (xTmp > 0)
    {
        phase += xTmp;
        if (phase > 65535)
        {
            phase -= 65535;
        }
        if (phase > 32768 && oldPhase < 32768)
            phaseTrig = 1;
        else
            phaseTrig = 0;
        oldPhase = phase;
    }
    return (sin(phase));
}


// square -  pw (16 bit) controls pulse width / duty cycle
uint16_t module_vco::square(float xTmp, uint16_t pw)
{
    phase += xTmp;
    if (phase > 65535)
    {
        phase -= 65535;
    }
    if (phase > 32768 && oldPhase < 32768)
        phaseTrig = 1;
    else
        phaseTrig = 0;
    oldPhase = phase;
    if (phase > pw)
        return (1088); // 1088 = -5V
    else
        return (3008); // 3008 = +5V
}
// square -  pw (16 bit) controls pulse width / duty cycle
uint16_t module_vco::square2x(float xTmp, uint16_t dist)
{
    int16_t out1, out2, out3;
    uint16_t pw = 32768; // 50%pw / 2
    phase2 += xTmp;
    if (phase2 > 65535) phase2 -= 65535;
    if (phase2 > 32768 && oldPhase < 32768) phaseTrig = 1;
    else phaseTrig = 0;
    oldPhase = phase2;
    if (phase2 > pw) out2 = -480; // 1088 = -5V
    else out2 = 480; // 3008 = +5V
    phase1 += xTmp - dist;
    if (phase1 > 65535) phase1 -= 65535;
    if (phase1 > pw) out1 = -480; // 10192/88 = -5V
    else out1 = 480; // 3008 = +5V
    phase3 += xTmp + dist;
    if (phase3 > 65535) phase3 -= 65535;
    if (phase3 > pw) out3 = -480; // 1088 = -5V
    else out3 = 480; // 3008 = +5V
    return(2048+out1+out3);
}
// square -  pw (16 bit) controls pulse width / duty cycle
uint16_t module_vco::square3x(float xTmp, uint16_t dist)
{
    int16_t out1, out2, out3;
    uint16_t pw = 32768; // 50%pw / 2
    phase2 += xTmp;
    if (phase2 > 65535) phase2 -= 65535;
    if (phase2 > 32768 && oldPhase < 32768) phaseTrig = 1;
    else phaseTrig = 0;
    oldPhase = phase2;
    if (phase2 > pw) out2 = -320; // 1088 = -5V
    else out2 = 320; // 3008 = +5V
    phase1 += xTmp - dist;
    if (phase1 > 65535) phase1 -= 65535;
    if (phase1 > pw) out1 = -320; // 10192/88 = -5V
    else out1 = 320; // 3008 = +5V
    phase3 += xTmp + dist;
    if (phase3 > 65535) phase3 -= 65535;
    if (phase3 > pw) out3 = -320; // 1088 = -5V
    else out3 = 320; // 3008 = +5V
    return(2048+out1+out2+out3);
}

// triangle -  shape (16 bit) morphs between reverse saw, triangle and saw (could use bresenham to avoid division...)
uint16_t module_vco::triangle(float xTmp, uint16_t shape)
{
    phase += xTmp;
    if (phase > 65535)
    {
        phase -= 65535;
    }
    if (phase > 32768 && oldPhase < 32768)
        phaseTrig = 1;
    else
        phaseTrig = 0;
    oldPhase = phase;

    if (phase < (65535 - shape))                                 // 12 bit -> 16 bit
        return (4095 - 1088 - (phase * 1920) / (65535 - shape)); // x*4095/(anAvg[2]*16)
    else
        return (4095 - 1088 - ((65535 - phase) * 1920) / (shape));
}


// triangle -  shape (16 bit) morphs between reverse saw, triangle and saw (could use bresenham to avoid division...)
uint16_t module_vco::triangle2x(float xTmp, uint16_t dist)
{
    uint16_t out1,out2,out3;
    //uint16_t shape = 32768; // triangle shape (not saw)
    uint16_t shape = 65000; // saw shape
    phase2 += xTmp;
    if (phase2 > 65535) phase2 -= 65535;
    if (phase2 > 32768 && oldPhase < 32768) phaseTrig = 1;
    else phaseTrig = 0;
    oldPhase = phase2;

    if (phase2 < (65535 - shape))                                 // 12 bit -> 16 bit
        out2 = (4095 - 1088 - (phase2 * 1920) / (65535 - shape)); // x*4095/(anAvg[2]*16)
    else
        out2 = (4095 - 1088 - ((65535 - phase2) * 1920) / (shape));

    phase1 += xTmp - dist;
    if (phase1 > 65535) phase1 -= 65535;
    if (phase1 < (65535 - shape)) out1 = (4095 - 1088 - (phase1 * 1920) / (65535 - shape));
    else out1 = (4095 - 1088 - ((65535 - phase1) * 1920) / (shape));

    phase3 += xTmp + dist;
    if (phase3 > 65535) phase3 -= 65535;
    if (phase3 < (65535 - shape)) out3 = (4095 - 1088 - (phase3 * 1920) / (65535 - shape));
    else out3 = (4095 - 1088 - ((65535 - phase3) * 1920) / (shape));
    return((out1+out3)>>1);
}

// triangle -  shape (16 bit) morphs between reverse saw, triangle and saw (could use bresenham to avoid division...)
uint16_t module_vco::triangle3x(float xTmp, uint16_t dist)
{
    uint16_t out1,out2,out3;
    //uint16_t shape = 32768; // triangle shape (not saw)
    uint16_t shape = 65000; // saw shape
    phase2 += xTmp;
    if (phase2 > 65535) phase2 -= 65535;
    if (phase2 > 32768 && oldPhase < 32768) phaseTrig = 1;
    else phaseTrig = 0;
    oldPhase = phase2;

    if (phase2 < (65535 - shape))                                 // 12 bit -> 16 bit
        out2 = (4095 - 1088 - (phase2 * 1920) / (65535 - shape)); // x*4095/(anAvg[2]*16)
    else
        out2 = (4095 - 1088 - ((65535 - phase2) * 1920) / (shape));

    phase1 += xTmp - dist;
    if (phase1 > 65535) phase1 -= 65535;
    if (phase1 < (65535 - shape)) out1 = (4095 - 1088 - (phase1 * 1920) / (65535 - shape));
    else out1 = (4095 - 1088 - ((65535 - phase1) * 1920) / (shape));

    phase3 += xTmp + dist;
    if (phase3 > 65535) phase3 -= 65535;
    if (phase3 < (65535 - shape)) out3 = (4095 - 1088 - (phase3 * 1920) / (65535 - shape));
    else out3 = (4095 - 1088 - ((65535 - phase3) * 1920) / (shape));
    return(((out1+out3)>>1)+out2>>1);
}

/*
uint16_t module_vco::int_saw(uint16_t x)
{
    uint32_t tmp = 4095 - (x >> 4); // (16 bit -> 12 bit)
    return (tmp);
}

uint16_t module_vco::int_r_saw(uint16_t x)
{
    uint32_t tmp = x >> 4; // (16 bit -> 12 bit)
    return (tmp);
}
*/
float module_vco::sin(float x)
{
#define PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062
#define r360 2 * PI
#define r180 PI
#define r90 PI / 2
#define PIR PI / 32768 // 65536/2
    x = x * PIR;       // convert uint16_t to rad
                       //    x = x * PI / 180;  // convert degrees to rad
    int sign = 1;
    if (x > r180)
    {
        x = r360 - x;
        sign = -1;
    }
    if (x > r90)
    {
        x = r180 - x;
    }
    float x2 = x * x;
    float y = sign * x * (x2 * (x2 * (42 - x2) - 840) + 5040) / 5040;
    y = y * 960 + 2048; // convert from +/-1 to 1088 - 3008 (-5V to +5V)
    return (y);
}
//-----10|-------20|-------30|-------40|-------50|-------60|-------70|-------80|