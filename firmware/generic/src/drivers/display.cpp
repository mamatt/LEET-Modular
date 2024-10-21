#include <stdint.h>
#include "display.h"
#include "spi.h"
#include "SysTick.h"
#include "res/font.h"
extern "C"
{
    #include "gd32vf103.h"

    //extern "C" const uint8_t font[1520];
}
extern SysTick stk;
/*
    The LCD display on the eurorack module is an ST7789 with a resolution
    of 240 x 240
    It's an SPI device connected as follows:
    SCK   PB13   (SCK1)
    MOSI  PB15   (MOSI1)
    RST   PA8    (reset)
    DC    PA11   (aka RS)
    BLK   PB10   (backlight)

*/

/*
    The LCD display on the sipeed longan nano seems to be an ST7735 with a resolution
    of 160 x 80
    It's an SPI device connected as follows:
    SCK   PA5   (SCK0)
    MOSI  PA7   (MOSI0)
    RS    PB0
    RST   PB1
    CS    PB2
*/
void display::begin(spi *SPI)
{

    this->SPI = SPI; // remember the SPI interface
    XYSwapped = 0;   // XY Not swapped to begin with

    // Turn on Port A
    rcu_periph_clock_enable(RCU_GPIOA);
    // Turn on Port B
    rcu_periph_clock_enable(RCU_GPIOB);

    //  Configure the various control pins
    gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_8 | GPIO_PIN_11);
    gpio_init(GPIOB, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_10);
    gpio_init(GPIOB, GPIO_MODE_AF_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13 | GPIO_PIN_15);
    SPI->begin();

    // Initialization sequence from Longan sample code
    RSTHigh();      // Drive reset high
    stk.delay(25);  // wait
    RSTLow();       // Drive reset low
    stk.delay(250); // wait
    RSTHigh();      // Drive reset high
    stk.delay(25);  // wait

    writeCommand(0x01); // sw reset
    stk.delay(150);     // wait

    writeCommand(0x11); // turn off sleep mode
    stk.delay(100);

    writeCommand(0x3A); // set color mode
    writeData8(0x55);   // 16 bit color
    stk.delay(10);

    writeCommand(0x36); // Color order RGB:0x00 BGR:0x08
    writeData8(0x00);   //

    // power settings
    writeCommand(0xb2); // porch control
    writeData8(0x0c);
    writeData8(0x0c);
    writeData8(0x00);
    writeData8(0x33);
    writeData8(0x33);
    writeCommand(0xb7); // gate control
    writeData8(0x35);
    writeCommand(0xbb); // vcoms
    writeData8(0x28);
    writeCommand(0xc0); // LCM control
    writeData8(0x0c);
    writeCommand(0xc2); // VDV and VRH command enable
    writeData8(0x01);
    writeData8(0xff);
    writeCommand(0xc3); // voltage VRHS set
    writeData8(0x10);
    writeCommand(0xc4); // VDV set
    writeData8(0x20);
    writeCommand(0xc6); // FR control 2
    writeData8(0x0f);
    writeCommand(0xd0); // power control 1
    writeData8(0xa4);
    writeData8(0xa1);

    writeCommand(0xe0); // positive gamma control
    writeData8(0xd0);
    writeData8(0x00);
    writeData8(0x02);
    writeData8(0x07);
    writeData8(0x0a);
    writeData8(0x28);
    writeData8(0x32);
    writeData8(0x44);
    writeData8(0x42);
    writeData8(0x06);
    writeData8(0x0e);
    writeData8(0x12);
    writeData8(0x14);
    writeData8(0x17);

    writeCommand(0xe1); // negative gamma control
    writeData8(0xd0);
    writeData8(0x00);
    writeData8(0x02);
    writeData8(0x07);
    writeData8(0x0a);
    writeData8(0x28);
    writeData8(0x31);
    writeData8(0x54);
    writeData8(0x47);
    writeData8(0x0e);
    writeData8(0x1c);
    writeData8(0x17);
    writeData8(0x1b);
    writeData8(0x1e);

    writeCommand(0x2A); // column address set
    writeData8(0x00);   //
    writeData8(0x00);   // x start
    writeData8(0x00);   //
    writeData8(0xef);   // x end (240)

    writeCommand(0x2B); // row address set
    writeData8(0x00);   //
    writeData8(0x00);   // y start
    writeData8(0x00);   // y end upper
    writeData8(0xef);   // y end lower (240)

    writeCommand(0x21); // invon
    stk.delay(10);
    writeCommand(0x13); // normal display on
    stk.delay(10);
    writeCommand(0x29); // main screen turn on
    stk.delay(10);

    BLKHigh();                                                // turn on backlight
    fillRectangle(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT, 0x0000); // black out the screen
}

