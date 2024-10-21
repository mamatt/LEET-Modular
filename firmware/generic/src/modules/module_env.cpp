
#include "module_env.h"
#include <stdio.h>
#include "drivers/display.h"
#include "res/colors.h"

// Generate envelope (ADSR / Attack Decay Sustain Release) waveform.
// All transitions are linear, but could be exponential by altering functions below.
// Operates between 0V to +5V (2048 - 3008)
// Sustain level is fixed to 50% (2.5V or 2528) since there are only three potentiometers avaliable

uint16_t module_env::int_adsr(uint16_t gate, uint16_t pot_a, uint16_t pot_d, uint16_t pot_r, uint8_t noSustain)
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

void module_env::draw(display &disp, uint16_t gate, uint16_t pot_a, uint16_t pot_d, uint16_t pot_r, uint8_t noSustain)
{

    uint16_t yOffset = 160;
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
        disp.putPixel(220, 80, blackColor);
        disp.putPixel(220, 89, blackColor);
        disp.putPixel(229, 80, blackColor);
        disp.putPixel(229, 89, blackColor);
    }

    if (pot_a >> scale != a || pot_d >> scale != d || pot_r >> scale != r)
    {
        // delete previous envelope (lines + circles)
        disp.drawLine(xOffset, yOffset, xOffset + a, yOffset - height, blackColor); // A
        disp.fillCircle(xOffset + a, yOffset - height, radius, blackColor);
        if (a + d < 238 - xOffset)
        {
            disp.drawLine(xOffset + a, yOffset - height, xOffset + a + d, yOffset - sh, blackColor); // D
            disp.fillCircle(xOffset + a + d, yOffset - sh, radius, blackColor);
        }
        if (a + d + s < 238 - xOffset)
            disp.drawLine(xOffset + a + d, yOffset - sh, xOffset + a + d + s, yOffset - sh, blackColor); // S
        if (a + d + s + r < 238 - xOffset)
        {
            disp.drawLine(xOffset + a + d + s, yOffset - sh, xOffset + a + d + s + r, yOffset, blackColor); // R
            disp.fillCircle(xOffset + a + d + s + r, yOffset, radius, blackColor);
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

        disp.fillCircle(xOffset + a, yOffset - height, radius - 1, blackColor);
        disp.fillCircle(xOffset + a + d + s + r, yOffset, radius - 1, blackColor);
        if (a + d < 238 - xOffset)
        {
            disp.fillCircle(xOffset + a + d, yOffset - sh, radius - 1, blackColor);
            disp.drawCircle(xOffset + a + d, yOffset - sh, radius, dColor);
        }
        if (a + d + s + r < 238 - xOffset)
        {
            disp.drawCircle(xOffset + a, yOffset - height, radius, aColor);
            disp.drawCircle(xOffset + a + d + s + r, yOffset, radius, rColor);
        }

        // show pot values
        /* char str[11];
        sprintf(str, "A: %d", pot_a);
        disp.fillRectangle(130, 160, 70, 16, blackColor);
        disp.putStr(str, 130 - 9, 160, aColor, blackColor);
        sprintf(str, "D: %d", pot_d);
        disp.fillRectangle(130, 180, 70, 16, blackColor);
        disp.putStr(str, 130 - 9, 180, dColor, blackColor);
        sprintf(str, "S: 50%%");
        disp.fillRectangle(130, 200, 70, 16, blackColor);
        disp.putStr(str, 130 - 9, 200, sColor, blackColor);
        sprintf(str, "R: %d", pot_r);
        disp.fillRectangle(130, 220, 70, 16, blackColor);
        disp.putStr(str, 130 - 9, 220, rColor, blackColor); */
    }

    if (gate < 2682) // 3V3 trig level
        disp.fillRectangle(220, 80, 10, 10, blackColor);
}
