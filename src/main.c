#include "ws2812.h"

// Global variables for encoder and button state
volatile int16_t encoder_value = 0;
volatile uint8_t last_encoder_state = 0;

// Button state variables
static uint8_t button_state = 0;
static uint8_t last_button_state = 1;  // Pulled up by default
static uint16_t last_debounce_time = 0;
static uint16_t debounce_delay = 50;  // 50ms debounce time
static uint16_t time_counter = 0;
static uint16_t last_save_time = 0;
#define SAVE_DELAY 3000  // Wait 3 seconds before saving

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
        
        // Save settings if they changed and enough time has passed
        if ((encoder_value != last_saved_brightness || mode != last_saved_mode) &&
            (time_counter - last_save_time > SAVE_DELAY)) {
            save_settings(encoder_value, mode);
            last_saved_brightness = encoder_value;
            last_saved_mode = mode;
            last_save_time = time_counter;
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