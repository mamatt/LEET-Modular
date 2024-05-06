//-----10|-------20|-------30|-------40|-------50|-------60|-------70|-------80|
#ifndef __module_vcf_h
#define __module_vcf_h
extern "C"
{
#include <gd32vf103.h>
}

class module_vcf
{
public:
    module_vcf(){};
    uint8_t phaseTrig;
    int16_t filterValue;
    int16_t filterOut;
    uint16_t r4pole1(uint16_t sample, uint16_t cutoff, uint16_t resonance);
    uint16_t r2pole1(uint16_t sample, uint16_t cutoff, uint16_t resonance);
    uint16_t r2pole2(uint16_t sample, uint16_t cutoff, uint16_t resonance);
    uint16_t test(uint16_t sample, uint16_t cutoff, uint16_t resonance); 

private:
    static float in1, in2, in3, in4, buf1, buf2, buf3, buf4, fb, in, cut, q;
    static int16_t oldOut;
};
#endif
//-----10|-------20|-------30|-------40|-------50|-------60|-------70|-------80|