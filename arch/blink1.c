#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>

#define LED_PIN PB0      // Data pin for WS2812B
#define NUM_LEDS 12      // Number of LEDs in the strip

// Direct port manipulation for speed
#define LED_PORT        PORTB
#define LED_PORT_REG    PORTB
#define LED_PIN_BIT     0

static inline void send_bit_0(void) {
    uint8_t sreg = SREG;
    cli();
    
    LED_PORT_REG |= _BV(LED_PIN_BIT);   // HIGH
    LED_PORT_REG &= ~_BV(LED_PIN_BIT);  // Immediately LOW
    _NOP(); _NOP(); _NOP(); _NOP();     // Padding
    _NOP(); _NOP();
    
    SREG = sreg;
}

static inline void send_bit_1(void) {
    uint8_t sreg = SREG;
    cli();
    
    LED_PORT_REG |= _BV(LED_PIN_BIT);   // HIGH
    _NOP(); _NOP(); _NOP(); _NOP();     // Stay HIGH longer
    _NOP();
    LED_PORT_REG &= ~_BV(LED_PIN_BIT);  // LOW
    _NOP();                             // Short LOW period
    
    SREG = sreg;
}

static inline void send_byte(uint8_t byte) {
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
        if (byte & bit) {
            send_bit_1();
        } else {
            send_bit_0();
        }
    }
}

static inline void send_color(uint8_t r, uint8_t g, uint8_t b) {
    send_byte(g);  // WS2812B expects GRB order
    send_byte(r);
    send_byte(b);
}

// Send a color to all LEDs
static inline void fill_strip(uint8_t r, uint8_t g, uint8_t b) {
    for(uint8_t i = 0; i < NUM_LEDS; i++) {
        send_color(r, g, b);
    }
}

int main(void) {
    // Configure LED pin as output
    DDRB |= _BV(LED_PIN_BIT);
    LED_PORT_REG &= ~_BV(LED_PIN_BIT);  // Start LOW
    
    // Power-up delay
    _delay_ms(100);
    
    uint8_t step = 0;
    
    while(1) {
        // Reset signal
        LED_PORT_REG &= ~_BV(LED_PIN_BIT);
        _delay_ms(1);  // Reset pulse
        
        // Show different colors in sequence
        switch(step) {
            case 0:  // Red
                fill_strip(255, 0, 0);
                break;
            case 1:  // Green
                fill_strip(0, 255, 0);
                break;
            case 2:  // Blue
                fill_strip(0, 0, 255);
                break;
            case 3:  // Purple
                fill_strip(255, 0, 255);
                break;
            case 4:  // Yellow
                fill_strip(255, 255, 0);
                break;
            case 5:  // Cyan
                fill_strip(0, 255, 255);
                break;
            default:
                step = 0;
                continue;
        }
        
        step = (step + 1) % 6;  // Cycle through colors
        _delay_ms(500);         // Show each color for 0.5 seconds
    }
    
    return 0;
}
