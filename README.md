# ESP32 piezo transmitter experiment

This project explores a 2N7000 MOSFET driver for a nominal 25 kHz piezo, with the goal of investigating microphone interference. One piezo produced an audible buzz during a 2.5 kHz-range physical test. There is a slight distortion when operating with the ultrasonic range. 

## Current physical circuit

The latest proposed arrangement is shown in [circuit_9v.svg](circuit_9v.svg): ESP32 powered through USB; a separate 9 V battery supplies the piezo stage; the battery negative and ESP32 ground are common. It uses one piezo, with a 3.3 kΩ resistor across it. There is no voltage divider in this version.

| From | To |
| --- | --- |
| ESP32 GPIO18 | MOSFET gate |
| ESP32 GPIO19 | Same gate junction, for pulse monitoring |
| MOSFET gate | 10 kΩ resistor to common ground |
| MOSFET source | Common ground |
| Battery negative | Common ground and ESP32 GND |
| Battery positive | Piezo main electrode and one end of 3.3 kΩ |
| MOSFET drain | Piezo brass electrode and other end of 3.3 kΩ |
| Piezo feedback electrode | Insulated and unconnected |

Confirm the 2N7000 pinout against the exact manufacturer's datasheet. From a photo, green appeared to be the main electrode, blue the brass backing, and yellow a separate feedback electrode; **those assignments are tentative** and must be verified. The other three piezos are disconnected in this experiment. The unidentified diode is not used.

**The 3.3 kΩ resistor is parallel to the piezo; it does not limit current through the piezo.** Its ideal dissipation with 9 V across it is approximately 0.025 W. The battery voltage may exceed 9 V when fresh, and the piezo voltage may ring above the supply. The piezo's permissible drive voltage, capacitance, impedance and any current limit are unknown; confirm its rating before applying 9 V. Never connect GPIO19 to the drain, piezo or battery positive. The gate signal is nominally 0–3.3 V. Avoid close exposure of ears or animals to ultrasound.

## ESP32 sketch and pulse monitor

Open [esp32_test/esp32_test.ino](esp32_test/esp32_test.ino) in Arduino IDE with Arduino-ESP32 core 3.x. The current file on disk has these settings:

- GPIO18 provides PWM; `PWM_BITS = 8` is a timer resolution setting, not a pin.
- `MIN_HZ = 2400` and `MAX_HZ = 2600`: the sketch currently chooses a random **2.4–2.6 kHz** frequency approximately every 10 ms. It initially attaches PWM at 25 kHz, then changes to the configured range on the first hop.
- GPIO19 counts rising edges at the gate. Connect it only to the gate/GPIO18 junction; confirm this pin is available on the particular board.
- Open Serial Monitor at **115200 baud**. The reported count is an average over roughly one second; the last configured frequency is an instantaneous setting, so they need not match. Pulse detection does not measure the gate peak voltage, piezo voltage, acoustic output or microphone interference.

For the intended 24–26 kHz experiment, the constants would need to be changed to `24000` and `26000`, then the sketch uploaded again. This change has not been made here. The sketch is unverified by compilation or hardware testing in this workspace. The actual pulse monitor was observed reporting around 25,000 pulses/s in a previous 24–26 kHz run.

## LTspice simulation

The LTspice circuit represents the ESP32 signal with a voltage source, uses a **2N7002** model, and models the piezo as an assumed **20 nF capacitor**. This is a simplified electrical model; it does not model the disc's resonance or acoustic output, and it is not an exact model of the physical 2N7000 or unknown piezo.

- [LTspice_simulation_instructions.txt](LTspice_simulation_instructions.txt) contains setup instructions and fixed/swept input examples. Its random-input waveform changes frequency **every cycle**, while the ESP32 sketch changes frequency approximately every **10 ms**; the signals are not identical. The file's statement that the ESP32 hops every 10 ms is correct, but consult the sketch for its current frequency range.
- [random_25khz_pwl.txt](random_25khz_pwl.txt) is a saved, repeatable pseudorandom 24–26 kHz gate-input waveform. It does not produce a new sequence each simulation.
- Use transient directive `.tran 0 10m 0 100n`. Plot the gate, drain, VDD, and differential voltage across the piezo model. A 20 nF value is illustrative, not a measured rating.

Observed example plots with the 9 V battery divider and assumed 20 nF model showed approximately 0.45 V peak-to-peak across the capacitor with 10 kΩ in parallel, and approximately 2.4 V peak-to-peak with 1 kΩ. A later ideal 9 V source and 3.3 kΩ simulation showed the capacitor voltage roughly 6.6–9 V. These are model-dependent readings, not measured hardware results.

A frequency sweep was also explored in LTspice using a behavioural source. The pseudorandom and sweep approaches change pulse spacing; they do not establish that a phone's filtering will be bypassed. At 25 kHz, the roughly 20 µs half-cycle must be compared with the piezo's actual response and the resistor discharge time.

## Supply and earlier versions

A proposed divider of 470 Ω over 560 Ω produces about 4.89 V from an ideal 9 V source without load. Its voltage falls when loaded, so it is not a regulated 5 V supply. The project subsequently moved to the direct 9 V circuit shown above. A regulated 5 V supply was also considered; do not connect 9 V or 5 V to ESP32 GPIOs or its 3V3 pin.

Earlier diagrams and notes are retained as historical variants, not the current 9 V circuit: [circuit.svg](circuit.svg), [circuit_3v3.svg](circuit_3v3.svg), [circuit_5v.svg](circuit_5v.svg), [specifications.txt](specifications.txt), [specifications_3v3.txt](specifications_3v3.txt), and [specifications_5v.txt](specifications_5v.txt). Their resistor values and supplies differ from the current arrangement.

## Remaining checks

1. Identify the exact piezo and MOSFET, confirm piezo electrode mapping and allowable voltage, and confirm resistor wattages and common ground.
2. Verify the gate and piezo voltage waveforms with suitable instrumentation; GPIO19 pulse count alone does not measure voltage or verify piezo drive.
3. Verify ultrasonic output with a suitable ultrasonic sensor. Compare a controlled recording with the circuit off and on to evaluate the particular microphone. The presence of an ultrasonic signal does not guarantee interference.
4. If modelling a different piezo capacitance in LTspice, set C1 to `{Cpiezo}` and add `.step param Cpiezo list 5n 20n 50n`. These remain ideal capacitor assumptions unless backed by measurements or a manufacturer model.
