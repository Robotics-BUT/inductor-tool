#include <libopencm3/cm3/nvic.h>
#include <libopencm3/stm32/rcc.h>
#include <libopencm3/stm32/gpio.h>
#include <libopencm3/stm32/adc.h>
#include <libopencm3/stm32/timer.h>
#include <libopencm3/usb/usbd.h>
#include <libopencm3/usb/cdc.h>
#include <libopencm3/cm3/scb.h>
#include <stdio.h>
#include <errno.h>
#include <stddef.h>
#include <sys/types.h>

#include "leds.h"
#include "serial.h"
#include "pwm.h"
#include "current.h"
#include "usb_vilem.h"
#include "usb_vilem.c"
#include "dac.h"
#include "protokol.h"


struct Param_Mer Parametry;
uint8_t usbd_control_buffer[64];
uint16_t nas_koef=1;
static uint8_t rbuf[1024];
static uint8_t tbuf[1024];
FILE *us2;
usbd_device *usbd_dev;
volatile bool run = false;

void usa_rxb(uint8_t ch);
void adc_finish(uint16_t values[]);
void inicializace(void);

void inicializace()
{
    rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_120MHZ]); //nastaveni obecnych hodin
//usb
    rcc_periph_clock_enable(RCC_GPIOD); //povoleni hodin pro port D kvuli bliknu ti led pri poslani 'p' - pak ostranit

    rcc_peripheral_enable_clock(&RCC_AHB1ENR, RCC_AHB1ENR_IOPAEN); //povoleni hodin pro usb
    rcc_peripheral_enable_clock(&RCC_AHB2ENR, RCC_AHB2ENR_OTGFSEN); //povoleni hodin pro usb

    gpio_mode_setup(GPIOA, GPIO_MODE_AF, GPIO_PUPD_NONE, GPIO9 | GPIO11 | GPIO12); //u portu A
    gpio_set_af(GPIOA, GPIO_AF10, GPIO9 | GPIO11 | GPIO12); //na pinech 9, 11, 12 posru A alternativni fce USB
    usbd_dev = usbd_init(&otgfs_usb_driver, &dev, &config, usb_strings, 3, usbd_control_buffer, sizeof(usbd_control_buffer)); //vytvoreni noveho USB
    usbd_register_set_config_callback(usbd_dev, cdcacm_set_config); //nastaveni "funkci" ktere jsou volany na obsluhu
//usb end

    us2 = fopenserial(1, 115200, tbuf,1024,rbuf,1024); // novy soubor pro uart



    leds_init();  //inicializace let
    pwm_init();    //inicializace pwm
    current_init(); //inicializace AD prevodniku
    dac_init(); //inicializace DAC
    init_protokol(); //inicializace protokolu
}

void adc_finish(uint16_t values[])
{
    if (!run)
        return;

    char zn[8];
    zn[0]=pwm[3];
    zn[1]=pwm[3]/256;
    zn[2]=values[0];
    zn[3]=values[0]/256;
    zn[4]=values[1];
    zn[5]=values[1]/256;
    zn[6]=values[2];
    zn[7]=values[2]/256;

    /*fwrite(&pwm[3],1,2,us2);
    fwrite(&values[0],1,2,us2);
    fwrite(&values[1],1,2,us2);
    fwrite(&values[2],1,2,us2);*/

    usbd_ep_write_packet(usbd_dev, 0x82, &zn, 8);

    // increment TIM5/OC4
    pwm[3] = (pwm[3] + nas_koef) & (0xFFF*nas_koef);

    if (pwm[3] == 0)
    {
        run = false;
        if(Parametry.set_param==1)
            set_mer();
    }


    timer_set_oc_value(TIM5, TIM_OC4, pwm[3]);
}

void usa_rxb(uint8_t ch)
{
    (void)ch;
    pwm[3] = 1;
    timer_set_oc_value(TIM5, TIM_OC4, pwm[0]);
    run = true;
}

int main(void)
{
    inicializace();

	while (1)
    {
		for (int i = 0; i < 100000000; i++)	/* Wait a bit. */
		{
		    //__asm__("nop");
		    usbd_poll(usbd_dev); //obsluha USB
		}

        LED_TGL(LED0);

        pwm[3] = 1;
        timer_set_oc_value(TIM5, TIM_OC4, pwm[3]);
        run = true;
	}

	return 0;
}

