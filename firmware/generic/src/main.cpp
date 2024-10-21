// ***********************************************************************************
// LEET modular - a versatile, easy to build, affordable and powerful eurorack module!
// ***********************************************************************************

// todo:
// * integrate midi to cv code
// * implement s&h
// * implement vca, attenuverter etc
// * transparent image for dual color intro
// * rainbow intro?
// * why is one ouput affecting the other in calibration manual cv? (variations in AD or vcc -> pots?)
// * improve potentiometer calibration
// * design and print dual attenuated output (with improved clip)

// done:
// * move generated contents (python scripts) to external files (with .h)
// * clean up code 
// * implement vcf and a few different filters
// * implement primitive delay
// * add dead zone for pot controling ammount of modulation
// * add logo for playback
// * invert tempo for playback
// * fix levels for am (enabling folding)
// * color code cv and pot levels
// * fix voltage levels for osc, adsr etc (not 20VPP)
// * implement adsr
// * add boot image
// * improve vco implementation (new menu with am,fm etc)
// * change AD range (amplification to allow 16 steps between each note (10V @1920 instead of 2048))
// * replace qLut with function (divide AD to get index)
// * switch to float xStep? (incl LUTs)
// * add quantization for cv-in
// * add pulsewidth for squarewave
// * add icons to modes
// * clean up code in modules
// * implement tft menu system with encoder interrupt
// * implement potentiometer filtering (running average)
// * replace waveform with 16 bit position (0-65534)
// * implement reliable rotation encoder support
// * implement ADC DMA reading
// * implement tmrIRQ for playback @ 44.1kHz
// * update spi and display to support 240x240 tft with st7789 driver
// * verify ADC resolution
// * optimize display update (remove flickering)
// note: calibration values for unconnected input (potentiometer calibration): 3238 // target 2473

//=====================================================================================================
// Config Section
//=====================================================================================================

// Which PBC 
// #define pcb_version 0 // the first 3dpcb version
#define pcb_version 1 // the first pcb version, marked rev 1 (menu and P2,P3 direction changed)

// pathThru IN1 -> OUT1 to easily duplicate CV in
#define  CVOut 0 

// Default mode at startup, see the list in modes.h 
#define defaultMain modeVCA
#define defaultSub  mode_VCA

#define version "0.9.0"
//=====================================================================================================
// End Config Section
//=====================================================================================================

/*  potentiometer and I/O mapping
    R  P2
    P4 P3
    C0 C1
    D0 D1
*/

// Encode wiring 
#if (pcb_version == 0)
  #define cw 'W'
  #define cc 'C'
#else
  #define cw 'C'
  #define cc 'W'
#endif

#include <stdio.h>

#include "drivers/SysTick.h"
#include "drivers/display.h"
#include "drivers/spi.h"
#include "drivers/timer.h"
#include "drivers/encoder.h"
#include "drivers/adc.h"
#include "drivers/dac.h"


#include "res/modes.h"
#include "res/colors.h"
#include "res/images.h"

#include "modules/module_vco.h"
#include "modules/module_noise.h"
#include "modules/module_vcf.h"
#include "modules/module_env.h"
#include "modules/module_delay.h"
#include "modules/module_vca.h"
#include "modules/module_lfo.h"
#include "modules/module_cal.h"

#include "lib/menu.h"
#include "lib/scope.h"

extern "C" { 
  #include "gd32vf103.h" 
}

timer Timer;
SysTick stk;
display Display;
spi SPI;
encoder Encoder;
adc ADC;
dac DACS ;
menu Menu;

module_vco vco;
module_noise noise;
module_vcf vcf;
module_env env;
module_delay delay;
module_vca vca ;
module_lfo lfo ;
module_cal cal ;

scope ch0(chan0Color);
scope ch1(chan1Color);
scope ch0old(blackColor);
scope ch1old(blackColor);

// hard coded song CV levels for debug purposes
/* const uint16_t song[] = {2458, 2526, 2577, 2662, 2731, 2577, 2662, 2731,
                         2458, 2526, 2577, 2662, 2731, 2577, 2662, 2731,
                         2458, 2492, 2611, 2697, 2748, 2611, 2697, 2748,
                         2458, 2492, 2611, 2697, 2748, 2611, 2697, 2748,
                         2441, 2492, 2577, 2697, 2748, 2577, 2697, 2748,
                         2441, 2492, 2577, 2697, 2748, 2577, 2697, 2748}; */



