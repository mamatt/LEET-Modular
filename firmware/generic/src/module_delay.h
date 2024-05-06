//-----10|-------20|-------30|-------40|-------50|-------60|-------70|-------80|
#ifndef __module_delay_h
#define __module_delay_h
extern "C"
{
#include <gd32vf103.h>
}

class module_delay
{
public:
    module_delay(){};
    uint16_t delay(uint16_t sample, uint16_t mix, uint16_t length);

private:
    uint16_t anHistory[4096]; // old readings from the analog input. Uses 8192 bytes of RAM...
    uint16_t anPos;           // index of the current reading
};
#endif
//-----10|-------20|-------30|-------40|-------50|-------60|-------70|-------80|