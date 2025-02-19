# ATtiny85 WS2812B LED Controller

A firmware project for controlling WS2812B LED strips using an ATtiny85 microcontroller with rotary encoder input for brightness control and button input for color selection.

## Project Structure

```
.
├── include/            # Header files
│   └── ws2812.h       # LED and pin definitions
├── src/               # Source files
│   ├── main.c         # Main program logic
│   └── ws2812.c       # LED control implementation
├── build/             # Build artifacts (created during build)
├── Makefile          # Build system configuration
└── README.md         # This file
```

## Hardware Requirements

- ATtiny85 microcontroller
- WS2812B LED strip
- Rotary encoder with push button
- 5V power supply
- Programming hardware (e.g., USBasp)

## Pin Configuration

- PB0: LED data output
- PB1: Encoder A
- PB2: Encoder B
- PB4: Push button (with internal pullup)

## Building and Flashing

1. Build the project:
   ```
   make
   ```

2. Flash to ATtiny85:
   ```
   make flash
   ```

3. Set fuses (only needed once):
   ```
   make fuses
   ```

## Features

- Brightness control via rotary encoder
- Color selection via push button
- Settings saved to EEPROM
- Debounced button input
- Efficient WS2812B control
# attiny85-leds