volatile int dispPos = 0; // counter for scope column (update screen when dispPos == 240)
uint8_t scopeTrig = 0;    // flag to init scope sampling
uint8_t cvOut = CVOut ;   // use dac0 as cv for vco?


void rcu_config(void)
{
  rcu_periph_clock_enable(RCU_AF); // GPIO IRQ
  rcu_periph_clock_enable(RCU_GPIOA);
  rcu_periph_clock_enable(RCU_ADC0);
  rcu_periph_clock_enable(RCU_DAC);
  rcu_periph_clock_enable(RCU_DMA0);
  rcu_adc_clock_config(RCU_CKADC_CKAPB2_DIV8); // ADC clock = 108MHz / 8 = 13.5MHz(14MHz max.)
}

void gpio_config(void)
{
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0);     // ADC ch0
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_3);     // ADC ch1
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_6);     // ADC pot UR
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_7);     // ADC pot LR
  gpio_init(GPIOB, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_0);     // ADC pot LL
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_4);     // DAC ch0
  gpio_init(GPIOA, GPIO_MODE_AIN, GPIO_OSPEED_50MHZ, GPIO_PIN_5);     // DAC ch1
  gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_5);     // rotary enc A
  gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_6);     // rotary enc B
  gpio_init(GPIOB, GPIO_MODE_IPU, GPIO_OSPEED_50MHZ, GPIO_PIN_4);     // rotary enc C
  gpio_init(GPIOC, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_13); // red LED
  gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_1);  // green LED
  gpio_init(GPIOA, GPIO_MODE_OUT_PP, GPIO_OSPEED_50MHZ, GPIO_PIN_2);  // blue LED
}

void dma_config(void)
{
  /* ADC_DMA_channel configuration */
  dma_parameter_struct dma_data_parameter;
  /* ADC DMA_channel configuration */
  dma_deinit(DMA0, DMA_CH0);
  /* initialize DMA single data mode */
  dma_data_parameter.periph_addr = (uint32_t)(&ADC_RDATA(ADC0));
  dma_data_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
  dma_data_parameter.memory_addr = (uint32_t)(&ADC.anRaw);
  dma_data_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
  dma_data_parameter.periph_width = DMA_PERIPHERAL_WIDTH_16BIT;
  dma_data_parameter.memory_width = DMA_MEMORY_WIDTH_16BIT;
  dma_data_parameter.direction = DMA_PERIPHERAL_TO_MEMORY;
  dma_data_parameter.number = 5;
  dma_data_parameter.priority = DMA_PRIORITY_HIGH;
  dma_init(DMA0, DMA_CH0, &dma_data_parameter);
  dma_circulation_enable(DMA0, DMA_CH0);
  /* enable DMA channel */
  dma_channel_enable(DMA0, DMA_CH0);
}

void addScope(uint16_t c0, uint16_t c1, uint8_t trig)
{
  if (dispPos < 240)
  {
    if (dispPos > 0 || trig == 1)
    {
      ch0.insert((4095 - c0) / 64);
      ch1.insert((4095 - c1) / 64);
      dispPos++;
    }
  }
}

