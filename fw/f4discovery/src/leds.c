#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>

#include "leds.h"

void leds_init(void)
{
	rcc_periph_clock_enable(RCC_GPIOD);

	gpio_mode_setup(GPIOD, GPIO_MODE_OUTPUT, GPIO_PUPD_NONE, LED_ALL);
	gpio_clear(GPIOD, LED_ALL); // turn off leds
}
