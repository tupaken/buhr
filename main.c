    #define F_CPU 1000000UL
    #include <avr/io.h>
    #include <util/delay.h>

    uint8_t zustand_hours=0b00000;
    uint8_t zustand_minutes=0b000000;

    int main () {
        DDRB=(1<<PB0)|(1<<PB1)|(1<<PB2);
        DDRC=(1<<PC5)|(1<<PC4)|(1<<PC3)|(1<<PC2)|(1<<PC1);
        DDRD &=~(1<<PD4);
        DDRD =(1<<PD0)|(1<<PD1)|(1<<PD7);
        PORTD |=(1<<PD4);
        while (1) {
            if (!(PIND&(1<<PD4))){
                _delay_ms(50);  // debounce 50 ms
                if (!(PIND & (1<<PD4))) { 
                    Time_addierung();
                    LEDS();
                }
                while (!(PIND & (1<<PD4))){  // ждем, пока кнопку отпустят
                _delay_ms(50);  // debounce отпускания
            }
            }
                //_delay_ms (2000) ;
                //PORTC &=~(1<<5) ;
                //_delay_ms (2000) ;
        }
    }

    void LEDS(){
        //Ledsanschlatung Minuten
        PORTB = (PORTB & ~(0b111111<<PB0)|((zustand_minutes>>2 &1)<<PB0)|
        ((zustand_minutes>>4 &1)<<PB1)|((zustand_minutes>>5 &1)<<PB2));
        PORTD = (PORTD & ~((1 << PD0) | (1 << PD1) | (1 << PD7))) | 
        ((zustand_minutes & 0b00000011) << PD0) |        
        ((zustand_minutes & 0b00001000) << (PD7 - 3));
        //Ledsanschatung Hours   
        PORTC = (PORTC & ~(0b11111 << PC1)) | (zustand_hours << PC1);
    };

    void Time_addierung(){
        if (zustand_minutes==0b111100){
            zustand_minutes=0b000000;
            zustand_hours =(zustand_hours+1);
        }
        else{
            zustand_minutes = (zustand_minutes + 1);
        }
    };