#include "dac.h"
extern "C"
{
    #include <gd32vf103.h>
}

void dac::dac_out(uint16_t d0, uint16_t d1)
{
  dac_data_set(DAC0, DAC_ALIGN_12B_R, 4095 - d0); // inverted due to inverting op amp
  dac_data_set(DAC1, DAC_ALIGN_12B_R, 4095 - d1); // inverted due to inverting op amp
  dac_software_trigger_enable(DAC0);
  dac_software_trigger_enable(DAC1);
}

void dac::init(void)
{
  dac_deinit();
  dac_trigger_source_config(DAC0, DAC_TRIGGER_SOFTWARE);
  dac_trigger_enable(DAC0);
  dac_wave_mode_config(DAC0, DAC_WAVE_DISABLE);
  dac_output_buffer_enable(DAC0);
  dac_enable(DAC0);

  dac_trigger_source_config(DAC1, DAC_TRIGGER_SOFTWARE);
  dac_trigger_enable(DAC1);
  dac_wave_mode_config(DAC1, DAC_WAVE_DISABLE);
  dac_output_buffer_enable(DAC1);
  dac_enable(DAC1);
}