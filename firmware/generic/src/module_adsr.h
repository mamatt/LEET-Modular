#ifndef __module_adsr_h
#define __module_adsr_h
extern "C"
{
#include "drivers/display.h"
}

class module_adsr
{
public:
    module_adsr(){};
    uint16_t int_adsr(uint16_t gate, uint16_t pot_a, uint16_t pot_d, uint16_t pot_r, uint8_t noSustain);
    void draw(display &disp, uint16_t gate, uint16_t pot_a, uint16_t pot_d, uint16_t pot_r, uint8_t noSustain);
    void init(display &disp);

private:
    uint16_t tCount;
    uint8_t adsrMode;
    uint8_t oldGate;
    uint8_t xorGate;
    int8_t gateCount;
    uint16_t a, d, s, r;
};
#endif