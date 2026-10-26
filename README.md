# Biomorphic Robotic Palm

A glove controls five servo-driven fingers through two Arduino Uno boards and a paired transparent UART radio link. Flex sensors measure each finger's bend; the receiver reproduces those five flexion values. The mechanical prototype uses PVC, plastic, string tendons, and a glove.

**Run the real calibration and protocol code on a laptop with `make test demo`, then compile both Uno sketches with `make compile`.** Physical radio, servo, and mechanical operation still requires hardware acceptance. A simulation does not establish a working physical build.

The original claim to reproduce “all the gestures of a human hand” was too broad: five flex sensors and five servos measure/control finger flexion, not wrist movement, finger spreading, or independent motion of every joint. [Original README](docs/README-original.md), [report](Report/), [connection diagrams](Connection%20Diagrams/), and [historical video](https://www.youtube.com/watch?v=sM6hTDGu5zk) remain available. Historical `.pde` listings are retained for provenance; use `firmware/` for new builds.

## Quick start without hardware

Install a C++11 compiler (`c++`, GCC, or Clang) and Make, then:

```sh
git clone https://github.com/sragsree/Robotic-Palm-with-Arduino.git
cd Robotic-Palm-with-Arduino
make test demo
```

The demo passes sample inputs through **the same C++ library used by the firmware**:

```text
sample 0 | servo targets: 10 10 10 10 10
sample 1 | servo targets: 10 50 90 130 170
sample 2 | servo targets: 170 170 170 170 170
radio silent 500 ms | servo targets: 10 10 10 10 10
```

These are calculated setpoints, not measured servo positions. Firmware moves toward targets by at most three degrees per 20 ms update.

## Compile and upload

Install [Arduino CLI](https://arduino.github.io/arduino-cli/1.3/installation/) (verified with 1.3.1), then:

```sh
arduino-cli core update-index
arduino-cli core install arduino:avr@1.8.6
arduino-cli lib install Servo@1.2.2
make compile
arduino-cli board list
```

Connect one board at a time. Replace the port below with that board's port from `board list`. Disconnect the radio from pins 0/1 while uploading.

```sh
arduino-cli upload --fqbn arduino:avr:uno --port /dev/ttyACM0 firmware/PalmTransmitter
# Connect the second board and select its port before this command.
arduino-cli upload --fqbn arduino:avr:uno --port /dev/ttyACM0 firmware/PalmReceiver
```

On macOS the port typically begins `/dev/cu.usbmodem`; Windows uses `COM` ports. With Arduino IDE, install `libraries/PalmControl` in the sketchbook's libraries directory and install Servo 1.2.2 from Library Manager, then open each matching `.ino` folder.

## Hardware and wiring

Required: two Uno R3 boards, five flex sensors with five voltage-divider resistors, five positional hobby servos, two compatible XBee/ZigBee radios with suitable regulated/level-shifting adapters, a glove, PVC/plastic structure, tendons, and supplies rated for the actual load. The original “two 5V batteries” list was not a complete power specification.

| Signal | Transmitter | Receiver |
|---|---|---|
| Thumb/index/middle/ring/little sensor dividers | A0/A1/A2/A3/A4 | — |
| Thumb/index/middle/ring/little servo signals | — | D2/D3/D4/D5/D6 |
| UART, 9600 baud, 8N1 | D1/TX → radio DIN through adapter | radio DOUT → D0/RX through adapter |
| Link indicator | — | Built-in LED lit while valid frames arrive |
| Ground | Common with its sensor/radio supply | Common with its servo/radio supply |

For each sensor, use `5V → flex sensor → analog node → fixed resistor → GND`. A 10 kΩ resistor is a starting point; choose a value appropriate to your sensor and measure the range. Code supports either rising or falling readings on bend.

**Power servos from a separate regulated supply sized for their combined stall current, with common ground. Do not power five servos through the Uno regulator. Bare XBee modules require model-specific voltage regulation and logic-level handling; do not connect a 5V Uno output directly to a 3.3V-only radio input.** Follow [Arduino's servo guidance](https://support.arduino.cc/hc/en-us/articles/360017053760-Troubleshoot-servo-motors) and your [Digi radio datasheet](https://docs.digi.com/resources/documentation/digidocs/pdfs/90002002.pdf).

Configure both radios for transparent UART mode (`AP=0` where supported), matching 9600 baud and compatible network/channel settings. Pair addressing/coordinator/router settings using instructions for your exact radio firmware. This firmware sends raw UART frames, not Digi API-mode packets. First prove byte transport with detached motors, then connect the palm.

## Calibrate before attaching tendons

1. Disconnect the radio; compile/upload `firmware/SensorCalibration` to the glove board. Open Serial Monitor at 9600 baud. Record each finger's open and closed reading in the printed thumb-to-little order.
2. Put those measured pairs in `kCalibration` in `PalmTransmitter.ino`. Supplied `200/460` pairs are historical starting values, not universal calibration. Equal endpoints fail open.
3. With unloaded servos, determine non-binding open/closed angles. Update `kOpenAngles` and `kClosedAngles` in `PalmReceiver.ino`; a reversed mechanism can use decreasing angles. Default 10–170° range must be checked for your servos and mechanism.
4. Compile/upload transmitter and receiver, reconnect radios, and confirm individual fingers respond correctly before attaching tendons or applying load.

At startup and after 500 ms without a valid frame, targets return to the configured open position. This is a software fallback, not a safety-certified stop; keep power disconnect accessible and keep fingers clear during calibration. There is no position/force feedback or detection of stalls or disconnected flex sensors.

## Data path and protocol

```text
5 analog sensors → calibrated flex angles → framed UART → paired radios
  → CRC/range validation → per-finger mechanical limits → slew-limited servos
```

Transmitter averages four ADC samples per finger and sends a 9-byte frame every 20 ms: `A5 5A sequence thumb index middle ring little CRC8`. Angles are unsigned 0–180. CRC-8/ATM covers the first eight bytes (polynomial `0x07`, initial `0`, no reflection/final XOR). Sequence wraps at 255 and is diagnostic; it does not authenticate senders or reject replays.

Receiver uses a sliding window to recover from noise, insertion, deletion, or partial frames. Bad CRC/out-of-range packets never refresh the link timer. Configure radio-network security if needed; CRC detects accidental corruption, not malicious packets. Both boards must run the new protocol; legacy `.pde` transmitters are incompatible.

## Requirements and verification

| Original requirement / implemented feature | Implementation | Verification |
|---|---|---|
| Five glove flex sensors | Calibration, clamping, averaging | Host tests over full ADC range, reversed/invalid calibration; Uno compilation |
| Five-finger gesture replication | Independent flex and servo mappings | Host samples; Uno compilation; physical movement needs bench test |
| Two-module wireless communication | 9600 baud transparent UART | Parser corruption/loss/noise tests; radio link needs paired hardware |
| Low-cost mechanical palm | Original report/diagrams retained | Historical build evidence; cost/mechanics not re-measured |
| Stream and link-loss handling | CRC-8, resynchronization, 500 ms timeout | Single-bit corruption, dropped-byte recovery, millis rollover tests |
| Mechanical limits and gradual motion | Per-servo endpoints and slew control | Mapping/step tests; actual safe endpoints need calibration |

CI runs host tests/demo and compiles all three sketches. Local verification passed with Arduino CLI 1.3.1, AVR core 1.8.6, Servo 1.2.2: transmitter 2,106 bytes flash / 215 bytes RAM; receiver 3,658 / 283; calibration 1,958 / 192. See [verification record](docs/verification.md) and [bench acceptance](docs/bench-acceptance.md) for physical checks still required.

## Repository layout

- `firmware/`: current Uno transmitter, receiver, and sensor-calibration sketches.
- `libraries/PalmControl/`: shared allocation-free protocol/control code.
- `tests/`, `tools/`: host regression checks and reproducible demonstration.
- `Code/`: original `.pde` listings retained unchanged as historical material.
- `Report/`, `Connection Diagrams/`: original project artifacts.

## Contributors and rights

Original contributors: Sreerag Sreenivasan, Vishnu K, Rahul R Nair, Vishnu S Dev. Original repository listed contributors under “License” but did not contain a software license grant; no new license is asserted here. [Original publication](http://ijirst.org/Article.php?manuscript=IJIRSTV2I6027) retained as historical attribution, not a build dependency.