void OnTimer() 
{
  // this is the main loop running @44K
  // it's handling audio processing
  // things should be kept simple and fast here !

  #if (pcb_version == 1)
    ADC.anRaw[2] = 4096 - ADC.anRaw[2]; // pot inverted in this pcb version
    ADC.anRaw[3] = 4096 - ADC.anRaw[3]; // pot inverted in this pcb version
  #endif

  // static float x;
  //static uint8_t songPos;  // position in song
  //static uint16_t tCount;  // temp for song

  int16_t tmp = 0;
  uint32_t cv2;

  // Handle some config options
  // Quantize
  /* if (moduleMode == mode_quantize) {
    vco.quantized ^= 1;
    moduleMode = prevMode;
  }
  // CV OUT if enable IN0 -> OUT0 to chain modules
  if (moduleMode == mode_cvout) {
    cvOut ^= 1;
    moduleMode = prevMode;
  } */
  
  // here are the core of the things 
  switch (Menu.mainMode) {
    // -------------------------- VCO ------------------------------------
    case modeVCO: 
      switch (Menu.subMode) {
        case mode_sine_am:                                               // sine (ampitude modulation)
          tmp = vco.processSineAM(ADC.anAvg[0],ADC.anRaw[1],ADC.anAvg[3],ADC.anAvg[4],ADC.anAvg[2]) ;
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);                 // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig);         // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_sine_add:                                              // sine (sum: in1 is added to the oscillator)
          tmp = vco.processSineADD(ADC.anAvg[0],ADC.anRaw[1],ADC.anAvg[3],ADC.anAvg[4],ADC.anAvg[2]) ;
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);                 // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig);         // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_sine_fm:                                            // sine (frequency modulation)
          tmp = vco.processSineFM(ADC.anAvg[0],ADC.anRaw[1],ADC.anAvg[3],ADC.anAvg[4],ADC.anAvg[2]) ;
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);              // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig);      // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_square_am:                                                      // square (amplitude modulation)
          cv2 = (ADC.anRaw[1] * ADC.anAvg[3] >> 11);                              // 0-8192. pot2 (bottom right) sets level for CV2 that attenuates or amplifies amplitude
          tmp = 2048 + ((int(vco.square(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]), 32768) - 2048) * cv2) >> 12); // get level from position
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);                               // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig);                  // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_square_add:                                    // square (sum: in1 is added to the oscillator)
          cv2 = (ADC.anRaw[1] * ADC.anAvg[3] >> 11);             // 0-8192. pot2 (bottom right) sets level for CV2 that attenuates or amplifies amplitude
          tmp = (int(vco.square(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]), 32768) + cv2) >> 1); // get level from position
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);              // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig); // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_square_fm:                                     // square (frequency modulation)
          cv2 = (ADC.anRaw[1] * ADC.anAvg[3] >> 12);             // 12+12-8=16 (0-65535). pot2 (bottom right) sets level for CV2 that modulates the frequency
          tmp = vco.square(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]) + cv2, 32768);             // get level from position (50% dyuty cycle)
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);              // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig); // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_square_pw:                                     // square (pulsewidth modulation)
          cv2 = (ADC.anRaw[1] * ADC.anAvg[3] >> 8);              // 12+12-8=16 (0-65535). pot2 (bottom right) sets level for CV2 that modulates the pulse width
          tmp = vco.square(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]), cv2);                     // get level from position (50% dyuty cycle)
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);              // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig); // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_square_2x:                                     // 2 slightly detuned square oscillators
          cv2 = (ADC.anRaw[1] * ADC.anAvg[3] >> 18);             // 12+12-8=16 (0-65535). pot2 (bottom right) sets level for CV2 that modulates the pulse width
          tmp = vco.square2x(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]), cv2);                   // get level from position (50% dyuty cycle)
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);              // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig); // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_square_3x:                                     // 3 slightly detuned square oscillators
          cv2 = (ADC.anRaw[1] * ADC.anAvg[3] >> 17);             // 12+12-8=16 (0-65535). pot2 (bottom right) sets level for CV2 that modulates the pulse width
          tmp = vco.square3x(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]), cv2);                   // get level from position (50% dyuty cycle)
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);              // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig); // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_triangle_am:                                                      // triangle (amplitude modulation)
          cv2 = ((ADC.anRaw[1] << 2) * ADC.anAvg[3] >> 11);                         // 0-8192. pot2 (bottom right) sets level for CV2 that attenuates or amplifies amplitude
          tmp = 2048 + ((int(vco.triangle(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]), 32768) - 2048) * cv2) >> 12); // get level from position
          tmp = vco.fold(tmp);                                                          // fold if outside range
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);                                 // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig);                    // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_triangle_add:                                    // triangle (sum: in1 is added to the oscillator)
          cv2 = ((ADC.anRaw[1] << 2) * ADC.anAvg[3] >> 11);        // 0-8192. pot2 (bottom right) sets level for CV2 that attenuates or amplifies amplitude
          tmp = (int(vco.triangle(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]), 32768) + cv2) >> 1); // get level from position
          tmp = vco.fold(tmp);                                         // fold if outside range
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);                // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig);   // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_triangle_fm:                                   // triangle (frequency modulation)
          cv2 = (ADC.anRaw[1] * ADC.anAvg[3] >> 8);              // 12+12-8=16 (0-65535). pot2 (bottom right) sets level for CV2 that modulates the frequency
          tmp = vco.triangle(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]) + cv2, 32768);           // get level from position (50% dyuty cycle)
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);              // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig); // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_triangle_saw:                                  // triangle (shape modulation)
          cv2 = (ADC.anRaw[1] * ADC.anAvg[3] >> 8);              // 12+12-8=16 (0-65535). pot2 (bottom right) sets level for CV2 that modulates the pulse width
          if (cv2 > 32767)                                       // prevent wrap around
            cv2 = 32767;                                         // ..
          cv2 = cv2 * 2;                                         // double value to get full range without modulation signal (pot only)
          tmp = vco.triangle(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]), cv2);                   // get level from position (50% dyuty cycle)
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);              // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig); // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_triangle_2x:                                   // 2 slightly detuned triangle oscillators
          cv2 = (ADC.anRaw[1] * ADC.anAvg[3] >> 18);             // 0-256. pot2 (bottom right) sets level for CV2 that attenuates or amplifies amplitude
          tmp = vco.triangle2x(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]), cv2);                 // get level from position
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);              // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig); // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_triangle_3x:                                   // 3 slightly detuned triangle oscillators
          cv2 = (ADC.anRaw[1] * ADC.anAvg[3] >> 17);             // 0-256. pot2 (bottom right) sets level for CV2 that attenuates or amplifies amplitude
          tmp = vco.triangle3x(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]), cv2);                 // get level from position
          DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);              // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
          addScope(ADC.anRaw[1], tmp * 2 - 2048, vco.phaseTrig); // store levels in scope buffer (amplifies oscillator output 2x)
          break;

      } ; break ;

    // -------------------------- Noise ----------------------------------
    case modeNoise:                                     // white noise generator (PRNG)
      tmp = noise.process(ADC.anRaw[0],ADC.anAvg[4]) ;  // sample, amp
      DACS.dac_out(cvOut ? ADC.anRaw[0] : tmp, tmp);    // output level on DAC1 & DAC0 if ! cvOut (else CV out on DAC0)
      addScope(ADC.anRaw[1], tmp * 2 - 2048, 1);        // store levels in scope buffer (amplifies oscillator output 2x)
      break;
    
    // -------------------------- LFO ------------------------------------
    case modeLFO: 
       switch(Menu.subMode) {
        case mode_LFO_sine:                                                    // LFO sine
          cv2 = ((ADC.anRaw[1] << 2) * ADC.anAvg[3] >> 11);                    // 0-8192. pot2 (bottom right) sets level for CV2 that attenuates or amplifies amplitude
          tmp = 2048 + ((int(vco.sine(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]) / 512) - 2048) * cv2) >> 12); // get level from position
          tmp = vco.fold(tmp);                                                     // fold if outside range
          DACS.dac_out(tmp, tmp);                                                   // output level on DAC0 & DAC1
          addScope(ADC.anRaw[1], tmp * 2 - 2048, 1);                           // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_LFO_square:                                    // LFO  square
          tmp = vco.square(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]) / 512, ADC.anAvg[3] << 4); // get level from position
          DACS.dac_out(tmp, tmp);                                     // output level on DAC0 & DAC1
          addScope(ADC.anRaw[1], tmp * 2 - 2048, 1);             // store levels in scope buffer (amplifies oscillator output 2x)
          break;

        case mode_LFO_triangle:                                    // LFO triangle
          tmp = vco.triangle(vco.getPhase(ADC.anAvg[0],ADC.anAvg[4],ADC.anAvg[2]) / 512, ADC.anAvg[3] << 4); // get level from position
          DACS.dac_out(tmp, tmp);                                       // output level on DAC0 & DAC1
          addScope(ADC.anRaw[1], tmp * 2 - 2048, 1);               // store levels in scope buffer (amplifies oscillator output 2x)
          break;

      } ; break ;

    // -------------------------- VCF ------------------------------------
    case modeVCF: 
       switch(Menu.subMode) {
        case mode_4_pole:                                              // VCF (Voltage Controled Filter)
          tmp = vcf.r4pole1(ADC.anRaw[0], ADC.anAvg[4], ADC.anAvg[3]); // sample, cutoff, resonance
          DACS.dac_out(tmp, tmp);                                      // output level on DAC0 & DAC1
          addScope(ADC.anRaw[0], tmp, vcf.phaseTrig);                  // store levels in scope buffer
          break;

        case mode_2_pole_v1:                                           // VCF (Voltage Controled Filter)
          tmp = vcf.r2pole1(ADC.anRaw[0], ADC.anAvg[4], ADC.anAvg[3]); // sample, cutoff, resonance
          DACS.dac_out(tmp, tmp);                                      // output level on DAC0 & DAC1
          addScope(ADC.anRaw[0], tmp, vcf.phaseTrig);                  // store levels in scope buffer
          break;

        case mode_2_pole_v2:                                           // VCF (Voltage Controled Filter)
          tmp = vcf.r2pole2(ADC.anRaw[0], ADC.anAvg[4], ADC.anAvg[3]); // sample, cutoff, resonance
          DACS.dac_out(tmp, tmp);                                      // output level on DAC0 & DAC1
          addScope(ADC.anRaw[0], tmp, vcf.phaseTrig);                  // store levels in scope buffer
          break;

        case mode_test:                                             // VCF (Voltage Controled Filter)
          tmp = vcf.test(ADC.anRaw[0], ADC.anAvg[4], ADC.anAvg[3]); // sample, cutoff, resonance
          DACS.dac_out(tmp, tmp);                                   // output level on DAC0 & DAC1
          addScope(ADC.anRaw[0], tmp, vcf.phaseTrig);               // store levels in scope buffer
          break;

      } ; break ;

    // -------------------------- VCA ------------------------------------
    case modeVCA:                                                               // VCA (Voltage Controled Amplifier)
      tmp = vca.process(ADC.anRaw[0],ADC.anAvg[1],ADC.anAvg[3],ADC.anAvg[2]) ;  // sample, env, amp, offset
      DACS.dac_out(tmp, tmp);                                                   // output level on DAC0 & DAC1
      addScope(ADC.anRaw[0], tmp , 1);                                          // store levels in scope buffer
      break;

   // -------------------------- Env ------------------------------------
    case modeEnv:
      switch (Menu.subMode) {
        case mode_ADSR:                                                                                  // adsr
          tmp = env.int_adsr(ADC.anRaw[0], ADC.anAvg[2] << 1, ADC.anAvg[4] << 1, ADC.anAvg[3] << 2, 0); // trig, A,D,R,!S
          cv2 = 2048 + (((tmp - 2048) * (ADC.anRaw[1] - 2048)) >> 9);                                    //
          DACS.dac_out(tmp, cv2);                                                                             // output level on DAC0 & DAC1
          break;

        case mode_ADR:                                                                                   // adsr
          tmp = env.int_adsr(ADC.anRaw[0], ADC.anAvg[2] << 1, ADC.anAvg[4] << 1, ADC.anAvg[3] << 2, 1); // trig, A,D,R,!S
          cv2 = 2048 + (((tmp - 2048) * (ADC.anRaw[1] - 2048)) >> 9);                                    //
          DACS.dac_out(tmp, cv2);                                                                             // output level on DAC0 & DAC1
          break;

      } ; break ;

    // -------------------------- Delay ----------------------------------
    case modeDelay:                                                         // Delay
      tmp = delay.delay(ADC.anRaw[0], ADC.anAvg[4], ADC.anAvg[3]);          // sample, mix, length
      DACS.dac_out(tmp, tmp);                                               // output level on DAC0 & DAC1
      addScope(ADC.anRaw[0], tmp * 2 - 2048, 1);                            // store levels in scope buffer (amplifies oscillator output 2x)
      break;

    // -------------------------- Calibration ----------------------------  
    case modeCalibrate:
      switch (Menu.subMode) {
        case mode_out_minus10:            // callibrate -10V
          DACS.dac_out(128, 128);         // 3840*(-10+10)/20+128 = 128
          vco.phaseC = 0;                 // DC => 0Hz
          addScope(ADC.anRaw[1], tmp, 1); // store levels in scope buffer
          break;

        case mode_out_0V:                 // callibrate 0V
          DACS.dac_out(2048, 2048);       // 3840*(0+10)/20+128 = 2048
          vco.phaseC = 0;                 // DC => 0Hz
          addScope(ADC.anRaw[1], tmp, 1); // store levels in scope buffer
          break;

        case mode_out_3V3:                // callibrate 3V3
          DACS.dac_out(2682, 2682);       // 3840*(3.3+10)/20+128 = 2682
          vco.phaseC = 0;                 // DC => 0Hz
          addScope(ADC.anRaw[1], tmp, 1); // store levels in scope buffer
          break;

        case mode_out_10V:                // callibrate 10V
          DACS.dac_out(3968, 3968);       // 3840*(10+10)/20+128 = 3968
          vco.phaseC = 0;                 // DC => 0Hz
          addScope(ADC.anRaw[1], tmp, 1); // store levels in scope buffer
          break;

        case mode_in1_to_out1:                                             // calibrate input (connect out to in and match triangle waveform)
          vco.phaseC = 450;                                                //
          tmp = vco.triangle(vco.phaseC, ADC.anAvg[3] << 4);               // triangle waveform (get level from position)
          DACS.dac_out(tmp, tmp);                                          // output level on DAC0 & DAC1
          addScope(ADC.anRaw[0] * 2 - 2048, tmp * 2 - 2048, vco.phaseTrig); // store levels in scope buffer
          break;

        case mode_in2_to_out1:                                              // calibrate input (connect out to in and match triangle waveform)
          vco.phaseC = 450;                                                 //
          tmp = vco.triangle(vco.phaseC, ADC.anAvg[3] << 4);                // triangle waveform (get level from position)
          DACS.dac_out(tmp, tmp);                                           // output level on DAC0 & DAC1
          addScope(ADC.anRaw[1] * 2 - 2048, tmp * 2 - 2048, vco.phaseTrig); // store levels in scope buffer
          break;

        case mode_pass_thru:                                                // pass thru to meassure noise and crosstalk
          DACS.dac_out(ADC.anRaw[0], ADC.anRaw[1]);                         // output ADC0 to DAC0 & ACD1 to DAC1
          addScope(ADC.anRaw[0] * 2 - 2048, ADC.anRaw[1] * 2 - 2048, 1);    // store levels in scope buffer
          break;

        case mode_square_A4:                          // used to measure frequency accuracy
          vco.phaseC = 1299;                          // 440Hz - A4 concert pitch (midi 69) Should be 2611.2 @ 108MHz -> 44.4KHz
          tmp = vco.square(vco.phaseC, 32768);        // square waveform (get level from position)
          DACS.dac_out(tmp, tmp);                     // output level on DAC0 & DAC1
          addScope(ADC.anRaw[0], tmp, vco.phaseTrig); // store levels in scope buffer
          break;

        case mode_manual_sine:
          tmp = (ADC.anAvg[2] >> 1) + 1472;           // 1472->3520 (not full range:1472->3584)
          vco.phaseC = vco.ad2phase(tmp);             // convert CV to logaritmic scale
          tmp = vco.sine(vco.phaseC);                 // get level from position
          DACS.dac_out(tmp, tmp);                     // output level on DAC0 & DAC1
          addScope(ADC.anAvg[2], tmp, vco.phaseTrig); // store levels in scope buffer
          break;

        case mode_manual_CV:                          // pot 3 controls cv1, pot2 controls cv2
          DACS.dac_out(ADC.anAvg[4], ADC.anAvg[3]);   // 4095*(10+10)/20 = 4095
          vco.phaseC = 0;                             // DC => 0Hz
          addScope(ADC.anAvg[4], ADC.anAvg[3], 1);    // store levels in scope buffer
          break;

      } ; break ;

     /*    case mode_CV_Play:
      tCount++;
      if (tCount < (4095 - ADC.anAvg[2]) * 5)  // genereate trig signal (50% duty)
        cv2 = 3008;                            // 5V trig signal (> 3.3V threshold)
      else                                     // no trig
        cv2 = 2048;                            // 0V
      if (tCount > (4095 - ADC.anAvg[2]) * 10) // playback speed / tempo
      {
        tCount = 0;
        songPos++;
        if (songPos >= 48)
          songPos = 0;
      }
      tmp = song[songPos];            // add pot + quantization?
      DACS.dac_out(tmp, cv2);              // output level on DAC0 & DAC1
      vco.phaseC = 0;                     // DC => 0Hz
      addScope(tmp, cv2, tCount ^ 1); // store levels in scope buffer
      break; */ /*    case mode_CV_Play:
      tCount++;
      if (tCount < (4095 - ADC.anAvg[2]) * 5)  // genereate trig signal (50% duty)
        cv2 = 3008;                            // 5V trig signal (> 3.3V threshold)
      else                                     // no trig
        cv2 = 2048;                            // 0V
      if (tCount > (4095 - ADC.anAvg[2]) * 10) // playback speed / tempo
      {
        tCount = 0;
        songPos++;
        if (songPos >= 48)
          songPos = 0;
      }
      tmp = song[songPos];            // add pot + quantization?
      DACS.dac_out(tmp, cv2);              // output level on DAC0 & DAC1
      vco.phaseC = 0;                     // DC => 0Hz
      addScope(tmp, cv2, tCount ^ 1); // store levels in scope buffer
      break; */

      /* case mode_SnH:                                              // SnH (Sample and Hold)
      tmp = vcf.test(ADC.anRaw[0], ADC.anAvg[4], ADC.anAvg[3]); // sample, cutoff, resonance
      DACS.dac_out(tmp, tmp);                                        // output level on DAC0 & DAC1
      addScope(ADC.anRaw[0], tmp * 2 - 2048, 1);                // store levels in scope buffer (amplifies oscillator output 2x)
      break;
    */
  } ;
  
  // update ADC averages and pull new values 
  ADC.update() ;

  // TODO: check if it can't be moved to the main loop to keep audio loop from jitters
  char encoderValue = Encoder.get() ;
  switch  (encoderValue) {
    case cw : Menu.prev() ; break ;
    case cc : Menu.next() ; break ;
    case 'S': Menu.switchMode() ; break ;
  };

}

