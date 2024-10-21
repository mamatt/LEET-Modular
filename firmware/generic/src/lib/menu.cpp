#include <stdio.h>
#include "menu.h"
#include "res/modes.h"
#include "res/colors.h"
#include "res/images.h"
#include "drivers/display.h"

void menu::prev() {
  if (selectMode == mainSelect ) {
    if ( mainMode > 1 ) {
      mainMode --  ;
    }
  } else {
     if ( subMode > 0 ) {
      subMode --  ;
    }
  }
  needUpdate = 1 ;
}

void menu::next() {
    if (selectMode == mainSelect ) {
    if ( mainMode < maxMode ) {
      mainMode ++  ;
    }
  } else {
     if ( subMode <  subMenuMax-1 ) {
      subMode ++  ;
    }
  }
  needUpdate = 1 ;
}

void menu::switchMode() {
  if (selectMode == mainSelect ) {
    selectMode = subSelect ;

  } else {
    selectMode = mainSelect ;
    subMode = 0 ;
  }
  needUpdate = 1 ;
}

void menu::drawSkel(display &Display) {
  // clean all screen
  Display.fillRectangle(0, 0, 240, 240, blackColor );
      
  // show logo
  Display.showImg(logoImage, imgSize, 0, 0, 240, 60, subModeColor);
      
  // show frame
  Display.drawRectangle(0, 86, 239, 79, whiteColor );

}

void menu::drawPotsName(display &Display) {

    char str[16] ; // to handle pot value formating
    // --------------------------- Line 1 --------------------------------
    // POT UR
    sprintf(str, "%s : ",potsName[0]);
    Display.putStr(str, 130, 180, potUpperRightColor, blackColor); 
    
    // --------------------------- Line 3 --------------------------------
    // POT LL
    sprintf(str, "%s : ",potsName[1]);
    Display.putStr(str, 0 , 200, potLowerLeftColor, blackColor);

    // POT LR 
    sprintf(str, "%s : ",potsName[2]);
    Display.putStr(str, 130, 200, potLowerRightColor, blackColor);

}

void menu::drawSubmode(display &Display) {

  char str[17] ; // to handle pot value formating

  Display.fillRectangle(0, 60, 240, 20, blackColor);
  sprintf(str,"%16s",subMenuTxt[subMode]) ;
  Display.putStr(str, 80, 60, subModeColor, blackColor);

      if (selectMode == subSelect ) {
        sprintf(str,"Sub") ;
        Display.putStr(str, 1, 60, main1Color, blackColor);
         //Display.showImg(arrowLeftImage, sizeof(arrowLeftImage), 0, 60, 16, 16, main1Color);
      } else {
        sprintf(str,"Main") ;
        Display.putStr(str, 1, 60, main1Color, blackColor);
      }

  needUpdate = 0 ; // menu have been updated 
}
