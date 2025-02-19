#include <avr/io.h>
#include <stdint.h>
#include <util/delay.h>

int main(void) {
    DDRB |= (1 << PB4);

    while (1) {
        // Toggle the LED
        PORTB ^= (1 << PB4);   // Use XOR to toggle the pin
        _delay_ms(500);        // Delay for 500 milliseconds
    }
}