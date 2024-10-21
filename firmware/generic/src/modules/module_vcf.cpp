
#include "module_vcf.h"
#include "drivers/display.h"
#include "res/colors.h"
#include <stdio.h>

extern "C" {
    #include "gd32vf103.h"
}

// 4 pole ladder resonant low pass filter (Karlsen fast ladder)
uint16_t module_vcf::r4pole1(uint16_t sample, uint16_t cutoff, uint16_t resonance) {
    static int16_t oldOut;
    if (sample > 2048 && oldOut < 2048)
        phaseTrig = 1;
    else
        phaseTrig = 0;
    oldOut = sample;

    static float buf1, buf2, buf3, buf4, fb, in, cut, q;
    in = float(sample - 2048) / 2048;
    cut = float(cutoff) / 4096;
    q = float(resonance) / 512; // 0 to 64
    fb = buf4;
    if (fb > 1)
        fb = 1;
    if (fb < -1)
        fb = -1;
    in = in - (fb * q);
    buf1 = ((in - buf1) * cut) + buf1;
    buf2 = ((buf1 - buf2) * cut) + buf2;
    buf3 = ((buf2 - buf3) * cut) + buf3;
    buf4 = ((buf3 - buf4) * cut) + buf4;

    filterOut = buf4 * 2048 + 2048;
    if (buf4 > 1)
        filterOut = 4095;
    else if (buf4 < -1)
        filterOut = 0;
    return (filterOut);
}

// 2-pole resonant low pass filter (Paul Kellett)
uint16_t module_vcf::r2pole1(uint16_t sample, uint16_t cutoff, uint16_t resonance) {
    static int16_t oldOut;
    if (sample > 2048 && oldOut < 2048)
        phaseTrig = 1;
    else
        phaseTrig = 0;
    oldOut = sample;

    static float buf1, buf2, buf3, buf4, fb, in, cut, q;
    oldOut = sample;
    cut = float(cutoff) / 4096;
    q = float(resonance) / 2048;
    in = float(sample - 2048) / 2048;
    fb = q + q / (1.0 - cut);
    buf1 = buf1 + cut * (in - buf1 + fb * (buf1 - buf2));
    if (buf1 > 1)
        buf1 = 1;
    else if (buf1 < -1)
        buf1 = -1;
    buf2 = buf2 + cut * (buf1 - buf2);
    filterOut = buf2 * 2048 + 2048;
    if (buf2 > 1)
        filterOut = 4095;
    else if (buf2 < -1)
        filterOut = 0;
    return (filterOut);
}

// 2-pole resonant low pass filter (BASS)
uint16_t module_vcf::r2pole2(uint16_t sample, uint16_t cutoff, uint16_t resonance) {
    // F = LPF (cutoff)
    // Q = RESONANCE
    // q = SCALED_RESONANCE
    // buf2 => output
    // f = (1 - F) / 2 + Q_offset;
    // q = Q - f = Q - (1 - F) / 2 + Q_offset;

    static int16_t oldOut;
    if (sample > 2048 && oldOut < 2048)
        phaseTrig = 1;
    else
        phaseTrig = 0;
    oldOut = sample;
    static float buf1, buf2, buf3, buf4, fb, in, cut, q;

    cut = float(cutoff) / 4096;
    q = float(resonance) / 4096;
    in = float(sample - 2048) / 2048;

    fb = q - (1 - cut) / 2 + 0;
    buf1 += cut * (in - buf1 + fb * 4 * (buf1 - buf2));
    buf2 += cut * (buf1 - buf2);

    filterOut = buf2 * 2048 + 2048;
    if (buf2 > 1)
        filterOut = 4095;
    else if (buf2 < -1)
        filterOut = 0;
    return (filterOut);
}

// test for filter development
uint16_t module_vcf::test(uint16_t sample, uint16_t cutoff, uint16_t resonance) {
    static int16_t oldOut;
    if (sample > 2048 && oldOut < 2048)
        phaseTrig = 1;
    else
        phaseTrig = 0;
    oldOut = sample;
    static float buf1, buf2, buf3, buf4, fb, in, cut, q;

    cut = float(cutoff) / 4096;
    q = float(resonance) / 4096;
    in = float(sample - 2048) / 2048;

    filterOut = in * 2048 + 2048;
    if (buf2 > 1)
        filterOut = 4095;
    else if (buf2 < -1)
        filterOut = 0;
    return (filterOut);
}

