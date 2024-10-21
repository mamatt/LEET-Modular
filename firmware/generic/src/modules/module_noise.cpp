
#include "module_noise.h"
#include "drivers/display.h"

// constructor
module_noise::module_noise(void) {
    this-> Rnd = 4294967294;
}

// simple pseudo random noise generator
uint16_t module_noise::prng(uint16_t volume) {
  LowBit = this->Rnd & 1;
  this->Rnd >>= 1;
  this->Rnd ^= LowBit ? 0x80000057ul : 0ul;
  if (LowBit)
    return (2048+volume); // 2048 = 0V
  else
    return (2048-volume);
}

// main sample processing
int16_t module_noise::process(uint16_t in1, int potLL)
{
      uint32_t cv1;
      cv1 = (in1 * potLL >> 13);      // pot2 (bottom right) sets level for CV1 that attenuates or amplifies amplitude
      return prng(cv1); 
}