void display::showImg(const uint8_t *img, uint16_t size, uint8_t x, uint8_t y, uint16_t width, uint16_t height, uint16_t color)
{
    uint8_t preRow[width];
    uint8_t pixel = 0;
    uint8_t invert = 0;
    uint8_t pixelCount;
    uint16_t rgb = 0;
    for (uint8_t i = 0; i < width; i++)
        preRow[i] = 0;
    openAperture(x, y, x + width, y + height);
    for (uint16_t pImg = 0; pImg < size; pImg++)
    {
        pixelCount = *(img + pImg);
        if (pixelCount < 255)
        {
            pixelCount++;
            invert = 1;
        }
        while (pixelCount > 0)
        {
            preRow[x] ^= pixel;
            rgb = RGBToWord(0x0, 0x0, 0x0);
            if (preRow[x] != 0)
            {
                rgb = color;
            }
            writeData16(rgb);
            pixelCount--;
            x++;
            if (x == width)
                x = 0;
        }
        if (invert == 1)
        {
            pixel ^= 1;
            invert = 0;
        }
    }
}

void display::putChar(char ch, uint8_t x, uint8_t y, uint16_t txtFg, uint16_t txtBg)
{
    if ((' ' <= ch) && (ch <= '~')) // 7bit ASCII
    {
        openAperture(x, y, x + 8 - 1, y + 16 - 1);
        ch -= 32; // why this offset ... 
        const uint8_t *charFont = &font[(uint8_t)ch * 16];
        
        for (uint8_t y = 0; y < 16; ++y) {
            for (uint8_t x = 0, c = charFont[y]; x < 8; ++x, c >>= 1) {
                uint16_t rgb = (c & 0x01) ? txtFg : txtBg;
                writeData16(rgb);
            }
        }
    }
}

void display::putStr(const char *str, uint8_t x, uint8_t y, uint16_t txtFg, uint16_t txtBg)
{
    while (*str)
        putChar(*(str++), x += 9, y, txtFg, txtBg);
}

void display::putPixel(uint16_t x, uint16_t y, uint16_t colour)
{
    openAperture(x, y, x + 1, y + 1);
    writeData16(colour);
}

void display::putImage(uint16_t x, uint16_t y, uint16_t width, uint16_t height, const uint16_t *Image)
{
    uint16_t Colour;
    openAperture(x, y, x + width - 1, y + height - 1);
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            Colour = *(Image++);
            writeData16(Colour);
        }
    }
}

void display::drawRectangle(uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t Colour)
{
    drawLine(x, y, x + w, y, Colour);
    drawLine(x, y, x, y + h, Colour);
    drawLine(x + w, y, x + w, y + h, Colour);
    drawLine(x, y + h, x + w, y + h, Colour);
}

void display::fillRectangle(uint16_t x, uint16_t y, uint16_t width, uint16_t height, uint16_t Colour)
{
    openAperture(x, y, x + width - 1, y + height - 1);
    for (y = 0; y < height; y++)
    {
        for (x = 0; x < width; x++)
        {
            writeData16(Colour);
        }
    }
}

