//-----10|-------20|-------30|-------40|-------50|-------60|-------70|-------80|
#include "module_adsr.h"
//#include <stdint.h>
#include <stdio.h>
#include "drivers\display.h"
// compressed image generated with python script (used by init)
const uint8_t img_env[196] = {255, 255, 255, 255, 255, 255, 255, 255, 126, 65, 15, 59, 15, 4, 48, 10, 82, 0, 12, 1, 59, 0, 12, 1, 52, 0, 94, 0, 10, 0, 62, 0, 10, 0, 151, 0, 8, 0, 64, 0, 8, 0, 66, 0, 85, 0, 6, 0, 66, 0, 6, 0, 55, 0, 255, 83, 0, 4, 0, 68, 0, 4, 0, 67, 0, 226, 0, 255, 234, 0, 226, 0, 41, 59, 15, 48, 255, 55, 0, 10, 0, 255, 255, 195, 0, 10, 0, 31, 59, 255, 255, 103, 0, 10, 0, 255, 255, 195, 0, 10, 0, 255, 255, 195, 0, 10, 0, 255, 255, 4, 59, 130, 0, 10, 0, 255, 222, 0, 226, 0, 255, 33, 59, 91, 37, 10, 0, 255, 255, 207, 0, 103, 0, 74, 0, 156, 0, 74, 0, 62, 0, 105, 0, 74, 0, 154, 0, 74, 0, 62, 0, 97, 0, 8, 0, 64, 0, 8, 0, 52, 0, 97, 0, 10, 1, 61, 0, 10, 0, 50, 0, 96, 1, 13, 1, 56, 2, 12, 1, 46, 1, 32, 64, 17, 3, 48, 3, 17, 46, 255, 255, 255, 255, 255, 255, 177};

#define black RGBToWord(0x00, 0x00, 0x00)
#define aColor RGBToWord(0xff, 0x00, 0xff)
#define dColor RGBToWord(0xff, 0xff, 0x00)
#define sColor RGBToWord(0xaa, 0xaa, 0xaa)
#define rColor RGBToWord(0x00, 0x55, 0xff)
#define logoColor RGBToWord(0x00, 0xff, 0xff)

// Generate envelope (ADSR / Attack Decay Sustain Release) waveform.
// All transitions are linear, but could be exponential by altering functions below.
// Operates between 0V to +5V (2048 - 3008)
// Sustain level is fixed to 50% (2.5V or 2528) since there are only three potentiometers avaliable
uint16_t module_adsr::int_adsr(uint16_t gate, uint16_t pot_a, uint16_t pot_d, uint16_t pot_r, uint8_t noSustain)
{
    uint16_t out = 2048; // out level (2048 = 0V)

    if (gate > 2528 && gateCount < 8)
        gateCount++;

    if (gateCount == 8 && oldGate == 0) // confirmed rising edge (new envelope)
    {
        adsrMode = 1;
        tCount = 0;
        oldGate = 1;
        xorGate ^= 1;
    }

    if (gate < 2528 && gateCount > -8)
        gateCount--;

    if (gateCount == -8 && oldGate == 1) // confirmed falling edge
        oldGate = 0;

    tCount++;

    if (adsrMode == 1) // Attack
    {
        if (tCount < pot_a)
            out = tCount * 960 / pot_a + 2048;
        else
        {
            adsrMode = 2;
            tCount = 0;
        }
    }
    if (adsrMode == 2) // Decay
    {
        if (tCount < pot_d)
            out = 3008 - tCount * 480 / pot_d; // 3008 - 480 = 2528 => 50% sustain level (2.5V)
        else
        {
            adsrMode = 3;
            tCount = 0;
            if (noSustain == 1)
                adsrMode = 4; // skip sustain (for percussion)
        }
    }
    if (adsrMode == 3) // Sustain
    {
        if (gateCount > 4 && oldGate == 1)
            out = 2528; // 50% sustain level (2.5V)
        else
        {
            adsrMode = 4;
            tCount = 0;
        }
    }
    if (adsrMode == 4) // Release
    {
        if (tCount < pot_r)
            out = 2528 - tCount * 480 / pot_r;
        else
        {
            adsrMode = 0;
            tCount = 0;
            out = 2048; // 0V
        }
    }
    return (out);
}

