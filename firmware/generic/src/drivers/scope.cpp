#include "scope.h"
#include "drivers\display.h"
scope::scope(uint16_t Colour)
{
    this->Colour = Colour;
    offset = 76; // add space above scope
    scale = 1;
    BufferInputIndex = 0;
    for (uint32_t i = 0; i < SCOPE_BUFFER_SIZE; i++)
    {
        Buffer[i] = 0;
    }
}

void scope::draw(display &dsp)
{
    uint16_t YLocn;
    for (uint32_t i = 0; i < SCOPE_BUFFER_SIZE; i++)
    {
        YLocn = /*dsp.getHeight()/2 +*/ offset + (Buffer[i] * scale);
        dsp.putPixel(i, YLocn, Colour);
    }
}

void scope::draw1(display &dsp, uint16_t pos)
{
    uint16_t YLocn;
    YLocn = /*dsp.getHeight()/2 +*/ offset + (Buffer[pos] * scale);
    dsp.putPixel(pos, YLocn + 1, Colour);
}

uint16_t scope::read(uint16_t pos)
{
    return (Buffer[pos]);
}

void scope::write(uint16_t pos, uint16_t value)
{
    Buffer[pos] = value;
}

void scope::insert(uint16_t value)
{
    Buffer[BufferInputIndex] = value;
    BufferInputIndex = (BufferInputIndex + 1) % SCOPE_BUFFER_SIZE;
}
