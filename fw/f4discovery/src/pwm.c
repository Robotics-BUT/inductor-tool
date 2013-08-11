#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/timer.h>

#include "pwm.h"

uint32_t pwm[4] = { 0x4FF, 0x2FF, 0x7FF, 1};

void pwm_init(void)
{
	rcc_periph_clock_enable(RCC_TIM5);
	rcc_periph_clock_enable(RCC_GPIOA);

	gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO0 | GPIO1 | GPIO2 | GPIO3);

	gpio_set_af(GPIOA, GPIO_AF2, GPIO0 | GPIO1 | GPIO2 | GPIO3);

	gpio_set_output_options(GPIOA, GPIO_OTYPE_PP, GPIO_OSPEED_50MHZ,  GPIO0 | GPIO1 | GPIO2 | GPIO3);

	timer_reset(TIM5);
    timer_set_mode(TIM5, TIM_CR1_CKD_CK_INT, TIM_CR1_CMS_EDGE, TIM_CR1_DIR_UP);
    timer_set_prescaler(TIM5, 1);   // 20.5kHz
    timer_set_period(TIM5, 0xFFF);
    timer_set_counter(TIM5, 0);
    timer_set_repetition_counter(TIM5, 0);
    timer_continuous_mode(TIM5);

    /* -- OC1 and OC1N configuration -- */

	timer_disable_oc_output(TIM5, TIM_OC1);         // CCxE
    timer_enable_oc_preload(TIM5, TIM_OC1);         // OCxPE           MUST for OC
	timer_set_oc_mode(TIM5, TIM_OC1, TIM_OCM_PWM1); // CCxS, OCxM
	timer_set_oc_polarity_low(TIM5, TIM_OC1);      // CCxP
	timer_set_oc_idle_state_set(TIM5, TIM_OC1);     // OISx
	timer_set_oc_value(TIM5, TIM_OC1, pwm[0]);
	timer_enable_oc_output(TIM5, TIM_OC1);          // CCxE

    /* -- OC2 and OC2N configuration -- */

	timer_disable_oc_output(TIM5, TIM_OC2);
    timer_enable_oc_preload(TIM5, TIM_OC2);
    timer_set_oc_mode(TIM5, TIM_OC2, TIM_OCM_PWM1);
	timer_set_oc_polarity_high(TIM5, TIM_OC2);
	timer_set_oc_idle_state_set(TIM5, TIM_OC2);
	timer_set_oc_value(TIM5, TIM_OC2, pwm[1]);
	timer_enable_oc_output(TIM5, TIM_OC2);

    /* -- OC3 and OC2N configuration -- */

	timer_disable_oc_output(TIM5, TIM_OC3);
    timer_enable_oc_preload(TIM5, TIM_OC3);
    timer_set_oc_mode(TIM5, TIM_OC3, TIM_OCM_PWM1);
	timer_set_oc_polarity_high(TIM5, TIM_OC3);
	timer_set_oc_idle_state_set(TIM5, TIM_OC3);
	timer_set_oc_value(TIM5, TIM_OC3, pwm[2]);
	timer_enable_oc_output(TIM5, TIM_OC3);

    /* -- OC4 and OC2N configuration -- */

	timer_disable_oc_output(TIM5, TIM_OC4);
    timer_enable_oc_preload(TIM5, TIM_OC4);
    timer_set_oc_mode(TIM5, TIM_OC4, TIM_OCM_PWM1);
	timer_set_oc_polarity_high(TIM5, TIM_OC4);
	timer_set_oc_idle_state_set(TIM5, TIM_OC4);
	timer_set_oc_value(TIM5, TIM_OC4, pwm[3]);
	timer_enable_oc_output(TIM5, TIM_OC4);


	/* ---- */

	timer_enable_preload(TIM5);                             // ARPE            MUST for upcount or center-aligned5
	timer_enable_break_main_output(TIM5);                   // MOE

 //   timer_enable_irq(TIM5, TIM_DIER_CC4IE);
    timer_enable_counter(TIM5);

 //   nvic_enable_irq(NVIC_TIM5_IRQ);
}

static uint8_t counter;

void tim5_isr(void)
{
    if (timer_interrupt_source(TIM5,TIM_SR_CC4IF))
    {
        if (++counter == 0)
        {

        }
    }
}

void pwm_update(void)
{
    timer_set_oc_value(TIM5, TIM_OC1, pwm[0]);
    timer_set_oc_value(TIM5, TIM_OC2, pwm[1]);
    timer_set_oc_value(TIM5, TIM_OC3, pwm[2]);
    timer_set_oc_value(TIM5, TIM_OC4, pwm[3]);
}
