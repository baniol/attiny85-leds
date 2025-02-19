#include <Adafruit_NeoPixel.h>

// Pin Definitions
#define LED_PIN      4   // PB0 for NeoPixel
#define ENCODER_A    1   // PB1 for Rotary Encoder A
#define ENCODER_B    2   // PB2 for Rotary Encoder B
#define NUM_LEDS    8   // Number of LEDs

// NeoPixel Configuration
Adafruit_NeoPixel strip(NUM_LEDS, LED_PIN, NEO_GRB + NEO_KHZ800);

// Variables for rotary encoder
volatile int encoderPosition = 0;
int lastEncoderState = 0;

// Color Variables
int hue = 0;  // Hue value (0-255 for Adafruit_NeoPixel)

void setup() {
  // Initialize NeoPixel
  strip.begin();
  strip.show(); // Turn all LEDs off initially

  // Rotary Encoder Setup
  pinMode(ENCODER_A, INPUT_PULLUP);
  pinMode(ENCODER_B, INPUT_PULLUP);

  // Initialize rotary encoder state
  lastEncoderState = digitalRead(ENCODER_A);
}

void loop() {
  // Rotary Encoder Logic
  int currentEncoderState = digitalRead(ENCODER_A);
  if (currentEncoderState != lastEncoderState) {
    if (digitalRead(ENCODER_B) != currentEncoderState) {
      encoderPosition++;
    } else {
      encoderPosition--;
    }
    lastEncoderState = currentEncoderState;

    // Adjust hue based on encoder position
    hue = (encoderPosition * 5) % 256;  // Map position to 0-255 range
    if (hue < 0) hue += 256;           // Handle negative wrapping

    // Update LED colors
    setAllLEDsColor(hue);
  }
}

void setAllLEDsColor(int hue) {
  // Convert hue to RGB and set all LEDs
  for (int i = 0; i < NUM_LEDS; i++) {
    strip.setPixelColor(i, strip.ColorHSV(hue * 65536L / 256, 255, 255));
  }
  strip.show();
}

