
#include "module_vco.h"
#include "res/colors.h"
#include "res/images.h"
#include "drivers/display.h"
#include <stdio.h>

extern "C" {
    #include "gd32vf103.h"
    extern "C" const float freq[2113];
}

//Sine Amplitude modulation
int16_t module_vco::processSineAM(uint16_t note, uint16_t amplitude, int amplitudeLevel, int octave, int fine) 
{
    int16_t tmp = 0;
    uint32_t cv2;

    cv2 = ((amplitude << 2) * amplitudeLevel >> 11);                            // 0-8192. pot2 (bottom right) sets level for CV2 that attenuates or amplifies amplitude
    tmp = 2048 + ((int(sine(getPhase(note,octave,fine)) - 2048) * cv2) >> 12);  // get level from position
    return fold(tmp);                                                           // fold if outside range
}

//Sine Mix
int16_t module_vco::processSineADD(uint16_t note, uint16_t sample, int sampleLevel, int octave, int fine) 
{
    int16_t tmp = 0;
    uint32_t cv2;

    cv2 = ((sample << 2) * sampleLevel >> 11);                  // 0-8192. pot2 (bottom right) sets level for CV2 that attenuates or amplifies amplitude
    tmp = (int(sine(getPhase(note,octave,fine)) + cv2) >> 1);   // get level from position
    return fold(tmp);                                           // fold if outside range
}

//Sine FM
int16_t module_vco::processSineFM(uint16_t note, uint16_t frequency, int frequencyLevel, int octave, int fine) 
{
    int16_t tmp = 0;
    uint32_t cv2;

    cv2 = (frequency * frequencyLevel >> 12);                  // 12+12-13=11 (0-2048). pot2 (bottom right) sets level for CV2 that modulates the frequency
    tmp = int(sine(getPhase(note,octave,fine) + cv2 - 1024));   // get level from position (1088-3008 => -5V to +5V)
    return fold(tmp);                                           // fold if outside range
          
}

// sine 
float module_vco::sine(float xTmp) {
    if (xTmp > 0)
    {
        phase += xTmp;
        if (phase > 65535)
        {
            phase -= 65535;
        }
        if (phase > 32768 && oldPhase < 32768)
            phaseTrig = 1;
        else
            phaseTrig = 0;
        oldPhase = phase;
    }
    return (sin(phase));
}

// square -  pw (16 bit) controls pulse width / duty cycle
uint16_t module_vco::square(float xTmp, uint16_t pw) {
    phase += xTmp;
    if (phase > 65535)
    {
        phase -= 65535;
    }
    if (phase > 32768 && oldPhase < 32768)
        phaseTrig = 1;
    else
        phaseTrig = 0;
    oldPhase = phase;
    if (phase > pw)
        return (1088); // 1088 = -5V
    else
        return (3008); // 3008 = +5V
}

// square -  pw (16 bit) controls pulse width / duty cycle
uint16_t module_vco::square2x(float xTmp, uint16_t dist) {
    int16_t out1, out2, out3;
    uint16_t pw = 32768; // 50%pw / 2
    phase2 += xTmp;
    if (phase2 > 65535) phase2 -= 65535;
    if (phase2 > 32768 && oldPhase < 32768) phaseTrig = 1;
    else phaseTrig = 0;
    oldPhase = phase2;
    if (phase2 > pw) out2 = -480; // 1088 = -5V
    else out2 = 480; // 3008 = +5V
    phase1 += xTmp - dist;
    if (phase1 > 65535) phase1 -= 65535;
    if (phase1 > pw) out1 = -480; // 10192/88 = -5V
    else out1 = 480; // 3008 = +5V
    phase3 += xTmp + dist;
    if (phase3 > 65535) phase3 -= 65535;
    if (phase3 > pw) out3 = -480; // 1088 = -5V
    else out3 = 480; // 3008 = +5V
    return(2048+out1+out3);
}

// square -  pw (16 bit) controls pulse width / duty cycle
uint16_t module_vco::square3x(float xTmp, uint16_t dist) {
    int16_t out1, out2, out3;
    uint16_t pw = 32768; // 50%pw / 2
    phase2 += xTmp;
    if (phase2 > 65535) phase2 -= 65535;
    if (phase2 > 32768 && oldPhase < 32768) phaseTrig = 1;
    else phaseTrig = 0;
    oldPhase = phase2;
    if (phase2 > pw) out2 = -320; // 1088 = -5V
    else out2 = 320; // 3008 = +5V
    phase1 += xTmp - dist;
    if (phase1 > 65535) phase1 -= 65535;
    if (phase1 > pw) out1 = -320; // 10192/88 = -5V
    else out1 = 320; // 3008 = +5V
    phase3 += xTmp + dist;
    if (phase3 > 65535) phase3 -= 65535;
    if (phase3 > pw) out3 = -320; // 1088 = -5V
    else out3 = 320; // 3008 = +5V
    return(2048+out1+out2+out3);
}

// triangle -  shape (16 bit) morphs between reverse saw, triangle and saw (could use bresenham to avoid division...)
uint16_t module_vco::triangle(float xTmp, uint16_t shape) {
    phase += xTmp;
    if (phase > 65535)
    {
        phase -= 65535;
    }
    if (phase > 32768 && oldPhase < 32768)
        phaseTrig = 1;
    else
        phaseTrig = 0;
    oldPhase = phase;

    if (phase < (65535 - shape))                                 // 12 bit -> 16 bit
        return (4095 - 1088 - (phase * 1920) / (65535 - shape)); // x*4095/(anAvg[2]*16)
    else
        return (4095 - 1088 - ((65535 - phase) * 1920) / (shape));
}


