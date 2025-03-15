#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
int main () {
    DDRC=(1<<PD5);
    DDRD &=~(1<<PD4);
    PORTD |=(1<<4);
    while (1) {
        if (PIND&(1<<PD4)){
            PORTC |=(1<<PD5);
        }
        else{
            PORTC &=~(1<<PD5) ;
        }
            //_delay_ms (2000) ;
            //PORTC &=~(1<<5) ;
            //_delay_ms (2000) ;
    }
}

void LEDS(){

};