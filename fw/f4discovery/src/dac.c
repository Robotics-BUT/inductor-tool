
#include "dac.h"
void dac_init(void)
{
	rcc_clock_setup_hse_3v3(&hse_8mhz_3v3[CLOCK_3V3_120MHZ]); //nastaveni globalnich hodin
	rcc_periph_clock_enable(RCC_GPIOA);     //pusteni hodin pro port A
	rcc_periph_clock_enable(RCC_DAC);       //pusteni hodin pro DAC

	gpio_mode_setup(GPIOA, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO4); //pin4 portu A nako analogovy bez pullu/pulldown
	dac_disable(CHANNEL_1);         //zakazani kanalu 1 DAC
	dac_disable_waveform_generation(CHANNEL_1);     //bastaveni generovani signalu na kanalu 1 DAC
	dac_enable(CHANNEL_1);          //Povoleni DAC na kanalu 1
	dac_set_trigger_source(DAC_CR_TSEL1_SW); //Triger kanalu 1 nastaven na softwarovy
}


/* takhle se pak nastavi hodnota do DAC

    dac_load_data_buffer_single(j, RIGHT12, CHANNEL_1);//do kanalu 1 DAC nahrat hodnotu (j)
	dac_software_trigger(CHANNEL_1); //pustit hodnotu ven

*/
