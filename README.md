# Nano-X
This is a Kool under 40 dollar Synth Project. It uses a Arduino NANO 3.0 atmega328p, a SI5351A, and a AD9850. It also has 4 sliding cross-over style 10k pots. a rotary encoder #really just a fancy pot. and a headphone jack. future add ons of switches   to handle the hetrodyning capabilities ie odd/even and integer harmonics thru the 5351a


# Arduino Nano V3.0 + AD9850 + Si5351A MIDI Synth

A DIY synthesizer built around an **Arduino Nano V3.0 (`ATmega328P`)**, an **AD9850 DDS module** (for low-frequency bass and square wave generation), and an **Si5351A clock generator** (for upper harmonic voices), driven by an **Akai MPK88** MIDI controller[span_0](start_span)[span_0](end_span).

## Overview

- **AD9850 DDS Module:** Generates low-frequency bass down to `0 Hz` with `0.0291 Hz` frequency resolution and sine output (`ZOUT1` / `ZOUT2`) from its internal 10-bit DAC, plus a synchronized square wave (`QOUT1`)[span_1](start_span)[span_1](end_span).
- **Si5351A Clock Generator:** Provides higher-frequency clock/harmonic outputs (`CLK0`, `CLK1`) over I2C (`SDA` / `SCL`) to complement the AD9850 bass voice[span_2](start_span)[span_2](end_span).
- **Akai MPK88 Controller:** Sends note and pitch-bend messages into the Arduino Nano hardware UART (`D0 / RX0`) via a MIDI breakout board.

## Required Libraries (Arduino IDE 2.x)

Install both libraries in **Library Manager** before compiling:
1. **`Etherkit Si5351`**
2. **`MIDI Library`** (by FortySevenEffects)
author:Francois Best

## Wiring to Arduino Nano V3.0

### Si5351A (I2C)
- **`VCC`** -> `5V` (only if your breakout has an onboard `3.3V` regulator; otherwise `3.3V`)[span_3](start_span)[span_3](end_span)
- **`GND`** -> `GND`[span_4](start_span)[span_4](end_span)
- **`SDA`** -> `A4`[span_5](start_span)[span_5](end_span)
- **`SCL`** -> `A5`[span_6](start_span)[span_6](end_span)

### AD9850 (Serial Load Mode)
- **`VCC`** -> `5V`[span_7](start_span)[span_7](end_span)
- **`GND`** -> `GND`[span_8](start_span)[span_8](end_span)
- **`W_CLK`** -> `D8`[span_9](start_span)[span_9](end_span)
- **`FQ_UD`** -> `D9`[span_10](start_span)[span_10](end_span)
- **`DATA`** -> `D10`[span_11](start_span)[span_11](end_span)
- **`RESET`** -> `D11`[span_12](start_span)[span_12](end_span)
- **`ZOUT1` / `ZOUT2`** -> Filtered sine bass output[span_13](start_span)[span_13](end_span)
- **`QOUT1`** -> Synchronized square wave output[span_14](start_span)[span_14](end_span)
- Leave `D0–D7` on the AD9850 unconnected when using serial load mode[span_15](start_span)[span_15](end_span).

### MIDI & Gate Connections
- **MIDI Breakout `TX`** -> Nano `D0 (RX0)` (**disconnect before USB code uploads**, reconnect to play)
- **Optional Gate Out** -> Nano `D2` (`HIGH` while a note is held)

## Analog Bass Conditioning & Mixing

1. **DC Blocking Cap:** Place a `4.7 uF` to `10 uF` capacitor in series with `ZOUT1`, followed by a `47 kOhm` resistor to `GND` before a mixer or amplifier[span_16](start_span)[span_16](end_span).
2. **Analog Warmth / Growl:** Route AC-coupled `ZOUT1` through a `1 kOhm` resistor into two anti-parallel diodes (such as `1N4148` or `1N34A`) to `GND`, followed by a passive RC low-pass filter (`2.2 kOhm` + `100 nF`, ~`720 Hz` cutoff)[span_17](start_span)[span_17](end_span).
3. **Mixing with Si5351A:** Sum the conditioned AD9850 sine bass (via a `10 kOhm` resistor/fader) with `CLK0` / `CLK1` from the Si5351A (each via a DC-blocking capacitor and `22 kOhm–47 kOhm` resistor/fader)[span_18](start_span)[span_18](end_span).

## Flashing & First Test Checklist

1. Disconnect the MIDI breakout `TX` wire from Nano `D0 (RX0)`.
2. Select **Board: Arduino Nano**, **Processor: ATmega328P** (or **ATmega328P (Old Bootloader)** if needed), and your `/dev/ttyUSB*` port.
3. Verify and upload the sketch.
4. Reconnect the MIDI `TX` wire to `D0 (RX0)`, power the MPK88, verify its MIDI output routing is enabled, and bring up mixer faders starting with sine bass at low volume.
