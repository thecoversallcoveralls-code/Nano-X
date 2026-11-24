#include <Wire.h>
#include <si5351.h>

Si5351 si5351;

const uint8_t W_CLK = 8;
const uint8_t FQ_UD = 9;
const uint8_t DATA  = 10;
const uint8_t RESET = 11;

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

void setup() {
  pinMode(W_CLK, OUTPUT);
  pinMode(FQ_UD, OUTPUT);
  pinMode(DATA, OUTPUT);
  pinMode(RESET, OUTPUT);

  ad9850Reset();

  si5351.init(SI5351_CRYSTAL_LOAD_8PF, 0, 0);
  si5351.set_freq(1000000ULL, SI5351_CLK0);
  ad9850SetFreq(55.0);
}

void loop() {
  const double bassNotes[] = {55.00, 65.41, 73.42, 82.41};
  for (uint8_t i = 0; i < 4; i++) {
    ad9850SetFreq(bassNotes[i]);
    delay(400);
  }
}
