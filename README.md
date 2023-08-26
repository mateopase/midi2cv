# midi2cv
Code for an rp2040 powered midi to control voltage (1v/oct) converter.

The rp2040 is hooked up to a midi input circuit (using the required optocoupler and everything).
It's based on [this curcuit](https://www.notesandvolts.com/2015/02/midi-and-arduino-build-midi-input.html)
from Notes and Volts (which is really just the reference midi input circuit on a breadboard).
Since midi is a 5v protocol, but the rp2040 runs off a 3.3v logic level, a logic level shifter was used to make the two
talk.

The MCU was also hooked up to a 12bit 4-channel DAC, a [MCP4728](https://www.adafruit.com/product/4470).
The midi notes received from the input circuit were processed into a voltage, and sent to the DAC.

## The interesting bit
The rp2040 has two cores than can run independently. This is great for low-latency I/O applications like this, because
you can set up two independent workers, a reader and writer, and independently read and write as quickly as possible.

This Arduino sketch sets up core 0 in a loop to read midi notes and immediately push them into a FIFO for core 1 to process.

Core 1 is also a loop that pops stuff off the FIFO and converts them into a voltage and gate signal.

Cool!

# Acknowledgements
- The fantastic rp2040 Arduino core by Earle F. Philhower, III
    - https://github.com/earlephilhower/arduino-pico
- The Adafruit MCP4728 library
    - https://github.com/adafruit/Adafruit_MCP4728
- The  Arduino MIDI library by FortySevenEffects
    - https://github.com/FortySevenEffects/arduino_midi_library
