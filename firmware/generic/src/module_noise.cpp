//-----10|-------20|-------30|-------40|-------50|-------60|-------70|-------80|
#include "module_noise.h"
#include "drivers/display.h"

// compressed image generated with python script (used by init)
const uint8_t img_rnd[175] = {255, 255, 255, 255, 255, 255, 255, 255, 132, 59, 15, 59, 9, 65, 25, 1, 59, 0, 12, 1, 59, 0, 74, 0, 23, 0, 62, 0, 10, 0, 62, 0, 74, 1, 20, 0, 64, 0, 8, 0, 64, 0, 95, 0, 66, 0, 6, 0, 66, 0, 74, 0, 239, 0, 16, 0, 68, 0, 4, 0, 68, 0, 255, 255, 255, 255, 43, 48, 26, 48, 26, 48, 255, 255, 255, 233, 48, 255, 255, 255, 255, 189, 0, 255, 222, 0, 237, 0, 237, 0, 237, 0, 183, 41, 255, 224, 0, 11, 0, 255, 211, 0, 11, 0, 255, 79, 48, 82, 0, 11, 0, 255, 211, 0, 11, 0, 245, 0, 219, 0, 11, 0, 74, 0, 74, 0, 93, 0, 219, 0, 11, 0, 72, 0, 74, 0, 95, 0, 64, 0, 74, 0, 77, 0, 11, 0, 5, 1, 61, 0, 74, 0, 100, 1, 56, 2, 73, 1, 20, 10, 48, 12, 9, 3, 48, 3, 11, 64, 255, 255, 255, 255, 255, 255, 165};
#define logoColor RGBToWord(0x00, 0xff, 0xff)
//#define black RGBToWord(0x00, 0x00, 0x00)

/*  potentiometer and I/O mapping
    R  P2
    P4 P3
    C0 C1
    D0 D1
*/

// simple pseudo random noise generator
uint16_t module_noise::prng(uint16_t volume)
{
  LowBit = Rnd & 1;
  Rnd >>= 1;
  Rnd ^= LowBit ? 0x80000057ul : 0ul;
  if (LowBit)
    return (2048+volume); // 2048 = 0V
  else
    return (2048-volume);
}

void module_noise::init(display &disp)
{
  Rnd = 4294967294;
    // show logo
    disp.showImg(img_rnd, sizeof(img_rnd), 0, 0, 240, 60, logoColor);
}


//-----10|-------20|-------30|-------40|-------50|-------60|-------70|-------80|