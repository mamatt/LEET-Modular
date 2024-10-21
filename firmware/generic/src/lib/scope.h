#ifndef __SCOPE_H
#define __SCOPE_H
#include <stdint.h>
#include "drivers/display.h"
// Drawing of scope data:
/* Two basic approaches:
    1:  Don't remember any scope data and simply overwrite the
        screen each time you want to draw.
    2:  Remember the scope data and "undraw" before draw.
        This also allows dot-joining
Third possible approach is to have a screen buffer and do DMA
A bit concerned that this might cause interrupt jitter though.
Whatever the mechanism it would be nice to avoid too much flicker
on the display.
*/
// In order to avoid dynamic memory allocation for scope data
// a fixed size array has to be declared here.
#if SCREEN_WIDTH > SCREEN_HEIGHT
#define SCOPE_BUFFER_SIZE SCREEN_WIDTH
#else
#define SCOPE_BUFFER_SIZE SCREEN_HEIGHT
#endif

class scope
{
public:
    scope(uint16_t Colour);
    void draw(display &dsp);
    void draw1(display &dsp, uint16_t pos);
    uint16_t read(uint16_t pos);
    void write(uint16_t pos, uint16_t value);
    void insert(uint16_t value);

private:
    uint32_t offset;
    uint32_t scale;
    uint16_t Buffer[SCOPE_BUFFER_SIZE];
    uint32_t BufferInputIndex;
    uint16_t Colour;
};
#endif