void display::drawCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour)
{
    // Reference : https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);
    if (radius > x0)
        return; // don't draw even parially off-screen circles
    if (radius > y0)
        return; // don't draw even parially off-screen circles

    if ((x0 + radius) > SCREEN_WIDTH)
        return; // don't draw even parially off-screen circles
    if ((y0 + radius) > SCREEN_HEIGHT)
        return; // don't draw even parially off-screen circles
    while (x >= y)
    {
        putPixel(x0 + x, y0 + y, Colour);
        putPixel(x0 + y, y0 + x, Colour);
        putPixel(x0 - y, y0 + x, Colour);
        putPixel(x0 - x, y0 + y, Colour);
        putPixel(x0 - x, y0 - y, Colour);
        putPixel(x0 - y, y0 - x, Colour);
        putPixel(x0 + y, y0 - x, Colour);
        putPixel(x0 + x, y0 - y, Colour);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }

        if (err > 0)
        {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

void display::fillCircle(uint16_t x0, uint16_t y0, uint16_t radius, uint16_t Colour)
{
    // Reference : https://en.wikipedia.org/wiki/Midpoint_circle_algorithm
    // Similar to drawCircle but fills the circle with lines instead
    int x = radius - 1;
    int y = 0;
    int dx = 1;
    int dy = 1;
    int err = dx - (radius << 1);

    if (radius > x0)
        return; // don't draw even parially off-screen circles
    if (radius > y0)
        return; // don't draw even parially off-screen circles

    if ((x0 + radius) > SCREEN_WIDTH)
        return; // don't draw even parially off-screen circles
    if ((y0 + radius) > SCREEN_HEIGHT)
        return; // don't draw even parially off-screen circles
    while (x >= y)
    {
        drawLine(x0 - x, y0 + y, x0 + x, y0 + y, Colour);
        drawLine(x0 - y, y0 + x, x0 + y, y0 + x, Colour);
        drawLine(x0 - x, y0 - y, x0 + x, y0 - y, Colour);
        drawLine(x0 - y, y0 - x, x0 + y, y0 - x, Colour);

        if (err <= 0)
        {
            y++;
            err += dy;
            dy += 2;
        }

        if (err > 0)
        {
            x--;
            dx += 2;
            err += dx - (radius << 1);
        }
    }
}

void display::drawLineLowSlope(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t Colour)
{
    // Reference : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    int dx = x1 - x0;
    int dy = y1 - y0;
    int yi = 1;
    if (dy < 0)
    {
        yi = -1;
        dy = -dy;
    }
    int D = 2 * dy - dx;

    int y = y0;

    for (int x = x0; x <= x1; x++)
    {
        putPixel(x, y, Colour);
        if (D > 0)
        {
            y = y + yi;
            D = D - 2 * dx;
        }
        D = D + 2 * dy;
    }
}

void display::drawLineHighSlope(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t Colour)
{
    // Reference : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm

    int dx = x1 - x0;
    int dy = y1 - y0;
    int xi = 1;
    if (dx < 0)
    {
        xi = -1;
        dx = -dx;
    }
    int D = 2 * dx - dy;
    int x = x0;

    for (int y = y0; y <= y1; y++)
    {
        putPixel(x, y, Colour);
        if (D > 0)
        {
            x = x + xi;
            D = D - 2 * dy;
        }
        D = D + 2 * dx;
    }
}

void display::drawLine(uint16_t x0, uint16_t y0, uint16_t x1, uint16_t y1, uint16_t Colour)
{
    // Reference : https://en.wikipedia.org/wiki/Bresenham%27s_line_algorithm
    if (iabs(y1 - y0) < iabs(x1 - x0))
    {
        if (x0 > x1)
        {
            drawLineLowSlope(x1, y1, x0, y0, Colour);
        }
        else
        {
            drawLineLowSlope(x0, y0, x1, y1, Colour);
        }
    }
    else
    {
        if (y0 > y1)
        {
            drawLineHighSlope(x1, y1, x0, y0, Colour);
        }
        else
        {
            drawLineHighSlope(x0, y0, x1, y1, Colour);
        }
    }
}

void display::openAperture(uint16_t x1, uint16_t y1, uint16_t x2, uint16_t y2)
{
    // open up an area for drawing on the display
    /*
    // For some strange reason, the LCD on the Sipeed Longan Nano has a 26 pixel
    // offset on the x dimension and 1 pixel on the y-dimension
    if (!XYSwapped)
    {
        x1 += 26;
        x2 += 26;
        y1++;
        y2++;
    }
    else
    {
        y1 += 26;
        y2 += 26;
        x1++;
        x2++;
    }
    */
    writeCommand(0x2A); // Set X limits
    RSHigh();
    CSLow();
    SPI->writeData8(x1 >> 8);
    SPI->writeData8(x1 & 0xff);
    SPI->writeData8(x2 >> 8);
    SPI->writeData8(x2 & 0xff);
    CSHigh();
    writeCommand(0x2B); // Set Y limits
    RSHigh();
    CSLow();
    SPI->writeData8(y1 >> 8);
    SPI->writeData8(y1 & 0xff);
    SPI->writeData8(y2 >> 8);
    SPI->writeData8(y2 & 0xff);
    CSHigh();
    writeCommand(0x2c); // put display in to data write mode
}

void display::RSLow()
{ // drive D/C pin low
    gpio_bit_reset(GPIOA, GPIO_PIN_11);
}

void display::RSHigh()
{ // drive D/C pin high
    gpio_bit_set(GPIOA, GPIO_PIN_11);
}

void display::RSTLow()
{ // Drive reset low
    gpio_bit_reset(GPIOA, GPIO_PIN_8);
}

void display::RSTHigh()
{ // Drive reset high
    gpio_bit_set(GPIOA, GPIO_PIN_8);
}

void display::CSLow()
{ // Drive chip select low
    //    gpio_bit_reset(GPIOB,GPIO_PIN_2);
}

void display::CSHigh()
{ // Drive chip select high
    //    gpio_bit_set(GPIOB,GPIO_PIN_2);
}

void display::BLKLow()
{ // Drive backlight low (off)
    gpio_bit_reset(GPIOB, GPIO_PIN_10);
}

void display::BLKHigh()
{ // Drive backlight high (on)
    gpio_bit_set(GPIOB, GPIO_PIN_10);
}

void display::writeCommand(uint8_t Cmd)
{
    RSLow();
    CSLow();
    SPI->writeData8(Cmd);
    CSHigh();
}

void display::writeData8(uint8_t Data)
{
    RSHigh();
    CSLow();
    SPI->writeData8(Data);
    CSHigh();
}

void display::writeData16(uint16_t Data)
{
    RSHigh();
    CSLow();
    SPI->writeData16(Data);
    CSHigh();
}

void display::exchangeXY()
{
    writeCommand(0x36);
    if (!XYSwapped)
    {
        XYSwapped = 1;
        writeData8(0x20);
    }
    else
    {
        XYSwapped = 0;
        writeData8(0x00);
    }
}

uint32_t display::getWidth()
{
    if (!XYSwapped)
    {
        return SCREEN_WIDTH;
    }
    else
    {
        return SCREEN_HEIGHT;
    }
}

uint32_t display::getHeight()
{
    if (!XYSwapped)
    {
        return SCREEN_HEIGHT;
    }
    else
    {
        return SCREEN_WIDTH;
    }
}

void display::showSprite(const uint8_t *rle_data, uint16_t size) {
    const uint8_t *__ip, *__il, *__rd;
    __ip = (rle_data);
    __il = __ip + (size)*2;
    __rd = (rle_data);
    { /* RGB16 */
        while (__ip < __il)
        {
            uint16_t __l = *(__rd++);
            if (__l & 128)
            {
                __l = __l - 128;
                do
                {
                    // memcpy (__ip, __rd, 2);
                    writeData8(*__rd);
                    writeData8(*(__rd + 1));
                    __ip += 2;
                } while (--__l);
                __rd += 2;
            }
            else
            {
                __l *= 2;
                //memcpy (__ip, __rd, __l);
                for (int i = 0; i < __l; i++)
                {
                    writeData8(*(__rd + i));
                }
                __ip += __l;
                __rd += __l;
            }
        }
    }
}
