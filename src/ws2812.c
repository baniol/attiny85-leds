#include "ws2812.h"

// Function to save settings to EEPROM
void save_settings(uint8_t brightness, uint8_t mode) {
    eeprom_write_byte((uint8_t*)EEPROM_BRIGHTNESS_ADDR, brightness);
    eeprom_write_byte((uint8_t*)EEPROM_MODE_ADDR, mode);
    eeprom_write_byte((uint8_t*)EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VALUE);
}

// Function to load settings from EEPROM
void load_settings(uint8_t *brightness, uint8_t *mode) {
    // Check if EEPROM was initialized
    if (eeprom_read_byte((uint8_t*)EEPROM_MAGIC_ADDR) == EEPROM_MAGIC_VALUE) {
        *brightness = eeprom_read_byte((uint8_t*)EEPROM_BRIGHTNESS_ADDR);
        *mode = eeprom_read_byte((uint8_t*)EEPROM_MODE_ADDR);
    } else {
        // Default values for first run
        *brightness = 32;
        *mode = 0;
        // Save defaults
        save_settings(*brightness, *mode);
    }
}

// LED control functions
void send_bit_0(void) {
    uint8_t sreg = SREG;
    cli();
    
    LED_PORT_REG |= _BV(LED_PIN_BIT);   // HIGH
    LED_PORT_REG &= ~_BV(LED_PIN_BIT);  // Immediately LOW
    _NOP(); _NOP(); _NOP(); _NOP();     // Padding
    _NOP(); _NOP();
    
    SREG = sreg;
}

void send_bit_1(void) {
    uint8_t sreg = SREG;
    cli();
    
    LED_PORT_REG |= _BV(LED_PIN_BIT);   // HIGH
    _NOP(); _NOP(); _NOP(); _NOP();     // Stay HIGH longer
    _NOP();
    LED_PORT_REG &= ~_BV(LED_PIN_BIT);  // LOW
    _NOP();                             // Short LOW period
    
    SREG = sreg;
}

void send_byte(uint8_t byte) {
    for(uint8_t bit = 0x80; bit; bit >>= 1) {
        if (byte & bit) {
            send_bit_1();
        } else {
            send_bit_0();
        }
    }
}

void send_color(uint8_t r, uint8_t g, uint8_t b) {
    send_byte(g);  // WS2812B expects GRB order
    send_byte(r);
    send_byte(b);
}

void fill_strip(uint8_t r, uint8_t g, uint8_t b) {
    for(uint8_t i = 0; i < NUM_LEDS; i++) {
        send_color(r, g, b);
    }
} 