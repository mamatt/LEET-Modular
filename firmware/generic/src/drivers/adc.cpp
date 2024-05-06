#include "adc.h"
extern "C"
{
#include "gd32vf103.h"
}

// anUpdate (downshifted oversampling)
uint8_t anCount = 0;      // counter for oversampling (new value after 255, @44,4KHz => 173Hz, suitable for potentiometers)
uint32_t anTotal[MAX_AN]; // the running total
int anAvg[MAX_AN];        // analog average
uint16_t anRaw[8];          // raw sampled values stored by DMA

// analog average (downshifted oversampling)
void adc::update(uint8_t ch)
{
    if (anCount < 255)
    {
        anTotal[ch] += anRaw[ch];
    }
    else
    {
        if (ch == 2 || ch == 3)                    // these potentiometers are inverted due to routing
            anAvg[ch] = 4095 - (anTotal[ch] >> 8); // note that raw[2] and raw[3] is still inverted...
        else
            anAvg[ch] = anTotal[ch] >> 8; // inputs and pot 3 is not inverted
        anTotal[ch] = 0;
    }
}