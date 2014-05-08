#ifndef DAC_H_INCLUDED
#define DAC_H_INCLUDED
#include <libopencm3/stm32/dac.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/rcc.h>
void dac_init(void);
void set_napeti(uint16_t napeti);
#endif // DAC_H_INCLUDED
