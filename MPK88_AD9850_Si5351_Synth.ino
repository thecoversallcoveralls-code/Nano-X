#include <Wire.h>
#include <si5351.h>
#include <MIDI.h>

Si5351 si5351;
MIDI_CREATE_DEFAULT_INSTANCE();

// AD9850 Pins (Serial Load Mode)
const uint8_t W_CLK = 8;
const uint8_t FQ_UD = 9;
const uint8_t DATA  = 10;
const uint8_t RESET = 11;

// Optional Gate Pin (HIGH while any key is held)
const uint8_t GATE_PIN = 2;

// Monophonic Note Stack (Last-Note Priority)
const uint8_t MAX_NOTES = 16;
uint8_t noteStack[MAX_NOTES];
uint8_t stackCount = 0;

int8_t currentNote = -1;
double pitchBendSemis = 0.0;
const double BEND_RANGE_SEMITONES = 2.0;

void pulsePin(uint8_t pin) {
  digitalWrite(pin, HIGH);
  digitalWrite(pin, LOW);
}

void ad9850Reset() {
  pulsePin(RESET);
  pulsePin(W_CLK);
  pulsePin(FQ_UD);
}

void ad9850SetFreq(double freqHz) {
  if (freqHz < 0.0) freqHz = 0.0;
  uint32_t tuningWord = (uint32_t)((freqHz * 4294967296.0) / 125000000.0);
  for (uint8_t i = 0; i < 32; i++) {
    digitalWrite(DATA, (tuningWord >> i) & 0x01);
    pulsePin(W_CLK);
  }
  for (uint8_t i = 0; i < 8; i++) {
    digitalWrite(DATA, LOW);
    pulsePin(W_CLK);
  }
  pulsePin(FQ_UD);
}

double midiNoteToFreq(uint8_t note, double bendSemitones) {
  return 440.0 * pow(2.0, ((double)note - 69.0 + bendSemitones) / 12.0);
}

void updateOscillators() {
  if (stackCount == 0) {
    currentNote = -1;
    digitalWrite(GATE_PIN, LOW);
    ad9850SetFreq(0.0);
    si5351.output_enable(SI5351_CLK0, 0);
    si5351.output_enable(SI5351_CLK1, 0);
    return;
  }

  currentNote = noteStack[stackCount - 1];
  digitalWrite(GATE_PIN, HIGH);

  double baseFreq = midiNoteToFreq(currentNote, pitchBendSemis);

  // 1. AD9850 plays the fundamental bass frequency (0 Hz - 4186+ Hz across 88 keys)
  ad9850SetFreq(baseFreq);

  // 2. Si5351A plays upper harmonics (in 0.01 Hz units)
  uint64_t harmonic1 = (uint64_t)(baseFreq * 16.0 * 100ULL); // +4 octaves
  uint64_t harmonic2 = (uint64_t)(baseFreq * 24.0 * 100ULL); // +4 octaves + 5th

  if (harmonic1 >= 300000ULL && harmonic1 <= 15000000000ULL) {
    si5351.set_freq(harmonic1, SI5351_CLK0);
    si5351.output_enable(SI5351_CLK0, 1);
  } else {
    si5351.output_enable(SI5351_CLK0, 0);
  }

  if (harmonic2 >= 300000ULL && harmonic2 <= 15000000000ULL) {
    si5351.set_freq(harmonic2, SI5351_CLK1);
    si5351.output_enable(SI5351_CLK1, 1);
  } else {
    si5351.output_enable(SI5351_CLK1, 0);
  }
}

void pushNote(uint8_t note) {
  for (uint8_t i = 0; i < stackCount; i++) {
    if (noteStack[i] == note) {
      for (uint8_t j = i; j < stackCount - 1; j++) {
        noteStack[j] = noteStack[j + 1];
      }
      noteStack[stackCount - 1] = note;
      return;
    }
  }
  if (stackCount < MAX_NOTES) {
    noteStack[stackCount++] = note;
  } else {
    for (uint8_t i = 0; i < MAX_NOTES - 1; i++) {
      noteStack[i] = noteStack[i + 1];
    }
    noteStack[MAX_NOTES - 1] = note;
  }
}

void removeNote(uint8_t note) {
  for (uint8_t i = 0; i < stackCount; i++) {
    if (noteStack[i] == note) {
      for (uint8_t j = i; j < stackCount - 1; j++) {
        noteStack[j] = noteStack[j + 1];
      }
      stackCount--;
      return;
    }
  }
}

void handleNoteOn(byte channel, byte note, byte velocity) {
  if (velocity == 0) {
    removeNote(note);
  } else {
    pushNote(note);
  }
  updateOscillators();
}

void handleNoteOff(byte channel, byte note, byte velocity) {
  removeNote(note);
  updateOscillators();
}

void handlePitchBend(byte channel, int bend) {
  pitchBendSemis = ((double)bend / 8192.0) * BEND_RANGE_SEMITONES;
  if (stackCount > 0) {
    updateOscillators();
  }
}

void setup() {
  pinMode(W_CLK, OUTPUT);
  pinMode(FQ_UD, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(RESET, OUTPUT);
  pinMode(GATE_PIN, OUTPUT);
  digitalWrite(GATE_PIN, LOW);

  ad9850Reset();
  ad9850SetFreq(0.0);

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351.output_enable(SI5351_CLK0, 0);
  si5351.output_enable(SI5351_CLK1, 0);

  MIDI.setHandleNoteOn(handleNoteOn);
  MIDI.setHandleNoteOff(handleNoteOff);
  MIDI.setHandlePitchBend(handlePitchBend);
  MIDI.begin(MIDI_CHANNEL_OMNI);
}

void loop() {
  MIDI.read();
}