// triangle -  shape (16 bit) morphs between reverse saw, triangle and saw (could use bresenham to avoid division...)
uint16_t module_vco::triangle2x(float xTmp, uint16_t dist) {
    uint16_t out1,out2,out3;
    //uint16_t shape = 32768; // triangle shape (not saw)
    uint16_t shape = 65000; // saw shape
    phase2 += xTmp;
    if (phase2 > 65535) phase2 -= 65535;
    if (phase2 > 32768 && oldPhase < 32768) phaseTrig = 1;
    else phaseTrig = 0;
    oldPhase = phase2;

    if (phase2 < (65535 - shape))                                 // 12 bit -> 16 bit
        out2 = (4095 - 1088 - (phase2 * 1920) / (65535 - shape)); // x*4095/(anAvg[2]*16)
    else
        out2 = (4095 - 1088 - ((65535 - phase2) * 1920) / (shape));

    phase1 += xTmp - dist;
    if (phase1 > 65535) phase1 -= 65535;
    if (phase1 < (65535 - shape)) out1 = (4095 - 1088 - (phase1 * 1920) / (65535 - shape));
    else out1 = (4095 - 1088 - ((65535 - phase1) * 1920) / (shape));

    phase3 += xTmp + dist;
    if (phase3 > 65535) phase3 -= 65535;
    if (phase3 < (65535 - shape)) out3 = (4095 - 1088 - (phase3 * 1920) / (65535 - shape));
    else out3 = (4095 - 1088 - ((65535 - phase3) * 1920) / (shape));
    return((out1+out3)>>1);
}

// triangle -  shape (16 bit) morphs between reverse saw, triangle and saw (could use bresenham to avoid division...)
uint16_t module_vco::triangle3x(float xTmp, uint16_t dist) {
    uint16_t out1,out2,out3;
    //uint16_t shape = 32768; // triangle shape (not saw)
    uint16_t shape = 65000; // saw shape
    phase2 += xTmp;
    if (phase2 > 65535) phase2 -= 65535;
    if (phase2 > 32768 && oldPhase < 32768) phaseTrig = 1;
    else phaseTrig = 0;
    oldPhase = phase2;

    if (phase2 < (65535 - shape))                                 // 12 bit -> 16 bit
        out2 = (4095 - 1088 - (phase2 * 1920) / (65535 - shape)); // x*4095/(anAvg[2]*16)
    else
        out2 = (4095 - 1088 - ((65535 - phase2) * 1920) / (shape));

    phase1 += xTmp - dist;
    if (phase1 > 65535) phase1 -= 65535;
    if (phase1 < (65535 - shape)) out1 = (4095 - 1088 - (phase1 * 1920) / (65535 - shape));
    else out1 = (4095 - 1088 - ((65535 - phase1) * 1920) / (shape));

    phase3 += xTmp + dist;
    if (phase3 > 65535) phase3 -= 65535;
    if (phase3 < (65535 - shape)) out3 = (4095 - 1088 - (phase3 * 1920) / (65535 - shape));
    else out3 = (4095 - 1088 - ((65535 - phase3) * 1920) / (shape));
    return(((out1+out3)>>1)+out2>>1);
}


float module_vco::sin(float x) {
    #define PI 3.1415926535897932384626433832795028841971693993751058209749445923078164062
    #define r360 2 * PI
    #define r180 PI
    #define r90 PI / 2
    #define PIR PI / 32768 // 65536/2

    x = x * PIR;       // convert uint16_t to rad
                       //    x = x * PI / 180;  // convert degrees to rad
    int sign = 1;
    if (x > r180)
    {
        x = r360 - x;
        sign = -1;
    }
    if (x > r90)
    {
        x = r180 - x;
    }
    float x2 = x * x;
    float y = sign * x * (x2 * (x2 * (42 - x2) - 840) + 5040) / 5040;
    y = y * 960 + 2048; // convert from +/-1 to 1088 - 3008 (-5V to +5V)
    return (y);
};

int16_t module_vco::fold(int16_t tmp) {
  do
  {
    if (tmp < 1088)
      tmp = (1088 * 2 - tmp);
    if (tmp > 3008)
      tmp = (3008 * 2 - tmp);
  } while (tmp < 1088 || tmp > 3008);
  return (tmp);
};

uint16_t module_vco::deadzone(uint16_t tmp) {
  if (tmp < 255)        // within?
    return (0);         // ..yes - return zero
  else                  // .. no
    return (tmp - 255); // return value - deadzone
};

float module_vco::ad2phase(uint16_t in) {
  float xTmp;
  if (in < 1472) // below -3V
    xTmp = 24.135547645748918;
  else if (in > 3584) // above 8V
    xTmp = 49429.601578493784;
  else
  {
    if (quantized == 0)
      xTmp = freq[in - 1472];
    else
    {
      uint16_t q = (in - 1472 + 8) >> 4; // add 16/2 to quantize between notes
      xTmp = freq[q << 4];
    }
  }
  return (xTmp);
}

float module_vco::getPhase(int cv, int octave, int note ) {
  uint8_t p_note;
  uint16_t p_octave;
  p_note = deadzone(note) >> 4;                           // add deadzone, one octave is 192 AD steps (1V). (4096-1024/16)=192
  p_octave = (octave >> 9) * 192;                         // 4096/512 => 8 octaves
  phaseC = ad2phase(cv + p_octave - (192 << 1) + p_note); // convert CV to logaritmic scale
  return (phaseC);
}

/*  potentiometer and I/O mapping
    R  P2
    P4 P3
    C0 C1
    D0 D1
*/

