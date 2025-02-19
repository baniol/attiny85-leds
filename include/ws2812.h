#ifndef WS2812_H
#define WS2812_H

#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/cpufunc.h>
#include <avr/eeprom.h>

// Pin definitions
#define LED_PIN     PB0    // LED data on PB0
#define ENC_A       PB1    // Encoder A on PB1
#define ENC_B       PB2    // Encoder B on PB2
#define BTN_PIN     PB4    // Button on PB4 (internal pullup)

#define NUM_LEDS    12     // Number of LEDs in strip
#define MAX_BRIGHTNESS 255  // Maximum brightness value

// EEPROM addresses
#define EEPROM_BRIGHTNESS_ADDR 0
#define EEPROM_MODE_ADDR      1
#define EEPROM_MAGIC_ADDR     2
#define EEPROM_MAGIC_VALUE    0x42  // To check if EEPROM was initialized

// Direct port manipulation for speed
#define LED_PORT_REG    PORTB
#define LED_PIN_BIT     0

// Function prototypes
void save_settings(uint8_t brightness, uint8_t mode);
void load_settings(uint8_t *brightness, uint8_t *mode);
void send_bit_0(void);
void send_bit_1(void);
void send_byte(uint8_t byte);
void send_color(uint8_t r, uint8_t g, uint8_t b);
void fill_strip(uint8_t r, uint8_t g, uint8_t b);

#endif // WS2812_H 