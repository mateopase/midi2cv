#include <Adafruit_MCP4728.h>
#include <MIDI.h>
#include <Wire.h>

/*
 * Midi poller. Reads in midi messages and pushes them into the
 * worker core's queue as quickly as possible.
 */

const int MIDI_CHANNEL = 13;
const int GATE_MASK = 0x100;
const int NOTE_MASK = 0xFF;

void pushNoteOn(byte channel, byte note, byte velocity) {
  rp2040.fifo.push(int(note) | GATE_MASK);
}

void pushNoteOff(byte channel, byte note, byte velocity) {
  rp2040.fifo.push(int(note));
}

MIDI_CREATE_INSTANCE(HardwareSerial, Serial1, MIDI);

void setup() {
  MIDI.begin(MIDI_CHANNEL); 
  MIDI.setHandleNoteOn(pushNoteOn);
  MIDI.setHandleNoteOff(pushNoteOff);
}

void loop() {
  MIDI.read();
}

/*
 * CV worker. Converts midi note on/off messages to cv/gate.
 * 
 * Midi messages gives us the number of semitones up from C0.
 * 
 * Converting a the semitones to a DAC value:
 * We need to convert the seminote value (0 - 127) to a DAC value (12bit)
 * We're targeting 1V/octave, so that's 1/12 V/semitone (12 semitones in an octave).
 * 
 * So, semitones * 1/12 V/semitone = volts per semitone.
 * 
 * Then, we need to convert the voltage back to a DAC value.
 * DAC voltage ranges from 0 (0V) to 4095 (VREF), where VREF is 4.096.
 * So, 4096 values / 4.096V = DAC value per volt.
 * 
 * Thus, dac_value(n) = (n semitones * 1v/12semitone) * 4096 values / VREF V
 * Finally, DAC value just needs to be an integer so we round it off.
 */

const float VREF = 4.096; // Reference voltage on DAV
const float CONVERSION_FACTOR = 4096 / (12 * VREF);
const int DAC_HIGH = round(4095 * (5 / 5.175)); // 5.175V measure off MC

Adafruit_MCP4728 dac;

int current_note = 0;

void setup1() {
  dac.begin();
  
  // Start gate at 0V
  dac.setChannelValue(MCP4728_CHANNEL_B, 0);
}

void loop1() {
  int msg = rp2040.fifo.pop();

  int note = msg & NOTE_MASK;
  int is_note_on = msg & GATE_MASK;

  if (is_note_on && note < 50) {
    current_note = note;
    
    dac.setChannelValue(MCP4728_CHANNEL_A, round(note * CONVERSION_FACTOR), MCP4728_VREF_INTERNAL, MCP4728_GAIN_2X);
    dac.setChannelValue(MCP4728_CHANNEL_B, DAC_HIGH);
  }
  else if (note == current_note) {
    dac.setChannelValue(MCP4728_CHANNEL_B, 0);
  }
}
