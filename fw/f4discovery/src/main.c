#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/timer.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>

#include "leds.h"
#include "serial.h"
#include "pwm.h"
#include "current.h"

static uint8_t rbuf[1024];
static uint8_t tbuf[1024];
FILE *us2;

void usa_rxb(uint8_t ch);
void adc_finish(uint16_t values[]);

volatile bool run = false;

void adc_finish(uint16_t values[])
{
    if (!run)
        return;

    fwrite(&pwm[3],1,2,us2);
    fwrite(&values[0],1,2,us2);
    fwrite(&values[1],1,2,us2);
    fwrite(&values[2],1,2,us2);

    // increment TIM5/OC4
    pwm[3] = (pwm[3] + 1) & 0xFFF;

    if (pwm[3] == 0)
        run = false;

    timer_set_oc_value(TIM5, TIM_OC4, pwm[3]);
}

void usa_rxb(uint8_t ch)
{
    (void)ch;
    pwm[3] = 1;
    timer_set_oc_value(TIM5, TIM_OC4, pwm[3]);
    run = true;
}

int main(void)
{
    rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_168MHZ]);

    us2 = fopenserial(1, 115200, tbuf,1024,rbuf,1024);

    leds_init();
    pwm_init();
    current_init();

	while (1) {

		for (int i = 0; i < 1000000000; i++)	/* Wait a bit. */
			__asm__("nop");

        LED_TGL(LED0);

        pwm[3] = 1;
        timer_set_oc_value(TIM5, TIM_OC4, pwm[3]);
        run = true;
	}

	return 0;
}
