#ifndef __dac_h
#define __dac_h
extern "C"
{
    #include <gd32vf103.h>
    #include "drivers/display.h"
}

class dac {
    public:
        void init(void) ;
        void dac_out(uint16_t d0, uint16_t d1) ;
};
#endif