#include "mbed.h"
#include <cstdio>
#include "Grove_LCD_RGB_Backlight.h"


AnalogIn s_llum(A1);
InterruptIn s_pulsador(D4);
Grove_LCD_RGB_Backlight pantalla(D14, D15);
PwmOut led(D3);
PwmOut buzzer(D5);
#define DEADLINE 50
#define LUXMAX 544.5

float counts;
float lux,avg_count,avg_lux=0,mean_10s;
uint64_t now;
uint64_t counter;
int cont,cont_pantalla=0,contador_sleep=0;
bool calcul_on=false;

float calcul_lux(){
    counts = s_llum.read_u16();
    counts = (counts*3.3)/65535.0;
    lux = ((((3.3*500.0)*counts))/10.0);
    return  (100 - (lux * 100 / LUXMAX));
}
void calculate_mean(void){
    avg_count+= calcul_lux();
    cont++;
    if(contador_sleep>=200){
        printf("Last 10 seconds: %f and count: %d\n", avg_count/cont, cont);
        mean_10s=avg_count/cont;
        contador_sleep=0;
        s_pulsador.enable_irq();
        calcul_on=false;
        Kernel::attach_idle_hook(NULL);
    }
     
}

void pulsador_intr()
{
    
    s_pulsador.disable_irq();
    avg_count=0.0;
    cont=0;
    calcul_on=true;
    Kernel::attach_idle_hook(calculate_mean); 
    
}

void write_display(){
    cont_pantalla++;
    avg_lux +=calcul_lux(); 
    char buf[21];
    led.write(calcul_lux()/100);
    if (cont_pantalla >= 10){
        avg_lux=avg_lux/cont_pantalla;
    
        sprintf(buf, "Dark in %% %f", avg_lux);
        pantalla.locate(0, 0);
        pantalla.setRGB(rand()%255, rand()%255, rand()%255);
        pantalla.print(buf);
        cont_pantalla=0;
        avg_lux=0;
    
        sprintf(buf,"Mean of 10s:%f",mean_10s);
        pantalla.locate(0, 1);
        pantalla.print(buf);
        
    }
    
}
bool deadline(){
    if((Kernel::get_ms_count()-now) <= DEADLINE)
        return true;
    else{
        buzzer.write(0.25);
        return false;
    }
    
}

int main()
{       
    s_pulsador.rise(&pulsador_intr);
    buzzer.period_ms(1);
    while (true) {

        now = Kernel::get_ms_count();
        write_display(); //12ms
        
        if(calcul_on){  
            contador_sleep++;

         
            //1ms
        }   
            
        if(deadline()){
            uint64_t time_remaining = DEADLINE - (Kernel::get_ms_count()-now);
            ThisThread::sleep_for(time_remaining);
            //printf("deadline ms: %llu \r\n", time_remaining);
                
        }else{
            buzzer.write(0);  
        }
    }
        buzzer.write(0);
           
    
}