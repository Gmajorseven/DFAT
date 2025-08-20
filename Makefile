BOARD?=arduino:avr:uno  # You can change the BOARD variable to match your board, e.g., arduino:avr:nano for Arduino Nano 
PORT?=$(shell ls /dev/ttyUSB*)

.PHONY: default lint all flash clean

default: all flash clean

all:
	arduino-cli compile --fqbn $(BOARD) ./
flash:
	arduino-cli upload -p $(PORT) --fqbn $(BOARD)
clean:
	rm -rf build
