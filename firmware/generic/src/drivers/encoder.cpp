extern "C"
{
#include "gd32vf103.h"
}
#include "encoder.h"

// Check rotation encoder for changes.
// Does not use gpio interrupts, designed to be called @ ~44Khz inside timer int.

char encoder::get()
{
    uint16_t portB = gpio_input_port_get(GPIOB) & 0x70; // PB4 select, PB5 & PB6 encoder data
    if (portB == 0x70)
    {
        if (ccCount > 0)
            ccCount--;
        if (cwCount > 0)
            cwCount--;
        if (ccCount == 0 && cwCount == 0)
        {
            encDir = 0;
        }
    }
    else if (cwCount < 32 && portB == 0x50)
        cwCount++;
    else if (ccCount < 32 && portB == 0x30)
        ccCount++;

    // test clockwise
    if (cwCount == 32 && encDir == 0)
    {
        encDir = 'W';
        return 'W';
    }

    // test counter clockwise
    else if (ccCount == 32 && encDir == 0)
    {
        encDir = 'C';
        return 'C';
    }

    // test encoder click
    else if (portB == 0x60 && encDir == 0)
    {
        encDir = 'S';
        ccCount = 32; // delay before reset (remove debounce jitter)
        return 'S';
    }
    return 0;
}