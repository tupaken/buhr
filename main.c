#define F_CPU 1000000UL
#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>

// Глобальные переменные
volatile uint8_t zustand_sekunds = 0b000000;
volatile uint8_t zustand_hours   = 0b00000;
// СДЕЛАЙ
volatile uint8_t zustand_minutes = 0b00000;
volatile uint8_t sleepflag       = 0;

// Прерывание таймера 2 (CTC) — считает секунды
ISR(TIMER2_COMPA_vect){
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

// Отрисовка на светодиодах
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

// Логика прибавления минут
void Time_addierung(){
    if (zustand_minutes == 0b111100){
        zustand_minutes = 0b000000;
        check_hours();
    }
    else {
        zustand_minutes = (zustand_minutes + 1);
    }
};

// Инициализация таймера 2 (асинхрон, кварц)
void timer2_init() {
    ASSR |= (1 << AS2);
    TCCR2A |= (1 << WGM01);        // Режим CTC (сброс по совпадению)
    TCCR2B |= (1 << CS22) | (1 << CS20); // Предделитель 1024
    OCR2A   = 255;                // Подстроено под ~1 сек при 1 МГц
    TIMSK2 |= (1 << OCIE2A);      // Включаем прерывание по совпадению
    sei();                        // Включаем глобальные прерывания
}

// Проверка часов — прибавление часа
void check_hours(){
    if (zustand_hours == 0b10111){
        zustand_hours = 0b00000;
    }
    else {
        zustand_hours = (zustand_hours + 1);
    }
}

// Тестовое включение светодиодов (Minutes/Hours)
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

// «Подтверждение» — массовое вкл/выкл
void confirmation(){
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
    PORTB &= (PORTB & ~(0b111111 << PB0));
    PORTD &= (PORTD & ~(0b11 << PD0) & ~(0b1 << PD7));
    //Ledsanschatung Hours
    PORTC &= (PORTC & ~(0b11111 << PC1));
}

// Прерывание INT0 (PD2) — переключение флага сна
ISR(INT0_vect){
    sleepflag = !sleepflag;
    if (!sleepflag){
        LEDS();
    }
}

// Функция ухода в сон
void go_sleep(){
    // Вы обнуляете здесь все порты,
    PORTB &= (PORTB & ~(0b111111 << PB0));
    PORTD &= (PORTD & ~(0b11 << PD0) & ~(0b1 << PD7));
    //Ledsanschatung Hours
    PORTC &= (PORTC & ~(0b11111 << PC1));

    set_sleep_mode(SLEEP_MODE_PWR_SAVE);
    sleep_enable();

    cli();
    sei();
    sleep_cpu();   // MCU засыпает
    sleep_disable(); // Возврат после пробуждения
}

int main (){
    // --- Настройка портов ---
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);    // Minutes 110100
    DDRC |= (1 << PC5) | (1 << PC4) | (1 << PC3)
          | (1 << PC2) | (1 << PC1);                // Hours

    // Button Sleep на PD2 (INT0)
    DDRD &= ~(1 << PD2); 
    PORTD |= (1 << PD2);
    EICRA |= (1 << ISC01);
    EICRA &= ~(1 << ISC00);
    EIMSK |= (1 << INT0);

    // Taste Mitte на PD3
    DDRD &= ~(1 << PD3);
    PORTD |= (1 << PD3);

    // Ground Minuten PD6
    DDRD |= (1 << PD6);
    PORTD &= ~(1 << PD6);

    // Ground Hours PD5
    DDRD |= (1 << PD5);
    PORTD &= ~(1 << PD5);

    // Minutes: PD0, PD1, PD7 (001011)
    DDRD |= (1 << PD0) | (1 << PD1) | (1 << PD7);

    // Прерывание INT1 (для Taste Mitte)
    EIMSK |= (1 << INT1);  // Iterapt Mask
    EICRA |= (1 << ISC11); // Прерывание по спаду
    EICRA &= ~(1 << ISC10);

    // Button first PD4
    DDRD &= ~(1 << PD4);
    PORTD |= (1 << PD4);

    // Запуск теста
    start_test();
    sei();

    // Запуск таймера
    timer2_init();

    while (1) {
        // Кнопка PD4 → Time_addierung
        if (!(PIND & (1 << PD4))){
            _delay_ms(50);  // debounce 50 ms
            if (!(PIND & (1 << PD4))) { 
                Time_addierung();
                LEDS();
            }
            while (!(PIND & (1 << PD4))){  // ждем, пока кнопку отпустят
                _delay_ms(50);  // debounce отпускания
            }
        }
        // Если sleepflag = 1 → уходим в сон
        else if (sleepflag) {
            go_sleep();
            //sleepflag = 0;
        }
        asm("nop");
    }
}
