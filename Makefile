# Project structure
SRC_DIR = src
INC_DIR = include
BUILD_DIR = build

# Microcontroller settings
MCU = attiny85
F_CPU = 8000000
PROGRAMMER = usbasp

# Compiler settings
CC = avr-gcc
OBJCOPY = avr-objcopy
SIZE = avr-size

# Source files
TARGET = ws2812_controller
SRC = $(wildcard $(SRC_DIR)/*.c)
OBJ = $(SRC:$(SRC_DIR)/%.c=$(BUILD_DIR)/%.o)

# Compiler flags
CFLAGS = -std=c99 -Wall -g -Os -mmcu=$(MCU) -DF_CPU=$(F_CPU)
CFLAGS += -I$(INC_DIR)

# Linker flags
LDFLAGS = -mmcu=$(MCU)

# Output files
HEX = $(BUILD_DIR)/$(TARGET).hex
ELF = $(BUILD_DIR)/$(TARGET).elf

# AVRDude settings
AVRDUDE = avrdude
AVRDUDE_FLAGS = -c $(PROGRAMMER) -p $(MCU)

# Fuse settings for 8MHz internal oscillator
# Low fuse: Enable 8MHz internal oscillator, no clock divide
# High fuse: Default settings
# Extended fuse: Default settings
LFUSE=0xE2
HFUSE=0xDF
EFUSE=0xFF

.PHONY: all clean flash fuses size mkdir

# Default target
all: mkdir $(HEX) size

# Create build directory
mkdir:
	@mkdir -p $(BUILD_DIR)

# Compile source files
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.c
	$(CC) $(CFLAGS) -c $< -o $@

# Link object files
$(ELF): $(OBJ)
	$(CC) $(LDFLAGS) $^ -o $@

# Create hex file
$(HEX): $(ELF)
	$(OBJCOPY) -j .text -j .data -O ihex $< $@

# Display size information
size: $(ELF)
	$(SIZE) --format=avr --mcu=$(MCU) $<

# Flash to device
flash: $(HEX)
	$(AVRDUDE) $(AVRDUDE_FLAGS) -U flash:w:$<:i

# Program fuses
fuses:
	$(AVRDUDE) -c $(PROGRAMMER) -p $(MCU) -U lfuse:w:$(LFUSE):m -U hfuse:w:$(HFUSE):m -U efuse:w:$(EFUSE):m

# Clean build files
clean:
	rm -rf $(BUILD_DIR)

