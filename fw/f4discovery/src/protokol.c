#include "protokol.h"

extern volatile bool run;
extern struct Param_Mer Parametry;
extern int16_t nas_koef;

//naplni strukturu daty z retezce
void protokol(char *buf) //0 pro pøepsat  nastaveni, 1 pro spustit a 2 pro pøepsat a spustit
{
    if(buf[0]==0)
    {
        Parametry.Cas_Nas=buf[3]*256+buf[4];
        Parametry.Napeti=buf[1]*256+buf[2];
        Parametry.set_param=1;
        Parametry.start_mer=0;
    }
    else if(buf[0]==1)
    {
        pwm[3] = 1;
        timer_set_oc_value(TIM5, TIM_OC4, pwm[3]);
        run=true;
    }
    else if(buf[0]==2)
    {
        Parametry.Cas_Nas=buf[3]*256+buf[4];
        Parametry.Napeti=buf[1]*256+buf[2];
        Parametry.set_param=1;
        Parametry.start_mer=1;
        set_mer();
    }
    else
    {
        run=false;
    }

}

void set_mer(void) //prenese parametry ze struktury do promenych, a podle hodnoty ve strukture spusti enbo enspusti mereni
{
    set_napeti(Parametry.Napeti);
    nas_koef=Parametry.Cas_Nas;
    pwm[0]=Parametry.Cas_Nas*Parametry.default_pwm[0];
    pwm[1]=Parametry.Cas_Nas*Parametry.default_pwm[1];
    pwm[2]=Parametry.Cas_Nas*Parametry.default_pwm[2];
    pwm_init();
    if(Parametry.start_mer==1)
        run=true;

}

void init_protokol(void) // dopsat DAC
{
    Parametry.set_param=0; //vynulovani struktury na nastaveni parametru mereni
    Parametry.Napeti=0;
    Parametry.Cas_Nas=0;
    Parametry.start_mer=0;
    Parametry.default_pwm[0]=pwm[0];
    Parametry.default_pwm[1]=pwm[1];
    Parametry.default_pwm[2]=pwm[2];
    Parametry.default_pwm[3]=pwm[3];

}
