
#ifndef __module_vco_h
#define __module_vco_h

#include "gd32vf103.h"

// sub modes
#define mode_sine_am 0
#define mode_sine_add 1
#define mode_sine_fm 2
#define mode_square_am 3
#define mode_square_add 4
#define mode_square_fm 5
#define mode_square_pw 6
#define mode_square_2x 7
#define mode_square_3x 8
#define mode_triangle_am 9
#define mode_triangle_add 10
#define mode_triangle_fm 11
#define mode_triangle_saw 12
#define mode_triangle_2x 13
#define mode_triangle_3x 14

#define vcoColor RGBToWord(0x0E, 0x11, 0xC7)  // dark blue
#define vcoSubMenuMax 15 
const uint8_t vcoImage[693] = {255,255,255,255,255,255,255,255,45,13,30,13,30,4,32,4,41,11,20,12,15,2,13,3,23,2,13,2,26,0,4,1,29,0,4,1,51,0,31,0,12,2,20,2,17,2,19,1,23,0,7,0,27,0,7,0,70,0,24,0,26,1,13,1,24,0,21,0,9,0,25,0,9,0,68,0,11,0,10,1,42,0,27,0,19,0,36,0,49,0,52,1,42,1,29,0,23,2,4,0,29,1,4,0,66,0,11,0,8,0,32,0,9,0,32,0,16,0,7,0,33,0,1,0,93,0,43,0,34,0,20,0,8,0,21,0,3,0,3,0,3,0,64,0,11,0,7,0,43,0,15,4,15,0,13,0,9,0,119,0,7,0,17,4,20,0,13,2,4,1,33,0,31,0,3,0,5,0,3,0,62,0,19,0,14,3,4,2,8,0,6,0,13,0,9,0,13,0,24,0,3,0,85,0,12,0,11,0,22,0,11,1,27,1,11,0,25,0,3,0,133,0,12,1,14,1,11,0,11,0,14,0,62,0,3,0,7,0,3,0,34,0,23,0,11,0,6,0,12,0,18,0,9,0,42,0,24,0,3,0,109,0,20,0,20,0,1,0,18,0,16,0,22,0,3,0,111,0,19,0,35,1,5,0,11,0,79,0,3,0,9,0,3,0,69,0,20,0,203,0,19,0,11,0,30,0,11,0,58,0,3,0,106,0,108,0,3,0,110,0,10,0,65,0,80,0,3,0,32,2,15,0,10,0,6,0,20,0,11,0,30,0,73,0,36,0,3,0,13,0,2,0,46,0,133,0,90,0,127,0,36,0,83,0,21,0,43,0,33,0,8,0,1,0,52,0,55,0,10,0,113,1,33,0,23,0,11,0,3,0,135,0,40,0,3,0,87,0,10,0,10,0,43,0,255,30,0,45,0,3,0,15,0,3,0,32,0,13,0,10,0,90,0,42,0,52,0,3,0,28,0,0,0,10,0,140,0,201,0,45,0,3,0,17,0,49,0,10,0,91,0,49,0,36,0,7,0,3,0,70,0,21,1,6,0,11,0,17,0,11,0,32,0,98,0,14,0,33,0,100,0,3,0,24,0,53,0,29,0,17,1,40,0,11,0,39,0,31,0,10,0,3,0,19,0,68,1,25,0,14,0,48,0,9,0,84,0,31,1,11,1,14,0,12,0,12,0,12,0,41,0,8,0,26,0,3,0,65,0,14,2,4,3,7,0,22,0,9,1,12,0,37,0,4,0,1,0,27,0,4,0,1,0,4,0,40,0,37,4,21,0,13,2,3,2,13,0,45,1,4,0,23,0,4,1,46,0,20,0,61,3,15,0,40,0,49,0,108,0,34,0,42,0,9,0,25,0,9,0,40,0,22,0,31,0,11,0,32,0,44,0,7,0,27,0,7,0,66,0,44,0,30,0,46,0,4,1,29,1,4,0,25,0,14,0,25,0,28,0,14,0,27,1,48,4,33,4,41,0,27,1,24,1,16,0,24,1,167,1,19,2,19,1,20,1,124,13,32,2,13,2,24,2,13,3,176,13,30,13,255,255,255,218};

class module_vco
{
    public:
        const char *subMenuText[15] = { "sine am", "sine add", "sine fm","square am", "square add", "square fm", "square pw", "square 2x", "square 3x","triangle am", "triangle add", "triangle fm", "triangle saw", "triangle 2x","triangle 3x"} ;
        const char *potsName[3] = {"Note","Octv","Levl"} ;
 
        uint8_t phaseTrig;
        float phaseC = 0;       // phaseCount | stepsize within one period (0-65535). frequenzy = xStep * 44100 / (2*65536)
        uint8_t quantized = 0;  // quantize CV input of module?

        int16_t processSineAM(uint16_t note, uint16_t amplitude, int amplitudeLevel, int octave, int fine)  ;
        int16_t processSineADD(uint16_t note, uint16_t amplitude, int amplitudeLevel, int octave, int fine)  ;
        int16_t processSineFM(uint16_t note, uint16_t amplitude, int amplitudeLevel, int octave, int fine)  ;
        
        uint16_t square(float xTmp, uint16_t pw);
        uint16_t square2x(float xTmp, uint16_t dist);
        uint16_t square3x(float xTmp, uint16_t dist);
        uint16_t triangle(float xTmp, uint16_t shape);
        uint16_t triangle2x(float xTmp, uint16_t dist);
        uint16_t triangle3x(float xTmp, uint16_t dist);

        int16_t fold(int16_t tmp) ;
        float ad2phase(uint16_t in) ;
        float sine(float xTmp);
        float getPhase(int cv, int octave, int note) ;
    private:
        float phase, phase1, phase2, phase3;
        float oldPhase;
        float sin(float x);
        uint16_t deadzone(uint16_t tmp) ;
};
#endif
