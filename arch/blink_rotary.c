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

// Global variables for encoder and button state
volatile int16_t encoder_value = 0;
volatile uint8_t last_encoder_state = 0;

// Button state variables
static uint8_t button_state = 0;
static uint8_t last_button_state = 1;  // Pulled up by default
static uint16_t last_debounce_time = 0;
static uint16_t debounce_delay = 50;  // 50ms debounce time
static uint16_t time_counter = 0;

// Function to save settings to EEPROM
static void save_settings(uint8_t brightness, uint8_t mode) {
    eeprom_write_byte((uint8_t*)EEPROM_BRIGHTNESS_ADDR, brightness);
    eeprom_write_byte((uint8_t*)EEPROM_MODE_ADDR, mode);
    eeprom_write_byte((uint8_t*)EEPROM_MAGIC_ADDR, EEPROM_MAGIC_VALUE);
}

// Function to load settings from EEPROM
static void load_settings(uint8_t *brightness, uint8_t *mode) {
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

// LED control functions remain the same
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

// Pin change interrupt for encoder only
ISR(PCINT0_vect) {
    // Read current encoder state
    uint8_t encoder_state = (PINB & (_BV(ENC_A) | _BV(ENC_B))) >> 1;
    
    // Detect rotation using gray code
    if (encoder_state != last_encoder_state) {
        if ((last_encoder_state == 0b00 && encoder_state == 0b01) ||
            (last_encoder_state == 0b01 && encoder_state == 0b11) ||
            (last_encoder_state == 0b11 && encoder_state == 0b10) ||
            (last_encoder_state == 0b10 && encoder_state == 0b00)) {
            if (encoder_value < MAX_BRIGHTNESS) {
                encoder_value++;
            }
        } else if (
            (last_encoder_state == 0b00 && encoder_state == 0b10) ||
            (last_encoder_state == 0b10 && encoder_state == 0b11) ||
            (last_encoder_state == 0b11 && encoder_state == 0b01) ||
            (last_encoder_state == 0b01 && encoder_state == 0b00)) {
            if (encoder_value > 0) {
                encoder_value--;
            }
        }
        last_encoder_state = encoder_state;
    }
}

int main(void) {
    // Configure pins
    DDRB = _BV(LED_PIN);
    PORTB = _BV(ENC_A) | _BV(ENC_B) | _BV(BTN_PIN);
    
    // Configure pin change interrupt for encoder only
    GIMSK |= _BV(PCIE);
    PCMSK |= _BV(PCINT1) | _BV(PCINT2);  // Only encoder pins
    
    sei();
    
    // Load saved settings
    uint8_t mode;
    uint8_t saved_brightness;
    load_settings(&saved_brightness, &mode);
    encoder_value = saved_brightness;
    
    uint8_t last_saved_brightness = saved_brightness;
    uint8_t last_saved_mode = mode;
    
    while(1) {
        // Reset signal for WS2812B
        LED_PORT_REG &= ~_BV(LED_PIN_BIT);
        _delay_ms(1);
        
        // Increment time counter (approximately every millisecond)
        time_counter++;
        
        // Read the button state
        uint8_t reading = (PINB & _BV(BTN_PIN)) ? 1 : 0;
        
        // If the button state changed, reset the debounce timer
        if (reading != last_button_state) {
            last_debounce_time = time_counter;
        }
        
        // Check if enough time has passed since the last state change
        if ((time_counter - last_debounce_time) > debounce_delay) {
            // If the button state has changed:
            if (reading != button_state) {
                button_state = reading;
                
                // Only trigger on button press (transition from high to low)
                if (button_state == 0) {
                    mode = (mode + 1) % 3;
                }
            }
        }
        
        last_button_state = reading;
        
        // Ensure encoder_value stays in valid range
        if (encoder_value < 0) encoder_value = 0;
        if (encoder_value > MAX_BRIGHTNESS) encoder_value = MAX_BRIGHTNESS;
        
        // Save settings if they changed
        if (encoder_value != last_saved_brightness || mode != last_saved_mode) {
            save_settings(encoder_value, mode);
            last_saved_brightness = encoder_value;
            last_saved_mode = mode;
        }
        
        // Display pattern based on mode
        switch(mode) {
            case 0:
                fill_strip(encoder_value, 0, 0);
                break;
            case 1:
                fill_strip(0, encoder_value, 0);
                break;
            case 2:
                fill_strip(0, 0, encoder_value);
                break;
        }
        
        _delay_ms(1);  // Reduced delay for better button responsiveness
    }
    
    return 0;
}
