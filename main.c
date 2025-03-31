#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

volatile uint8_t zustand_sekunds = 0b000000;
volatile uint8_t zustand_hours   = 0b00000;
volatile uint8_t zustand_minutes = 0b00000;
volatile uint8_t sleepflag       = 0;
volatile uint8_t brightness      = 128;


ISR(TIMER2_COMPA_vect){ //Iteraptor 
    if(zustand_sekunds == 0b111100){ // 0b111100
        Time_addierung();
        if (!sleepflag)
        {
            LEDS();
        }
        zustand_sekunds = 0b000000;
    }
    else {
        zustand_sekunds = (zustand_sekunds + 1);
    }
}

void LEDS(){
    // Ledsanschlatung Minuten
    PORTB = (PORTB & ~(0b111111 << PB0))
          | (((zustand_minutes >> 2) & 1) << PB0)
          | (((zustand_minutes >> 4) & 1) << PB1)
          | (((zustand_minutes >> 5) & 1) << PB2);

    PORTD = (PORTD & ~((1 << PD0) | (1 << PD1) | (1 << PD7)))
          | ((zustand_minutes & 0b00000011) << PD0)
          | ((zustand_minutes & 0b00001000) << (PD7 - 3));
    
    // Ledsanschlatung Hours
    PORTC = (PORTC & ~(0b11111 << PC1)) 
          | ((zustand_hours << PC1) & 0xFF);
};

// Minutes Logik
void Time_addierung(){
    if (zustand_minutes == 0b111100){
        zustand_minutes = 0b000000;
        check_hours();
    }
    else {
        zustand_minutes = (zustand_minutes + 1);
    }
};

void timer2_init() {
    ASSR |= (1 << AS2); //Asyncrone modus aktiviert
    TCCR2A |= (1 << WGM01);        //Clear Timer on Compare Match
    TCCR2B |= (1 << CS22) | (1 << CS20); // Setztprescale auf 1024
    OCR2A   = 255;                // zählt bis 31 OCR2A= (32768/1024)-1
    TIMSK2 |= (1 << OCIE2A);      // Aktiviert Iterapt 
    while (ASSR & ((1<<TCN2UB)|(1<<OCR2AUB)|(1<<TCR2AUB)|(1<<TCR2BUB))); // register syncronisieren
    sei();                        // Iterapts Global aktivieren
}


void check_hours(){
    if (zustand_hours == 0b10111){
        zustand_hours = 0b00000;
    }
    else {
        zustand_hours = (zustand_hours + 1);
    }
}

void start_test(){
    //Test Minutes Leds
    PORTD |= (0b1 << PD0);
    _delay_ms(3000);
    PORTD &= ~(0b1 << PD0);
    _delay_ms(3000);
    PORTD |= (0b1 << PD1);
    _delay_ms(3000);
    PORTD &= ~(0b1 << PD1);
    _delay_ms(3000);
    PORTB |= (0b1 << PB0);
    _delay_ms(3000);
    PORTB &= ~(0b1 << PB0);
    _delay_ms(3000);
    PORTD |= (0b1 << PD7);
    _delay_ms(3000);
    PORTD &= ~(0b1 << PD7);
    _delay_ms(3000);
    PORTB |= (0b1 << PB1);
    _delay_ms(3000);
    PORTB &= ~(0b1 << PB1);
    _delay_ms(3000);
    PORTB |= (0b1 << PB2);
    _delay_ms(3000);
    PORTB &= ~(0b1 << PB2);

    //Test Hours Leds
    for (int i = 0; i <= 5; i++){
        _delay_ms(3000);
        PORTC |= (0b1 << i);
        _delay_ms(3000);
        PORTC &= ~(0b1 << i);
    }
    confirmation();
    _delay_ms(1500);
    confirmation();
}

// startup blink
void confirmation(){
    //Ledsanschatung Minutes
    PORTB |= (PORTB & ~(0b111111 << PB0)
             | ((0b111111 >> 2 & 1) << PB0)
             | ((0b111111 >> 4 & 1) << PB1)
             | ((0b111111 >> 5 & 1) << PB2));
    PORTD |= (PORTD & ~((1 << PD0) | (1 << PD1) | (1 << PD7)))
          | ((0b111111 & 0b00000011) << PD0)
          | ((0b111111 & 0b00001000) << (PD7 - 3));
    //Ledsanschatung Hours
    PORTC |= (PORTC & ~(0b11111 << PC1)) 
          | ((0b11111 << PC1) & 0xFF);
    _delay_ms(3000);
    //Ledsausschatung Minutes
    PORTB &= (PORTB & ~(0b111111 << PB0));
    PORTD &= (PORTD & ~(0b11 << PD0) & ~(0b1 << PD7));
    //Ledsausschatung Hours
    PORTC &= (PORTC & ~(0b11111 << PC1));
}

