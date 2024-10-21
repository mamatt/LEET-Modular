#include "adc.h"
extern "C"
{
    #include "drivers/SysTick.h"
    #include "gd32vf103.h"
}

// anUpdate (downshifted oversampling)
//uint8_t anCount = 0;      // counter for oversampling (new value after 255, @44,4KHz => 173Hz, suitable for potentiometers)
//uint32_t anTotal[MAX_AN]; // the running total
//int anAvg[MAX_AN];        // analog average
//uint16_t anRaw[8];          // raw sampled values stored by DMA

// analog average (downshifted oversampling)
void adc::updateAVG(uint8_t ch)
{
    if (anCount < 255)
    {
        anTotal[ch] += anRaw[ch];
    }
    else
    {
        if (ch == 2 || ch == 3)                    // these potentiometers are inverted due to routing
            anAvg[ch] = 4095 - (anTotal[ch] >> 8); // note that raw[2] and raw[3] is still inverted...
        else
            anAvg[ch] = anTotal[ch] >> 8; // inputs and pot 3 is not inverted
        anTotal[ch] = 0;
    }
}

void adc::update()
{
  // Running average is done after DAC since it will add jitter due to uneven number of cycles
  anCount++;
  updateAVG(0); // ADC0
  updateAVG(1); // ADC1
  updateAVG(2); // pot0 (not inverted anymore)
  updateAVG(3); // pot1 (not inverted anymore)
  updateAVG(4); // pot2

  // Start ADC sampling (using DMA)
  adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL); // start ADC: DMA => adc_value[]
}

void adc::init(SysTick &stk) {
    /* reset ADC */
    adc_deinit(ADC0);
    /* ADC mode config */
    adc_mode_config(ADC_MODE_FREE); // ADC0 and ADC1 work independently
    /* ADC data alignment config */
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    /* Configure word length */
    adc_resolution_config(ADC0, ADC_RESOLUTION_12B);
    /* ADC channel length config */
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, 5);
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_0, ADC_SAMPLETIME_13POINT5); // ADC left 41.4+12.5=54 cycles =5uS *5=20us => 50.1Hz
    adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_3, ADC_SAMPLETIME_13POINT5); // ADC right
    adc_regular_channel_config(ADC0, 2, ADC_CHANNEL_6, ADC_SAMPLETIME_13POINT5); // pot0 (UR)
    adc_regular_channel_config(ADC0, 3, ADC_CHANNEL_7, ADC_SAMPLETIME_13POINT5); // pot1 (LR)
    adc_regular_channel_config(ADC0, 4, ADC_CHANNEL_8, ADC_SAMPLETIME_13POINT5); // pot2 (LL)
    /* ADC trigger config */
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC0_1_EXTTRIG_REGULAR_NONE); // software trigger
    /* ADC external trigger enable */
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, ENABLE);
    /* ADC discontinuous mode */
    adc_discontinuous_mode_config(ADC0, ADC_REGULAR_CHANNEL, 5); // should it be 4?
    /* enable ADC interface */
    adc_enable(ADC0);
    stk.delay(1);
    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC0);
    stk.delay(1);
    /* ADC DMA function enable */
    adc_dma_mode_enable(ADC0);
}