/*
// MOOG 4pole 24 db resonant low pass (Stilson/Smith & Timo Tossavainen)
// ***doesnt work*** - takes too long to calculate at 44kHz ;(
uint16_t module_vcf::moog24db(uint16_t sample, uint16_t cutoff, uint16_t resonance) {

    static int16_t oldOut;
    if (sample > 2048 && oldOut < 2048)
        phaseTrig = 1;
    else
        phaseTrig = 0;
    oldOut = sample;

    // Tdouble MoogVCF::run(double input, double fc, double res)
    static float in1, in2, in3, in4, buf1, buf2, buf3, buf4, fb, in, cut, q;

    in = float(sample - 2048) / 2048;
    cut = float(cutoff) / 3531; // = cutoff*1.16
    q = float(resonance) / 1024;

    fb = q * (1.0 - 0.15 * cut * cut);
    in -= buf4 * fb;
    in *= 0.35013 * (cut * cut) * (cut * cut);
    buf1 = in + 0.3 * in1 + (1 - cut) * buf1; // Pole 1
    in1 = in;
    buf2 = buf1 + 0.3 * in2 + (1 - cut) * buf2; // Pole 2
    in2 = buf1;
    buf3 = buf2 + 0.3 * in3 + (1 - cut) * buf3; // Pole 3
    in3 = buf2;
    buf4 = buf3 + 0.3 * in4 + (1 - cut) * buf4; // Pole 4
    in4 = buf3;
    filterOut = buf4 * 2048 + 2048;
    if (buf4 > 1) filterOut = 4095;
    else if (buf4 < -1) filterOut = 0;
    return (filterOut);
}
*/

/*
// integer implementation of 2pole filter, without feedback division. fast, but not so good...
uint16_t module_vcf::r2pole3(uint16_t sample, uint16_t cutoff, uint16_t resonance) {
    static int16_t filterValue;
    static int16_t filterOut;
    static int16_t out;
    static int16_t oldOut;

    if (sample > 2048 && oldOut < 2048)
        phaseTrig = 1;
    else
        phaseTrig = 0;
    oldOut = sample;

    resonance = resonance - cutoff;
    filterValue += cutoff * (((sample - 2048) >> 4) - (filterValue >> 8) + ((resonance * (filterValue - filterOut)) >> 24)) >> 6; //
    filterOut += cutoff * (filterValue - filterOut) >> 14;

    // filterValue += cutoff * (sample - 2048 - filterValue + (resonance * (filterValue - filterOut) >> 14) ) >> 15; //
    // filterOut += cutoff * (filterValue - filterOut) >> 13;
    out = (filterOut >> 4) + 2048;
    out = (out > 4095 ? 4095 : out);
    out = (out < 0 ? 0 : out);
    return (out);
}
*/

/*
// Running average analog filter ***works bad, too steep (exponential by nature)***:
static uint16_t anHistory[4096]; // old readings from the analog input. Uses 8192 bytes of RAM...
static uint16_t anPos = 0;       // index of the current reading
static uint16_t oldLength;       // previous filterlength
static uint32_t anTotal = 0;     // running total
uint16_t anAvg = 0;              // analog average

if (newLength > oldLength)                                // has filterlength increased ?
{                                                         //
    for (uint16_t i = oldLength + 1; i <= newLength; i++) // ..yes, update anHistory and anTotal
    {
        anHistory[i] = anHistory[oldLength];
        anTotal += anHistory[oldLength];
    }
}
else if (newLength < oldLength)                      // has filterlength decreased ?
{                                                    //
    for (uint16_t i = oldLength; i > newLength; i--) // ..yes, update anTotal
    {
        anTotal -= anHistory[i];
    }
}
if (anPos >= newLength)      // Are we're at the end of the array ?
    anPos = 0;               // ...yes, wrap around to the beginning
anTotal -= anHistory[anPos]; // subtract old reading to be replaced
anHistory[anPos] = sample;   // save new sampled value
anTotal += sample;           // add sample to anTotal
anPos++;                     // advance to the next position in the array:
anAvg = anTotal / newLength; // calculate the average:
oldLength = newLength;       //
return (anAvg);              // return average value
*/