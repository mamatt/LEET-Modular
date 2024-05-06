#ifndef __encoder_h
#define __encoder_h
extern "C"
{
#include <gd32vf103.h>
}

class encoder
{
public:
    encoder(){};
    char get();

private:
    char encDir;     // encoder direction
    uint8_t cwCount; // clockwise encoder count
    uint8_t ccCount; // counter clockwise encoder count
};
#endif