Created by @sstaub

# OSC Library

The **OSC Library** allows you to create OSC (Open Sound Control) messages. Only sending of OSC messages is supported.

## Installation

1. "Download":https://github.com/sstaub/OSC/archive/master.zip the Master branch from GitHub.
2. Unzip and modify the folder name to "OSC"
3. Move the modified folder on your Library folder (On your `Libraries` folder inside Sketchbooks or Arduino software).

## Example

Small example for creating an OSC message from a given OSC address pattern, a value and a transport type. It return the length of the OSC message and send it with an UDP instance **osc** of Ethernet.

```
char oscAddress[64] = "/osc/test";
char oscMessage[128];
int16_t oscMessageLength;
int32_t value = 1;
oscMessageLength = oscSend(oscMessage, oscAddress, value, UDP);
osc.beginPacket(IP_ADDRESS_OSC_TX, OSC_PORT_TX);
osc.write(oscMessage, oscMessageLength);
osc.endPacket();
```

## Documentation

### Data Types
- i int32_t
- f float
- s string
- T, F, N, I flags

### Protocoll Types
- UDP
- TCP10
- TCP11
	
### Functions

**int32_t oscSend(char oscPacket[], char oscAddressPattern[], int32_t value, protocol_t protocol);**<br>
for int32 types

**int32_t oscSend(char oscPacket[], char oscAddressPattern[], float value, protocol_t protocol);**<br>
for floats

**int32_t oscSend(char oscPacket[], char oscAddressPattern[], char string[], protocol_t protocol);**<br>
for strings

**int32_t oscSend(char oscPacket[], char oscAddressPattern[], flag_t flag, protocol_t protocol);**<br>
for flags

**int32_t oscSend(char oscPacket[], char oscAddressPattern[], protocol_t protocol);**<br>
for message without a value or flag