void module_adsr::draw(display &disp, uint16_t gate, uint16_t pot_a, uint16_t pot_d, uint16_t pot_r, uint8_t noSustain)
{

    uint16_t yOffset = 142;
    uint16_t height = 64;
    uint16_t sh = height >> 1;
    uint8_t radius = 4;
    uint8_t scale = 5; // scale for potentiometer value to pixels division = 2^scale
    static int16_t xOffset, a, d, s, r;
    if (xOffset <= 0)
        xOffset = 1;
    if (gate > 2682) // show trig 3v3 trig level
    {
        if (xorGate == 0)
            disp.fillRectangle(220, 80, 10, 10, RGBToWord(0xff, 0xff, 0x00));
        else
            disp.fillRectangle(220, 80, 10, 10, RGBToWord(0x00, 0xff, 0x00));
        disp.putPixel(220, 80, black);
        disp.putPixel(220, 89, black);
        disp.putPixel(229, 80, black);
        disp.putPixel(229, 89, black);
    }

    if (pot_a >> scale != a || pot_d >> scale != d || pot_r >> scale != r)
    {
        // delete previous envelope (lines + circles)
        disp.drawLine(xOffset, yOffset, xOffset + a, yOffset - height, black); // A
        disp.fillCircle(xOffset + a, yOffset - height, radius, black);
        if (a + d < 238 - xOffset)
        {
            disp.drawLine(xOffset + a, yOffset - height, xOffset + a + d, yOffset - sh, black); // D
            disp.fillCircle(xOffset + a + d, yOffset - sh, radius, black);
        }
        if (a + d + s < 238 - xOffset)
            disp.drawLine(xOffset + a + d, yOffset - sh, xOffset + a + d + s, yOffset - sh, black); // S
        if (a + d + s + r < 238 - xOffset)
        {
            disp.drawLine(xOffset + a + d + s, yOffset - sh, xOffset + a + d + s + r, yOffset, black); // R
            disp.fillCircle(xOffset + a + d + s + r, yOffset, radius, black);
        }

        a = pot_a >> scale;
        d = pot_d >> scale;
        if (noSustain == 1)
            s = 0;
        else
            s = height >> 1;
        r = pot_r >> scale;
        xOffset = (240 - a - d - s - r) >> 1;
        if (xOffset <= 0)
            xOffset = 1;

        // draw new envelope
        disp.drawLine(xOffset, yOffset, xOffset + a, yOffset - height, aColor); // A
        if (a + d < 238 - xOffset)
            disp.drawLine(xOffset + a, yOffset - height, xOffset + a + d, yOffset - sh, dColor); // D
        if (a + d + s < 238 - xOffset)
            disp.drawLine(xOffset + a + d, yOffset - sh, xOffset + a + d + s, yOffset - sh, sColor); // S
        if (a + d + s + r < 238 - xOffset)
            disp.drawLine(xOffset + a + d + s, yOffset - sh, xOffset + a + d + s + r, yOffset, rColor); // R

        disp.fillCircle(xOffset + a, yOffset - height, radius - 1, black);
        disp.fillCircle(xOffset + a + d + s + r, yOffset, radius - 1, black);
        if (a + d < 238 - xOffset)
        {
            disp.fillCircle(xOffset + a + d, yOffset - sh, radius - 1, black);
            disp.drawCircle(xOffset + a + d, yOffset - sh, radius, dColor);
        }
        if (a + d + s + r < 238 - xOffset)
        {
            disp.drawCircle(xOffset + a, yOffset - height, radius, aColor);
            disp.drawCircle(xOffset + a + d + s + r, yOffset, radius, rColor);
        }

        // show pot values
        char str[11];
        sprintf(str, "A: %d", pot_a);
        disp.fillRectangle(130, 160, 70, 16, black);
        disp.putStr(str, 130 - 9, 160, aColor, black);
        sprintf(str, "D: %d", pot_d);
        disp.fillRectangle(130, 180, 70, 16, black);
        disp.putStr(str, 130 - 9, 180, dColor, black);
        sprintf(str, "S: 50%%");
        disp.fillRectangle(130, 200, 70, 16, black);
        disp.putStr(str, 130 - 9, 200, sColor, black);
        sprintf(str, "R: %d", pot_r);
        disp.fillRectangle(130, 220, 70, 16, black);
        disp.putStr(str, 130 - 9, 220, rColor, black);
    }

    if (gate < 2682) // 3V3 trig level
        disp.fillRectangle(220, 80, 10, 10, black);
}

void module_adsr::init(display &disp)
{
    // show logo
    disp.showImg(img_env, sizeof(img_env), 0, 0, 240, 60, logoColor);
    // clear background
    disp.fillRectangle(0, 60, 240, 240 - 60, black);
}

//-----10|-------20|-------30|-------40|-------50|-------60|-------70|-------80|