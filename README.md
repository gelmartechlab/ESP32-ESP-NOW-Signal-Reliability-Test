# ESP32 ESP-NOW Signal Strength, Reliability & Response Timing

ESP32 Master Course — Episode 10  
Gelmar Tech Lab | Build. Code. Innovate.

This project explores ESP-NOW wireless communication between an ESP32-S3 transmitter and an ESP32-D receiver without a router.

## What This Episode Covers

- Received signal strength (RSSI)
- Packet delivery in a 100-packet close-range test
- Button-to-LED input and output transitions on an oscilloscope

## Components

- ESP32-S3 development board
- ESP32-D / ESP32-WROOM-32 development board
- Push button
- LED
- 330-ohm resistor
- Breadboard and jumper wires
- USB power and data cables
- Two-channel oscilloscope for observing response timing

## Wiring

### Transmitter — ESP32-S3

| Connection | Pin |
|---|---|
| Button, one side | GPIO4 |
| Button, opposite side | GND |

GPIO4 uses INPUT_PULLUP:
- Released = HIGH
- Pressed = LOW

For a four-leg push button, use contacts that connect only when pressed.

### Receiver — ESP32-D

| Connection | Destination |
|---|---|
| GPIO23 | 330-ohm resistor |
| Resistor | LED anode (+) |
| LED cathode (-) | GND |

Both boards use Wi-Fi channel 1.

## Recorded Close-Range Results

| Measurement | Result |
|---|---|
| Unique test packets received | 100/100 |
| Missing packets | 0 |
| Packet reception | 100% for this run |
| Average RSSI | -9.2 dBm |
| Strongest RSSI | -8 dBm |
| Weakest RSSI | -10 dBm |
| Duplicate packets | 0 |
| Receiver queue drops | 0 |

These results describe one recorded close-range run. They do not establish maximum range or guarantee the same results in another environment.

## Oscilloscope Connections

- Channel 1 tip: transmitter GPIO4 button input
- Channel 1 ground: transmitter GND
- Channel 2 tip: receiver GPIO23 LED output
- Channel 2 ground: receiver GND

Use low-voltage USB-powered development boards. Standard oscilloscope channel grounds are usually connected internally, so these connections also join the boards' grounds.

Compare the button input falling edge with the receiver LED output rising edge.

This interval includes button handling, wireless communication, and receiver processing.

The recorded wide time scale showed the transitions but did not establish an accurate numerical delay. A closer time scale and time cursors are needed to measure it.

## Sketches

- [Transmitter sketch](Transmitter_EP10/Transmitter_EP10.ino)
- [Receiver sketch](Receiver_EP10/Receiver_EP10.ino)

These sketches provide button-to-LED control and live RSSI.
The 100-packet results above came from a separate recorded test.

The transmitter uses 30 ms button debounce, which contributes
to the observed button-to-LED response delay.

Before uploading, update RECEIVER_MAC in the transmitter to match
the MAC printed by your receiver. Set Serial Monitor to 115200 baud.
## Channel

https://www.youtube.com/@gelmartechlab