// INT0 (PD2) 
ISR(INT0_vect){
    sleepflag = !sleepflag;
    if (!sleepflag){
        LEDS();
    }
    else {
        // Ports clear
        PORTB &= ~(0b111 << PB0);
        PORTD &= ~((1<<PD0)|(1<<PD1)|(1<<PD7));
        PORTC &= ~(0b11111 << PC1);
    }
}

ISR(INT1_vect){
    brightness += 51; // (~20%)
    if(brightness > 255) brightness = 0;
}

void software_pwm(){
    for(uint8_t i=0;i<255;i++){
        // MINUTEN (PORTB)
        if(i < brightness && (zustand_minutes & (1<<2))) PORTB |= (1<<PB0); else PORTB &= ~(1<<PB0);
        if(i < brightness && (zustand_minutes & (1<<4))) PORTB |= (1<<PB1); else PORTB &= ~(1<<PB1);
        if(i < brightness && (zustand_minutes & (1<<5))) PORTB |= (1<<PB2); else PORTB &= ~(1<<PB2);

        // MINUTEN (PORTD)
        if(i < brightness && (zustand_minutes & (1<<0))) PORTD |= (1<<PD0); else PORTD &= ~(1<<PD0);
        if(i < brightness && (zustand_minutes & (1<<1))) PORTD |= (1<<PD1); else PORTD &= ~(1<<PD1);
        if(i < brightness && (zustand_minutes & (1<<3))) PORTD |= (1<<PD7); else PORTD &= ~(1<<PD7);

        // STUNDEN (PORTC)
        if(i < brightness && (zustand_hours & (1<<0))) PORTC |= (1<<PC1); else PORTC &= ~(1<<PC1);
        if(i < brightness && (zustand_hours & (1<<1))) PORTC |= (1<<PC2); else PORTC &= ~(1<<PC2);
        if(i < brightness && (zustand_hours & (1<<2))) PORTC |= (1<<PC3); else PORTC &= ~(1<<PC3);
        if(i < brightness && (zustand_hours & (1<<3))) PORTC |= (1<<PC4); else PORTC &= ~(1<<PC4);
        if(i < brightness && (zustand_hours & (1<<4))) PORTC |= (1<<PC5); else PORTC &= ~(1<<PC5);

        _delay_us(30); // fast 130 Hz
    }
}




void go_sleep(){   // Sleep modi
    // Alle ports auf 0 setzen
    PORTB &= (PORTB & ~(0b111111 << PB0));
    PORTD &= (PORTD & ~(0b11 << PD0) & ~(0b1 << PD7));
    PORTC &= (PORTC & ~(0b11111 << PC1));

    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    sleep_enable();

    cli();
    sei();
    sleep_cpu();   // MCU sleep
    sleep_disable(); // Wieder aufwachen
}

void time_seting(){
    if (!(PIND & (1<<PD4))){
        check_hours();
        LEDS();
    }
    else{
        Time_addierung();
        LEDS();
    }
}

void ports(){ 
    DDRB |= (1<<PB0)|(1<<PB1)|(1<<PB2);
    DDRC |= (1<<PC1)|(1<<PC2)|(1<<PC3)|(1<<PC4)|(1<<PC5);
    DDRD |= (1<<PD0)|(1<<PD1)|(1<<PD7)|(1<<PD5)|(1<<PD6);

    DDRD &= ~((1<<PD2)|(1<<PD3)|(1<<PD4));// Konfigurieren Eingänge für Tasten
    PORTD |= (1<<PD2)|(1<<PD3)|(1<<PD4);

    EICRA |= (1<<ISC01)|(1<<ISC11); // Konfiguriert Triger auf falende Flanke
    EIMSK |= (1<<INT0)|(1<<INT1); //Maskiert Iterapt0 und 1
}

int main(){
    ports();
    start_test();
    timer2_init();
    sei();

    while(1){
        if (!(PIND & (1<<PD4))){
            _delay_ms(50);
            if (!(PIND & (1<<PD4))) { 
                _delay_ms(1000);
                time_seting();
            }

            while (!(PIND & (1<<PD4))){
            };
        }
        else if (sleepflag) go_sleep();
        else if (!sleepflag) software_pwm();
    }
}