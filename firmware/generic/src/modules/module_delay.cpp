#include "module_delay.h"
extern "C"
{
#include "gd32vf103.h"
}

/*  potentiometer and I/O mapping
    R  P2
    P4 P3
    C0 C1
    D0 D1
*/

uint16_t module_delay::delay(uint16_t sample, uint16_t mix, uint16_t length)
{
    static uint16_t anHistory[4096];    // old readings from the analog input. Uses 8192 bytes of RAM...
    static uint16_t anPos = 0;          // index of the current reading
    uint16_t old;                       //
    uint16_t tmp;                       //
    if (anPos >= 4095)                  // Are we're at the end of the array ?
        anPos = 0;                      // ...yes, wrap around to the beginning
    old = anHistory[anPos];             // get saved sample
    tmp = old * mix >> 12;              // mix old sample..
    tmp += sample * (4095 - mix) >> 12; // ..with new sample
    anHistory[anPos] = tmp;             // save new sample
    anPos++;                            // advance to the next position in the array:
    return (tmp);                       // return (mix)
}