int main() {

  uint8_t oldMainMode = 0 ;
  uint8_t needScope   = 1 ;
  char str[16] ;

  Menu.mainMode = defaultMain ;      // main mode VCO / VCA / VCF / ....
  Menu.subMode  = defaultSub ;       // sub mode : sine_am / triangle_fm / etc ....

  SPI.begin();
  stk.begin(1000);
  rcu_config();
  gpio_config();
  dma_config();

  ADC.init(stk);
  DACS.init();
  
  Display.begin(&SPI);

  // Welome screen
  sprintf(str,version) ;
  Display.showImg(img_1337, sizeof(img_1337), 0, 50, 240, 86, welcomeUpperColor);
  Display.showImg(img_modular, sizeof(img_modular), 0, 150, 240, 32, welcomeLowerColor);
  Display.putStr(str,80,220,welcomeUpperColor,blackColor);
  stk.delay(2000);

  //clean screen
  Display.fillRectangle(0, 0, 240, 240, blackColor );

  // create and attach main timer loop @44K
  // this loop is timer driven and handle audio part
  Timer.begin();
  Timer.attach(OnTimer);

  // endless loop which handle screen
  while (1) {
    if (Menu.mainMode != oldMainMode) { 
        switch (Menu.mainMode) {
          case modeVCO:
            needScope    = 1 ; 
            Menu.logoImage    = vcoImage ;
            Menu.imgSize      = sizeof(vcoImage) ;
            Menu.subModeColor = vcoColor ;
            Menu.subMenuTxt   = vco.subMenuText ;
            Menu.subMenuMax   = vcoSubMenuMax ;
            Menu.potsName     = vco.potsName  ;
            break ;
          case modeVCA:
            needScope    = 1 ; 
            Menu.logoImage    = vcaImage ;
            Menu.imgSize      = sizeof(vcaImage) ;
            Menu.subModeColor = vcaColor ;
            Menu.subMenuTxt   = vca.subMenuText ;
            Menu.subMenuMax   = vcaSubMenuMax ;
            Menu.potsName     = vca.potsName  ;
            break ;
          case modeVCF:
            needScope    = 1 ; 
            Menu.logoImage    = vcfImage ;
            Menu.imgSize      = sizeof(vcfImage) ;
            Menu.subModeColor = vcfColor ;
            Menu.subMenuTxt   = vcf.subMenuText ;
            Menu.subMenuMax   = vcfSubMenuMax ;
            Menu.potsName     = vcf.potsName  ;
            break ;
          case modeLFO:
            needScope    = 1 ;
            Menu.logoImage    = lfoImage ;
            Menu.imgSize      = sizeof(lfoImage) ;
            Menu.subModeColor = lfoColor ;
            Menu.subMenuTxt   = lfo.subMenuText ;
            Menu.subMenuMax   = lfoSubMenuMax ;
            Menu.potsName     = lfo.potsName  ;
            break ;
          case modeEnv:
            needScope    = 0 ; 
            Menu.logoImage    = envImage ;
            Menu.imgSize      = sizeof(envImage) ;
            Menu.subModeColor = envColor ;
            Menu.subMenuTxt   = env.subMenuText ;
            Menu.subMenuMax   = envSubMenuMax ;
            Menu.potsName     = env.potsName  ;
            break ;
          case modeDelay:
            needScope    = 1 ; 
            Menu.logoImage    = delayImage ;
            Menu.imgSize      = sizeof(delayImage) ;
            Menu.subModeColor = delayColor ;
            Menu.subMenuTxt   = delay.subMenuText ;
            Menu.subMenuMax   = delaySubMenuMax ;
            Menu.potsName     = delay.potsName  ;
            break ;
          case modeNoise:
            needScope    = 1 ;
            Menu.logoImage    = noiseImage ;
            Menu.imgSize      = sizeof(noiseImage) ;
            Menu.subModeColor = noiseColor ;
            Menu.subMenuTxt   = noise.subMenuText ;
            Menu.subMenuMax   = noiseSubMenuMax ;
            Menu.potsName     = noise.potsName  ;
            break ;
          case modeCalibrate:
            needScope    = 1 ;
            Menu.logoImage    = calImage ;
            Menu.imgSize      = sizeof(calImage) ;
            Menu.subModeColor = calColor ;
            Menu.subMenuTxt   = cal.subMenuText ;
            Menu.subMenuMax   = calSubMenuMax ;
            Menu.potsName     = cal.potsName  ;
            break ;
        }

      Menu.drawSkel(Display) ;      // draw the basic mode skel
      Menu.drawPotsName(Display) ;  // draw pots Name 
      oldMainMode = Menu.mainMode;  // update old mode 

    }

    // redraw submenu if needed 
    if (Menu.needUpdate == 1) Menu.drawSubmode(Display) ;

    // show AD(S)R
      if (Menu.mainMode == modeEnv) {
        switch (Menu.subMode) {
          case mode_ADSR:
            env.draw(Display, ADC.anRaw[0], ADC.anAvg[2], ADC.anAvg[4], ADC.anAvg[3], 0); // display, gate, A, D, R, !S
            break;
          case mode_ADR:
            env.draw(Display, ADC.anRaw[0], ADC.anAvg[2], ADC.anAvg[4], ADC.anAvg[3], 1); // display, gate, A, D, R, !S
            break;
        }
      }

    if (dispPos >= 240 ) {

      // show scope
      if (needScope == 1 ) {
        //scope.showScope(Display) ;
        for (uint32_t i = 1; i < SCOPE_BUFFER_SIZE - 1; i++) {
          
          ch0old.draw1(Display, i);     // clear old pixel
          ch0.draw1(Display, i);        // write new pixel
          ch0old.write(i, ch0.read(i)); // save old position

          ch1old.draw1(Display, i);
          ch1.draw1(Display, i);
          ch1old.write(i, ch1.read(i));
        }
        dispPos = 0;
      }

    } 

      // show AD values
      // --------------------------- Line 1 --------------------------------
      // Freq only for VCO
      if (Menu.mainMode == modeVCO) {
        uint16_t freqency = int(vco.phaseC * 44400 / (2 * 65536));
        if (freqency < 1000)
          sprintf(str, "Freq : %dHz", freqency);
        else
          sprintf(str, "Freq : %d", freqency);
        Display.fillRectangle(64, 180, 72, 16, blackColor);
        Display.putStr(str, 0, 180, whiteColor, blackColor);
      } else {
        Display.fillRectangle(0, 180, 64, 16, blackColor);
      }

      // POT UR
      sprintf(str, "%d",ADC.anAvg[2]);
      Display.fillRectangle(130+64, 180, 40, 16, blackColor);
      Display.putStr(str, 130+64, 180, potUpperRightColor, blackColor); 
      
      // --------------------------- Line 2 --------------------------------
      // POT LL
      sprintf(str, "%d",ADC.anAvg[4]);
      Display.fillRectangle(64,200, 64, 16, blackColor);
      Display.putStr(str, 64 , 200, potLowerLeftColor, blackColor);

      // POT LR 
      sprintf(str, "%d",ADC.anAvg[3]);
      Display.fillRectangle(130+64, 200, 88, 16, blackColor);
      Display.putStr(str, 130+64, 200, potLowerRightColor, blackColor);

      // --------------------------- Line 3 --------------------------------
      // IN 1 
      sprintf(str, "In 0 : %d", ADC.anRaw[0]);
      //sprintf(str, "In 1 : %d V", (4095 - ADC.anAvg[0]) * 10 / 2048 - 10);
      Display.fillRectangle(64, 220, 64, 16, blackColor);
      Display.putStr(str, 0, 220, in1Color, blackColor);
      
      // IN 2
      sprintf(str, "In 1 : %d", ADC.anRaw[1]);
      //sprintf(str, "In 2 : %d V", (4095 - ADC.anAvg[1]) * 10 / 2048 - 10);
      Display.fillRectangle(130+64, 220, 64, 16, blackColor);
      Display.putStr(str, 130 , 220, in2Color, blackColor);
    
       
    }
  
}
