#ifndef __adc_h
#define __adc_h
extern "C"
{
    #include "drivers/SysTick.h"
    #include <gd32vf103.h>
}

#define MAX_AN 5 // number of analog filters

class adc
{
public:
    adc(){};
    void init(SysTick &stk) ;    // initialise ADC
    uint16_t anRaw[8];   // raw sampled values stored by DMA
    int anAvg[MAX_AN];   // analog average
    uint8_t anCount;     // counter for oversampling (new value after 255, @44,4KHz => 173Hz, suitable for potentiometers)
    void update()   ;    // update ADC

private:
    uint32_t anTotal[MAX_AN]; // the running total
    void updateAVG(uint8_t ch) ;
};
#endif