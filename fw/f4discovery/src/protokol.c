#include "protokol.h"

extern volatile bool run;
extern struct Param_Mer Parametry;
extern int16_t nas_koef;

//naplni strukturu daty z retezce
void protokol(char *buf) //buf = 0 pro pøepsat  nastaveni, 1 pro spustit a 2 pro prepsat a spustit
{
    if(buf[0]==0)
    {
        Parametry.Cas_Nas=buf[3]*256+buf[4]; //uloz cas mereni
        Parametry.Napeti=buf[1]*256+buf[2];  //uloz merici napeti
        Parametry.set_param=1;  //nastav ze se maji zmenit parametry po domereni
        Parametry.start_mer=0;  // mereni se nema hend spustit
    }
    else if(buf[0]==1)
    {
        pwm[3] = 1; //vyresetuj mereni
        timer_set_oc_value(TIM5, TIM_OC4, pwm[3]); // nahraj do timeru 1
        run=true; //spust mereni
    }
    else if(buf[0]==2)
    {
        Parametry.Cas_Nas=buf[3]*256+buf[4]; //uloz do struktury cas
        Parametry.Napeti=buf[1]*256+buf[2]; //uloz do struktury merici napeti
        Parametry.set_param=1; //uloz priznak ze se maji zmenit parametry
        Parametry.start_mer=1; //uloz priznak ze se hned ma spustit nove mereni
        set_mer(); //zavolej fci, ktera prenese data ze struktury do patricnych periferii a promenych
    }
    else
    {
        run=false; //zastav mereni
    }

}

void set_mer(void) //prenese parametry ze struktury do promenych, a podle hodnoty ve strukture spusti enbo enspusti mereni
{
    set_napeti(Parametry.Napeti); //zavolej fci ktera nastavi vystupni anpeti
    nas_koef=Parametry.Cas_Nas; //uloz merici cas
    pwm[0]=Parametry.Cas_Nas*Parametry.default_pwm[0]; //do pole s parametry citacu uloz hodnoty
    pwm[1]=Parametry.Cas_Nas*Parametry.default_pwm[1];
    pwm[2]=Parametry.Cas_Nas*Parametry.default_pwm[2];
    pwm_init(); // volanim teto fce nahraju hodnoty do citacu
    Parametry.set_param=0; //zakazani dalsiho volani teto funkce kdyz se nezmenily parametry mereni
    if(Parametry.start_mer==1)
    {
        Parametry.start_mer=0; //zablokovani dalsiho spusteni mereno
        pwm[3] = 1; //reset citace od kereho se odviji spusteni ADC
        timer_set_oc_value(TIM5, TIM_OC4, pwm[3]);
        run=true;//spusteni mereni

    }


}

void init_protokol(void) //nastavi implicitni hodnoty struktury
{
    Parametry.set_param=0; //vynulovani struktury na nastaveni parametru mereni
    Parametry.Napeti=0;
    Parametry.Cas_Nas=1;
    Parametry.start_mer=0;
    Parametry.default_pwm[0]=pwm[0];
    Parametry.default_pwm[1]=pwm[1];
    Parametry.default_pwm[2]=pwm[2];
    Parametry.default_pwm[3]=pwm[3];

}
