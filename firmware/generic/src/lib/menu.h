#ifndef __menu_h
#define __menu_h
extern "C"
{

    #include "drivers/display.h"
}

#define mainSelect 0
#define subSelect 1

class menu {
    public:
        uint8_t selectMode = subSelect ;
        uint8_t mainMode ; 
        uint8_t subMode  ;
        uint8_t subMenuMax ;
        uint8_t needUpdate = 1;
        uint16_t subModeColor ;
        const char ** subMenuTxt ;
        const char ** potsName ;
        const uint8_t * logoImage ;
        uint16_t imgSize = 0  ;

        void switchMode() ;
        void prev() ;
        void next() ;

        void drawSkel(display &Display) ;
        void drawPotsName(display &Display) ;
        void drawSubmode(display &Display) ;
};
#endif