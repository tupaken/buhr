#define F_CPU 1000000UL // 1 MHz quartz clock

#include <avr/io.h>
#include <avr/interrupt.h>
#include <util/delay.h>
#include <stdint.h>

// Global time variables (24-hour format)
volatile uint8_t seconds = 0;
volatile uint8_t minutes = 0;
volatile uint8_t hours = 0; // 0 represents midnight

// Timer1 Compare Match Interrupt Service Routine (~1 second interval)
ISR(TIMER1_COMPA_vect)
{
    seconds++;
    if (seconds >= 60)
    {
        seconds = 0;
        minutes++;
        if (minutes >= 60)
        {
            minutes = 0;
            hours++;
            if (hours >= 24)
                hours = 0;
        }
    }
}

// Initialize Timer1 in CTC mode to generate ~1-second interrupts.
void init_timer1(void)
{
    // Configure Timer1 for CTC mode.
    TCCR1B |= (1 << WGM12);
    // With F_CPU = 1MHz and prescaler = 1024:
    // 1 sec ≈ 1,000,000 / 1024 ≈ 976 counts, so set OCR1A to 975.
    OCR1A = 975;

    // Enable Timer1 Compare Match A interrupt.
    TIMSK1 |= (1 << OCIE1A);

    // Start Timer1 with a 1024 prescaler.
    TCCR1B |= (1 << CS12) | (1 << CS10);
}

// Initialize I/O ports for the LEDs and buttons.
void init_ports(void)
{
    // --- Hours LEDs on PORTC (PC5 to PC1)
    // Set PC1, PC2, PC3, PC4, PC5 as outputs.
    DDRC |= 0x3E; // 0x3E = 0b00111110 (bits 1-5)

    // --- Minutes LEDs:
    // Minutes anodes are on:
    //   PORTD: PD0, PD1, PD7
    DDRD |= (1 << PD0) | (1 << PD1) | (1 << PD7);
    //   PORTB: PB0, PB1, PB2
    DDRB |= (1 << PB0) | (1 << PB1) | (1 << PB2);

    // --- Common Cathodes ("minus") for both LED groups:
    // Hours common cathode: PD6, Minutes common cathode: PD5.
    DDRD |= (1 << PD6) | (1 << PD5);
    // Drive common cathodes low.
    PORTD &= ~((1 << PD6) | (1 << PD5));

    // --- Setup Buttons ---
    // Configure PD2, PD3, and PD4 as inputs with internal pull-ups.
    DDRD &= ~((1 << PD2) | (1 << PD3) | (1 << PD4));
    PORTD |= (1 << PD2) | (1 << PD3) | (1 << PD4);
}

// LED Test Routine: Sequentially light each LED so you can verify the wiring.
void led_test(void)
{
    uint8_t i;

    // Test Hours LEDs on PORTC: PC5, PC4, PC3, PC2, PC1.
    for (i = 1; i <= 5; i++)
    {
        PORTC |= (1 << i); // Turn on LED at PC[i].
        _delay_ms(300);
        PORTC &= ~(1 << i); // Turn it off.
        _delay_ms(100);
    }

    // Test Minutes LEDs on PORTD: PD0, PD1, PD7.
    uint8_t pd_pins[3] = {PD0, PD1, PD7};
    for (i = 0; i < 3; i++)
    {
        PORTD |= (1 << pd_pins[i]);
        _delay_ms(300);
        PORTD &= ~(1 << pd_pins[i]);
        _delay_ms(100);
    }

    // Test Minutes LEDs on PORTB: PB0, PB1, PB2.
    uint8_t pb_pins[3] = {PB0, PB1, PB2};
    for (i = 0; i < 3; i++)
    {
        PORTB |= (1 << pb_pins[i]);
        _delay_ms(300);
        PORTB &= ~(1 << pb_pins[i]);
        _delay_ms(100);
    }
}

// Update the binary clock display.
void update_display(void)
{
    uint8_t minute_bits;
    uint8_t new_PORTD, new_PORTB;

    // --- Hours:
    // Hours value (0–23) is 5 bits. Map bit0->PC1, bit1->PC2, ... bit4->PC5.
    PORTC = (PORTC & ~0x3E) | ((hours & 0x1F) << 1);

    // --- Minutes:
    // Mapping for minutes (6 bits):
    //   Bit 0 → PD0
    //   Bit 1 → PD1
    //   Bit 2 → PB0
    //   Bit 3 → PD7
    //   Bit 4 → PB1
    //   Bit 5 → PB2
    minute_bits = minutes & 0x3F;

    new_PORTD = (((minute_bits >> 0) & 0x01) << PD0) |
                (((minute_bits >> 1) & 0x01) << PD1) |
                (((minute_bits >> 3) & 0x01) << PD7);
    PORTD = (PORTD & ~((1 << PD0) | (1 << PD1) | (1 << PD7))) | new_PORTD;

    new_PORTB = (((minute_bits >> 2) & 0x01) << PB0) |
                (((minute_bits >> 4) & 0x01) << PB1) |
                (((minute_bits >> 5) & 0x01) << PB2);
    PORTB = (PORTB & ~0x07) | new_PORTB;
}

int main(void)
{
    // Initialize ports and run the LED test.
    init_ports();
    led_test();

    // ---- Time Setup Mode ----
    // In setup mode the user can adjust the time using:
    //   - PD2: Increment hours.
    //   - PD3: Increment minutes.
    // The current time is shown on the display.
    // When the confirm button (PD4, active low) is pressed, setup mode ends.

    // Use a simple polling loop.
    while (PIND & (1 << PD4)) // Continue until PD4 is pressed (goes low)
    {
        // Check Hours Button (PD2)
        if (!(PIND & (1 << PD2))) // Button pressed (active low)
        {
            _delay_ms(50); // debounce delay
            // Wait for button release.
            while (!(PIND & (1 << PD2)))
            {
            }
            hours++;
            if (hours >= 24)
                hours = 0;
            update_display();
        }

        // Check Minutes Button (PD3)
        if (!(PIND & (1 << PD3))) // Button pressed (active low)
        {
            _delay_ms(50); // debounce delay
            // Wait for button release.
            while (!(PIND & (1 << PD3)))
            {
            }
            minutes++;
            if (minutes >= 60)
                minutes = 0;
            update_display();
        }

        update_display();
        _delay_ms(100); // Loop delay for responsiveness.
    }

    // When PD4 is pressed, exit setup mode.
    // Optionally wait for PD4 to be released.
    while (!(PIND & (1 << PD4)))
    {
    }

    // ---- Normal Operation: Start Timer and enable interrupts ----
    init_timer1();
    sei(); // Enable global interrupts

    // Main loop: update display continuously.
    while (1)
    {
        update_display();
        _delay_ms(200);
    }

    return 0;
}
