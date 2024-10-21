#include "module_vca.h"


int16_t module_vca::process(uint16_t sample, int env, int envAmp, int offset) {
      //int16_t tmp ;
      if (env < 2048 ) env = 2048 ;              // lock to 0V <->5V
      if (env > 3008 ) env = 3008 ;              // lock to 0V <->5V
      //tmp = ((ADC.anRaw[1]-2048) * ADC.anAvg[3])  >> 10  ;         // env (2048 <-> 3008 ; 0V <-> 5V ) * attenuation 0 <-> 4096
      //tmp = (sample-2048) * (env-2048) >> 10
      return (( (sample-2048) * (env-2048) ) >> 10) + 2048  ;   



}


   // cv2 = ((sample * env * envAmp) >> 21); // pot2 (bottom right) sets level for CV2 that attenuates or amplifies amplitude
   // cv2 += (offset >> 1) - 2048; 