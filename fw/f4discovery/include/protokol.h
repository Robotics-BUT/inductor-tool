#ifndef PROTOKOL_H_INCLUDED
#define PROTOKOL_H_INCLUDED


#include <libopencm3/cm3/scb.h>
#include <libopencm3/stm32/timer.h>

#include "pwm.h"
#include "dac.h"

struct Param_Mer
{
    uint8_t set_param;
    uint8_t start_mer;
    uint16_t Napeti;
    uint16_t Cas_Nas;
    uint32_t default_pwm[4];
};

void protokol(char *buf);
void set_mer(void);
void init_protokol(void);

#endif // PROTOKOL_H_INCLUDED
